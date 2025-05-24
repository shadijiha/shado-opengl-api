#include "Scene.h"

#include <box2d/b2_body.h>
#include <box2d/b2_circle_shape.h>
#include <box2d/b2_contact.h>
#include <box2d/b2_fixture.h>
#include <box2d/b2_polygon_shape.h>
#include <box2d/b2_world.h>

#include "Components.h"
#include "Entity.h"
#include "Prefab.h"
#include "asset/AssetManager.h" // <--- This is needed DO NOT REMOVE
#include "debug/Profile.h"
#include "renderer/Renderer2D.h"
#include "script/ScriptEngine.h"

namespace Shado {
    /**
     * **********************************
     ************************************
     * **********************************
     */
    // This is here to avoid havning to include box2d in header files
    class Physics2DCallback : public b2ContactListener {
    private:
        struct Points {
            glm::vec2 p1, p2;
        };

        struct Separations {
            float s1, s2;
        };

        struct Collision2DInfo {
            glm::vec2 normal;
            Points points;
            Separations separations;
        };

    public:
        Physics2DCallback(Scene* scene): scene(scene) {}

        virtual void BeginContact(b2Contact* contact) override {
            invokeCollisionFunction(contact, "OnCollision2DEnter");
        }

        virtual void EndContact(b2Contact* contact) override {
            invokeCollisionFunction(contact, "OnCollision2DLeave");
        }

    private:
        Collision2DInfo buildContactInfoObject(b2Contact* contact) {
            b2WorldManifold manifold;
            contact->GetWorldManifold(&manifold);

            // Create C# object
            glm::vec2 normal = {manifold.normal.x, manifold.normal.y};

            Points points = {
                glm::vec2{manifold.points[0].x, manifold.points[0].y},
                glm::vec2{manifold.points[1].x, manifold.points[1].y}
            };

            Separations separations = {
                manifold.separations[0],
                manifold.separations[1]
            };

            return {
                normal,
                points,
                separations
            };
        }

        void invokeCollisionFunction(b2Contact* contact, const std::string& functionName) {
            // Entity A
            uint64_t idA = contact->GetFixtureA()->GetBody()->GetUserData().pointer;
            uint64_t idB = contact->GetFixtureB()->GetBody()->GetUserData().pointer;

            Entity entityA = scene->getEntityById(idA);
            Entity entityB = scene->getEntityById(idB);

            // Call the Entity::OnCollision2D in C#
            if (entityA.isValid() && entityB.isValid()) {
                CSharpObject scriptAInstance = {};
                CSharpObject scriptBInstance = {};

                if (entityA.hasComponent<ScriptComponent>())
                    scriptAInstance = entityA.getComponent<ScriptComponent>().Instance;
                if (entityB.hasComponent<ScriptComponent>())
                    scriptBInstance = entityB.getComponent<ScriptComponent>().Instance;

                if (scriptAInstance.IsValid()) {
                    auto info = buildContactInfoObject(contact);
                    scriptAInstance.Invoke(functionName, info, idB);
                }

                if (scriptBInstance.IsValid()) {
                    auto info = buildContactInfoObject(contact);
                    scriptBInstance.Invoke(functionName, info, idA);
                }
            }
        }

    private:
        Scene* scene;
    };

    /**
     * **********************************
     ************ SCENE CLASS ***********
     * **********************************
     */
    static Physics2DCallback* s_physics2DCallback = nullptr;

    // Helpers
    namespace {
        template <typename T>
        concept ValidComponent = std::is_base_of_v<Shado::Component, T>;

        template <ValidComponent Component>
        void CopyComponent(entt::registry& dst, entt::registry& src,
                           const std::unordered_map<UUID, entt::entity>& enttMap) {
            auto view = src.view<Component>();
            for (auto e : view) {
                UUID uuid = src.get<IDComponent>(e).id;
                SHADO_CORE_ASSERT(enttMap.find(uuid) != enttMap.end(), "");
                entt::entity dstEnttID = enttMap.at(uuid);

                auto& component = src.get<Component>(e);
                dst.emplace_or_replace<Component>(dstEnttID, component);
            }
        }

        template <ValidComponent Component>
        void CopyComponentIfExists(Entity dst, Entity src) {
            if (src.hasComponent<Component>())
                dst.addOrReplaceComponent<Component>(src.getComponent<Component>());
        }
    }

    Scene::Scene() {}

    Scene::Scene(Scene& other) {
        m_ViewportWidth = other.m_ViewportWidth;
        m_ViewportHeight = other.m_ViewportHeight;
        name = other.name + " [Runtime]";

        auto& srcSceneRegistry = other.m_Registry;
        auto& dstSceneRegistry = m_Registry;

        CopyRegistries(srcSceneRegistry, dstSceneRegistry, [this](const auto& tag, auto uuid) {
            return this->createEntityWithUUID(tag, uuid);
        });

        other.m_ScriptStorage.CopyTo(this->m_ScriptStorage);
    }

    Scene::~Scene() {}

    Entity Scene::createEntity(const std::string& name) {
        return createEntityWithUUID(name, UUID());
    }

    Entity Scene::createEntityWithUUID(const std::string& name, Shado::UUID uuid) {
        entt::entity id = m_Registry.create();
        Entity entity = {id, this};

        entity.addComponent<IDComponent>().id = uuid;
        entity.addComponent<TransformComponent>();

        auto& tag = entity.addComponent<TagComponent>();
        tag.tag = name.empty() ? std::string("Entity ") + std::to_string((uint64_t)uuid) : name;

        return entity;
    }

    static void CopyAllComponentsHelper(Entity newEntity, Entity source, ScriptStorage& scriptStorage,
                                        bool copyTransform = true, bool copyScriptStorage = true) {
        if (copyTransform)
            CopyComponentIfExists<TransformComponent>(newEntity, source);
        CopyComponentIfExists<SpriteRendererComponent>(newEntity, source);
        CopyComponentIfExists<CircleRendererComponent>(newEntity, source);
        CopyComponentIfExists<LineRendererComponent>(newEntity, source);
        CopyComponentIfExists<CameraComponent>(newEntity, source);
        CopyComponentIfExists<NativeScriptComponent>(newEntity, source);
        CopyComponentIfExists<RigidBody2DComponent>(newEntity, source);
        CopyComponentIfExists<BoxCollider2DComponent>(newEntity, source);
        CopyComponentIfExists<CircleCollider2DComponent>(newEntity, source);
        CopyComponentIfExists<PrefabInstanceComponent>(newEntity, source);
        CopyComponentIfExists<TextComponent>(newEntity, source);

        // Script should always be last
        CopyComponentIfExists<ScriptComponent>(newEntity, source);

        if (newEntity.hasComponent<ScriptComponent>() && copyScriptStorage) {
            const auto& scriptComponent = newEntity.getComponent<ScriptComponent>();
            scriptStorage.InitializeEntityStorage(scriptComponent.ScriptID, newEntity.getUUID());
            scriptStorage.CopyEntityStorage(source.getUUID(), newEntity.getUUID(), scriptStorage);
        }
    }

    static Entity duplicateEntityWithUUID(Scene& scene, Entity source, UUID id, bool modifyTag,
                                          bool copyScriptStorage = true) {
        if (!source)
            return {};

        Entity newEntity = scene.createEntityWithUUID(
            source.getComponent<TagComponent>().tag +
            (modifyTag ? " (2)" : ""), id);

        CopyAllComponentsHelper(newEntity, source, scene.GetScriptStorage(), true, copyScriptStorage);

        return newEntity;
    }

    Entity Scene::duplicateEntity(Entity source, bool modifyTag) {
        return duplicateEntityWithUUID(*this, source, UUID(), modifyTag);
    }

    void Scene::destroyEntity(Entity entity) {
        if (!entity)
            return;

        // If currently inside animation
        if (m_World && m_World->IsLocked()) {
            toDestroy.push_back(entity);
            return;
        }

        // Destroy physics
        if (m_World) {
            if (entity.hasComponent<RigidBody2DComponent>()) {
                auto& rb = entity.getComponent<RigidBody2DComponent>();
                auto* body = (b2Body*)rb.runtimeBody;

                if (entity.hasComponent<BoxCollider2DComponent>()) {
                    auto& bc = entity.getComponent<BoxCollider2DComponent>();
                    body->DestroyFixture((b2Fixture*)bc.runtimeFixture);
                }
                if (entity.hasComponent<CircleCollider2DComponent>()) {
                    auto& bc = entity.getComponent<CircleCollider2DComponent>();
                    body->DestroyFixture((b2Fixture*)bc.runtimeFixture);
                }

                m_World->DestroyBody((b2Body*)rb.runtimeBody);
            }
        }

        // Destroy children
        for (const auto& child : entity.getChildren()) {
            destroyEntity(child);
        }

        // Call OnDestroy for scripts
        if (isRunning()) {
            if (entity.hasComponent<ScriptComponent>()) {
                auto& scriptComponent = entity.getComponent<ScriptComponent>();
                if (scriptComponent.Instance.IsValid()) {
                    scriptComponent.Instance.Invoke("OnDestroy");
                    ScriptEngine::GetMutable().DestroyInstance(entity.getUUID(), m_ScriptStorage);
                }
            }
        }

        m_Registry.destroy(entity);
    }

    Entity Scene::instantiatePrefabHelper(Ref<Prefab> prefab, Entity toDuplicate, bool modifyTag) {
        Scene& scene = *this;
        auto instantiatedEntityId = UUID();

        // Copy the script storage from the prefab to the new entity with instantiatedEntityId
        if (toDuplicate.hasComponent<ScriptComponent>()) {
            auto& scriptComponent = toDuplicate.getComponent<ScriptComponent>();
            scene.GetScriptStorage().InitializeEntityStorage(scriptComponent.ScriptID,
                                                             instantiatedEntityId);
            prefab->GetScriptStorage().CopyEntityStorage(toDuplicate.getUUID(), instantiatedEntityId,
                                                         scene.GetScriptStorage());

            if (scene.isRunning()) {
                scriptComponent.Instance = ScriptEngine::GetMutable().Instantiate(
                    instantiatedEntityId, scene.GetScriptStorage(),
                    uint64_t(instantiatedEntityId));

                // For some reason we need to delay the call to OnCreate
                Application::get().SubmitToMainThread([this, &scriptComponent]() {
                    scriptComponent.Instance.Invoke("OnCreate");
                });
            }
        }


        Entity duplicated = duplicateEntityWithUUID(scene,
                                                    toDuplicate,
                                                    instantiatedEntityId,
                                                    modifyTag,
                                                    false // We already copied script storage no need to copy it again
        );

        for (const auto& oldChild : toDuplicate.getChildren()) {
            Entity newChild = instantiatePrefabHelper(prefab, oldChild, modifyTag);
            newChild.getComponent<TransformComponent>().setParent(newChild, duplicated);
        }

        return duplicated;
    }

    Entity Scene::instantiatePrefab(Ref<Prefab> prefab, bool modifyTag) {
        return instantiatePrefabHelper(prefab, prefab->root, modifyTag);
    }

    void Scene::propagatePrefabChanges(Ref<Prefab> prefabChanged) {
        auto view = m_Registry.view<PrefabInstanceComponent>();

        for (auto entity : view) {
            Entity e = {entity, this};
            auto& prefabComponent = e.getComponent<PrefabInstanceComponent>();

            // If the prefab is not the same, skip
            // If the entity is a child of the prefab root, skip because it will be updated by the root
            if (prefabComponent.prefabId != prefabChanged->GetId())
                continue;

            UUID prefabEntityUniqueId = prefabComponent.prefabEntityUniqueId;
            Entity prefabEntity = prefabChanged->GetEntityByElementUniqueId(prefabEntityUniqueId); // <-- to copy

            if (prefabEntity) {
                // Make sure to keep the old parent id
                Entity oldParent = e.getComponent<TransformComponent>().getParent(*this);

                // Make sure to copy the script storage
                if (prefabEntity.hasComponent<ScriptComponent>()) {
                    m_ScriptStorage.InitializeEntityStorage(prefabEntity.getComponent<ScriptComponent>().ScriptID,
                                                            e.getUUID());
                    prefabChanged->GetScriptStorage().CopyEntityStorage(prefabEntity.getUUID(), e.getUUID(),
                                                                        m_ScriptStorage);
                }

                CopyAllComponentsHelper(e, prefabEntity, m_ScriptStorage, e.isChild(*this), false);
                e.getComponent<TransformComponent>().setParent(e, oldParent);
            } else
                SHADO_CORE_WARN("Prefab element {0} not found", prefabEntityUniqueId);
        }
    }

    void Scene::onRuntimeStart() {
        m_IsRunning = true;

        Ref<Scene> _this = this;
        // Scripting
        {
            auto& scriptEngine = ScriptEngine::GetMutable();
            scriptEngine.SetCurrentScene(_this);

            auto view = m_Registry.view<IDComponent, ScriptComponent>();
            for (auto scriptEntityID : view) {
                const auto& idComponent = view.get<IDComponent>(scriptEntityID);
                auto& scriptComponent = view.get<ScriptComponent>(scriptEntityID);

                if (!scriptEngine.IsValidScript(scriptComponent.ScriptID)) {
                    SHADO_CORE_WARN("Entity '{}' has an invalid script id '{}'", idComponent.id,
                                    scriptComponent.ScriptID);
                    continue;
                }

                if (!m_ScriptStorage.EntityStorage.contains(idComponent.id)) {
                    SHADO_CORE_ERROR("Entity {} isn't in script storage", Entity{scriptEntityID,
                                     this}.getComponent<TagComponent>().tag);
                    SHADO_CORE_ASSERT(false, "");
                }

                scriptComponent.Instance = scriptEngine.Instantiate(idComponent.id, m_ScriptStorage,
                                                                    uint64_t(idComponent.id));
            }

            for (auto scriptEntityID : view) {
                auto& scriptComponent = view.get<ScriptComponent>(scriptEntityID);

                if (!scriptEngine.IsValidScript(scriptComponent.ScriptID)) {
                    continue;
                }

                scriptComponent.Instance.Invoke("OnCreate");
            }
        }

        // TODO make the physics adjustable
        softResetPhysics();
    }

    void Scene::onRuntimeStop() {
        m_IsRunning = false;

        Ref<Scene> _this = this;
        auto& scriptEngine = ScriptEngine::GetMutable();
        auto view = m_Registry.view<IDComponent, ScriptComponent>();
        for (auto scriptEntityID : view) {
            const auto& idComponent = view.get<IDComponent>(scriptEntityID);
            auto& scriptComponent = view.get<ScriptComponent>(scriptEntityID);

            if (!scriptEngine.IsValidScript(scriptComponent.ScriptID)) {
                continue;
            }

            if (!m_ScriptStorage.EntityStorage.contains(idComponent.id) || !scriptComponent.Instance.IsValid()) {
                // Shouldn't happen
                continue;
            }

            scriptComponent.Instance.Invoke("OnDestroy");

            scriptEngine.DestroyInstance(idComponent.id, m_ScriptStorage);
        }

        delete m_World;
        m_World = nullptr;

        delete s_physics2DCallback;
        s_physics2DCallback = nullptr;
    }

    void Scene::onUpdateRuntime(TimeStep ts) {
        // Update Native script
        {
            m_Registry.view<NativeScriptComponent>().each([this, ts](auto entity, NativeScriptComponent& nsc) {
                // TODO: mave to onScenePlay
                if (!nsc.script) {
                    nsc.script = nsc.instantiateScript();
                    nsc.script->m_EntityHandle = entity;
                    nsc.script->m_Scene = this;
                    nsc.script->onCreate();
                }

                nsc.script->onUpdate(ts);
            });
        }

        // Update scripts
        {
            auto view = m_Registry.view<ScriptComponent>();
            const auto& scriptEngine = ScriptEngine::GetInstance();
            {
                SHADO_PROFILE_FUNCTION();

                for (auto scriptEntityID : view) {
                    auto& scriptComponent = view.get<ScriptComponent>(scriptEntityID);

                    if (!scriptEngine.IsValidScript(scriptComponent.ScriptID) || !scriptComponent.Instance.IsValid()) {
                        SHADO_CORE_ERROR("Entity {} has invalid script!",
                                         Entity(scriptEntityID, this).getComponent<TagComponent>().tag);
                        continue;
                    }

                    scriptComponent.Instance.Invoke<float>("OnUpdate", ts);
                }
            }

            {
                SHADO_PROFILE_FUNCTION();
                Timer timer;

                for (auto scriptEntityID : view) {
                    auto& scriptComponent = view.get<ScriptComponent>(scriptEntityID);

                    if (!scriptEngine.IsValidScript(scriptComponent.ScriptID) || !scriptComponent.Instance.IsValid()) {
                        continue;
                    }

                    scriptComponent.Instance.Invoke<float>("OnLateUpdate", ts);
                }
            }
        }

        // Update physics
        if (m_PhysicsEnabled) {
            const int32_t velocityIterations = 6;
            const int32_t positionIterations = 2;
            m_World->Step(ts, velocityIterations, positionIterations);

            // Get transforms from box 2d
            auto view = m_Registry.view<RigidBody2DComponent>();
            for (auto e : view) {
                Entity entity = {e, this};

                auto& transform = entity.getComponent<TransformComponent>();
                auto& rb2D = entity.getComponent<RigidBody2DComponent>();

                auto body = (b2Body*)rb2D.runtimeBody;
                if (body) {
                    transform.position.x = body->GetPosition().x;
                    transform.position.y = body->GetPosition().y;
                    transform.rotation.z = body->GetAngle();
                }
            }
        }

        // After world has update delete all entities that need to be deleted
        if (!toDestroy.empty()) {
            for (auto& to_delete : toDestroy) {
                destroyEntity(to_delete);
            }
            toDestroy.clear();
        }
    }

    void Scene::onDrawRuntime() {
        // Render 2D: Cameras
        Camera* primaryCamera = nullptr;
        glm::mat4 cameraTransform;
        {
            // Loop through ortho cameras
            auto group = m_Registry.view<TransformComponent, CameraComponent>();
            for (auto entity : group) {
                auto [transform, camera] = group.get<TransformComponent, CameraComponent>(entity);

                if (camera.primary) {
                    primaryCamera = camera.camera.Raw();
                    cameraTransform = transform.getTransform(*this);
                    break;
                }
            }
        }

        static EditorCamera camera;
        if (primaryCamera) {
            Renderer2D::BeginScene(*primaryCamera, cameraTransform);

            // Render stuff
            auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
            for (auto entity : group) {
                auto [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);
                Renderer2D::DrawSprite(transform.getTransform(*this), sprite, (int)entity);
            }

            // Draw circles
            {
                auto view = m_Registry.view<TransformComponent, CircleRendererComponent>();
                for (auto entity : view) {
                    auto [transform, circle] = view.get<TransformComponent, CircleRendererComponent>(entity);

                    if (circle.texture) {
                        Renderer2D::DrawCircle(transform.getTransform(*this), circle.texture, circle.tilingFactor,
                                               circle.color, circle.thickness, circle.fade, (int)entity);
                    } else {
                        Renderer2D::DrawCircle(transform.getTransform(*this), circle.color, circle.thickness,
                                               circle.fade, (int)entity);
                    }
                }
            }

            // Draw lines
            {
                auto view = m_Registry.view<TransformComponent, LineRendererComponent>();
                for (auto entity : view) {
                    auto [transform, line] = view.get<TransformComponent, LineRendererComponent>(entity);
                    Renderer2D::DrawLine(transform.getPosition(*this), line.target, line.color, (int)entity);
                }
            }

            // Draw Text
            {
                auto view = m_Registry.view<TransformComponent, TextComponent>();
                for (auto entity : view) {
                    auto [transform, text] = view.get<TransformComponent, TextComponent>(entity);
                    Renderer2D::DrawString(transform.getTransform(*this), text, (int)entity);
                }
            }

            Renderer2D::EndScene();
        }
    }

    void Scene::onUpdateEditor(TimeStep ts, EditorCamera& camera) {}

    void Scene::onDrawEditor(EditorCamera& camera) {
        Renderer2D::BeginScene(camera);

        // Render Quads stuff
        auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
        for (auto entity : group) {
            auto [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);
            Renderer2D::DrawSprite(transform.getTransform(*this), sprite, (int)entity);
        }

        // Draw circles
        {
            auto view = m_Registry.view<TransformComponent, CircleRendererComponent>();
            for (auto entity : view) {
                auto [transform, circle] = view.get<TransformComponent, CircleRendererComponent>(entity);

                if (circle.texture) {
                    Renderer2D::DrawCircle(transform.getTransform(*this), circle.texture, circle.tilingFactor,
                                           circle.color, circle.thickness, circle.fade, (int)entity);
                } else {
                    Renderer2D::DrawCircle(transform.getTransform(*this), circle.color, circle.thickness, circle.fade,
                                           (int)entity);
                }
            }
        }

        // Draw lines
        {
            auto view = m_Registry.view<TransformComponent, LineRendererComponent>();
            for (auto entity : view) {
                auto [transform, line] = view.get<TransformComponent, LineRendererComponent>(entity);
                Renderer2D::DrawLine(transform.getPosition(*this), line.target, line.color, (int)entity);
            }
        }

        // Draw Text
        {
            auto view = m_Registry.view<TransformComponent, TextComponent>();
            for (auto entity : view) {
                auto [transform, text] = view.get<TransformComponent, TextComponent>(entity);
                Renderer2D::DrawString(transform.getTransform(*this), text, (int)entity);
            }
        }

        Renderer2D::EndScene();
    }

    void Scene::onViewportResize(uint32_t width, uint32_t height) {
        m_ViewportWidth = width;
        m_ViewportHeight = height;

        // Resize cams
        // Loop through ortho cameras
        auto orthCams = m_Registry.view<CameraComponent>();
        for (auto entity : orthCams) {
            auto& camera = orthCams.get<CameraComponent>(entity);

            if (!camera.fixedAspectRatio) {
                camera.setViewportSize(width, height);
            }
        }
    }

    Entity Scene::getPrimaryCameraEntity() {
        auto cams = m_Registry.view<CameraComponent>();
        for (auto entity : cams) {
            const auto& camera = cams.get<CameraComponent>(entity);

            if (camera.primary)
                return {entity, this};
        }
        return {};
    }

    Entity Scene::getEntityById(uint64_t entityId) {
        auto ids = m_Registry.view<IDComponent>();
        for (auto entity : ids) {
            const auto& idComponent = ids.get<IDComponent>(entity);

            if (idComponent.id == entityId)
                return {entity, this};
        }
        return {};
    }

    Entity Scene::findEntityByName(std::string_view name) {
        auto view = m_Registry.view<TagComponent>();
        for (auto entity : view) {
            const TagComponent& tc = view.get<TagComponent>(entity);
            if (tc.tag == name)
                return Entity{entity, this};
        }
        return {};
    }

    std::vector<Entity> Scene::getAllEntities() {
        std::vector<Entity> entities;

        auto view = m_Registry.view<IDComponent>();
        for (auto e : view) {
            Entity entity = {e, this};
            entities.push_back(entity);
        }

        return entities;
    }

    void Scene::softResetPhysics() {
        if (m_World) {
            delete m_World;
            m_World = nullptr;
        }

        // TODO make the physics adjustable
        s_physics2DCallback = snew(Physics2DCallback) Physics2DCallback(this);
        m_World = snew(b2World) b2World({0.0f, -9.8f});
        m_World->SetContactListener(s_physics2DCallback);

        auto view = m_Registry.view<RigidBody2DComponent>();
        for (auto e : view) {
            Entity entity = {e, this};

            auto& id = entity.getComponent<IDComponent>();
            auto& transform = entity.getComponent<TransformComponent>();
            auto& rb2D = entity.getComponent<RigidBody2DComponent>();


            b2BodyDef bodyDef;
            bodyDef.type = (b2BodyType)rb2D.type; // TODO : maybe change this
            bodyDef.fixedRotation = rb2D.fixedRotation;
            bodyDef.position.Set(transform.position.x, transform.position.y);
            bodyDef.angle = transform.rotation.z;

            b2BodyUserData useData;
            useData.pointer = id.id;
            bodyDef.userData = useData;

            b2Body* body = m_World->CreateBody(&bodyDef);
            rb2D.runtimeBody = body;


            if (entity.hasComponent<BoxCollider2DComponent>()) {
                auto& collider = entity.getComponent<BoxCollider2DComponent>();

                b2PolygonShape polygonShape;
                polygonShape.SetAsBox(transform.scale.x * collider.size.x, transform.scale.y * collider.size.y);

                b2FixtureDef fixtureDef;
                fixtureDef.restitution = collider.restitution;
                fixtureDef.restitutionThreshold = collider.restitutionThreshold;
                fixtureDef.friction = collider.friction;
                fixtureDef.density = collider.density;
                fixtureDef.shape = &polygonShape;

                b2Fixture* fixture = body->CreateFixture(&fixtureDef);
                collider.runtimeFixture = fixture;
            }

            if (entity.hasComponent<CircleCollider2DComponent>()) {
                auto& cc2d = entity.getComponent<CircleCollider2DComponent>();

                b2CircleShape circleShape;
                circleShape.m_p.Set(cc2d.offset.x, cc2d.offset.y);
                circleShape.m_radius = transform.scale.x * cc2d.radius.x;

                b2FixtureDef fixtureDef;
                fixtureDef.shape = &circleShape;
                fixtureDef.density = cc2d.density;
                fixtureDef.friction = cc2d.friction;
                fixtureDef.restitution = cc2d.restitution;
                fixtureDef.restitutionThreshold = cc2d.restitutionThreshold;
                body->CreateFixture(&fixtureDef);
            }
        }
    }

    ScriptStorage& Scene::GetScriptStorage() {
        return m_ScriptStorage;
    }

    void CopyRegistries(entt::registry& srcSceneRegistry, entt::registry& dstSceneRegistry,
                        std::function<Entity(const std::string&, UUID)> entityCreatorFunction) {
        std::unordered_map<UUID, entt::entity> enttMap;

        auto idView = srcSceneRegistry.view<IDComponent>();
        for (auto e : idView) {
            UUID uuid = srcSceneRegistry.get<IDComponent>(e).id;
            const auto& tag = srcSceneRegistry.get<TagComponent>(e).tag;

            Entity entity = entityCreatorFunction(tag, uuid);
            enttMap[uuid] = (entt::entity)entity;
        }

        // Copy components (except ID Component and Tag component)
        CopyComponent<TransformComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
        CopyComponent<SpriteRendererComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
        CopyComponent<CircleRendererComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
        CopyComponent<LineRendererComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
        CopyComponent<CameraComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
        CopyComponent<NativeScriptComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
        CopyComponent<RigidBody2DComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
        CopyComponent<BoxCollider2DComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
        CopyComponent<CircleCollider2DComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
        CopyComponent<PrefabInstanceComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
        CopyComponent<TextComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
        CopyComponent<ScriptComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
    }
}
