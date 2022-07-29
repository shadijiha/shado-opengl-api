#include "Scene.h"
#include "Components.h"
#include "renderer/Renderer2D.h"
#include "Entity.h"
#include <box2d/b2_world.h>

#include "box2d/b2_body.h"
#include "box2d/b2_fixture.h"
#include "box2d/b2_polygon_shape.h"
#include "box2d/b2_circle_shape.h"

namespace Shado {
	template<typename Component>
	static void CopyComponent(entt::registry& dst, entt::registry& src, const std::unordered_map<UUID, entt::entity>& enttMap);
	template<typename Component>
	static void CopyComponentIfExists(Entity dst, Entity src);

	Scene::Scene() {
	}

	Scene::Scene(Scene& other)
	{
		m_ViewportWidth = other.m_ViewportWidth;
		m_ViewportHeight = other.m_ViewportHeight;
		name = other.name + " [Runtime]";

		auto& srcSceneRegistry = other.m_Registry;
		auto& dstSceneRegistry = m_Registry;
		std::unordered_map<UUID, entt::entity> enttMap;

		auto idView = srcSceneRegistry.view<IDComponent>();
		for(auto e : idView) {
			UUID uuid = srcSceneRegistry.get<IDComponent>(e).id;
			const auto& tag = srcSceneRegistry.get<TagComponent>(e).tag;

			Entity entity = createEntityWithUUID(tag, uuid);
			enttMap[uuid] = (entt::entity)entity;
		}

		// Copy components (except ID Component and Tag component)
		CopyComponent<TransformComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
		CopyComponent<SpriteRendererComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
		CopyComponent<CircleRendererComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
		CopyComponent<CameraComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
		CopyComponent<NativeScriptComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
		CopyComponent<RigidBody2DComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
		CopyComponent<BoxCollider2DComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
		CopyComponent<CircleCollider2DComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
		CopyComponent<ScriptComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
	}

	Scene::~Scene() {
	}

	Entity Scene::createEntity(const std::string& name) {
		return createEntityWithUUID(name, UUID());
	}

	Entity Scene::createEntityWithUUID(const std::string& name, Shado::UUID uuid) {
		entt::entity id = m_Registry.create();
		Entity entity = { id, this };

		entity.addComponent<IDComponent>().id = uuid;
		entity.addComponent<TransformComponent>();

		auto& tag = entity.addComponent<TagComponent>();
		tag.tag = name.empty() ? std::string("Entity ") + std::to_string((uint64_t)uuid) : name;

		return entity;
	}

	Entity Scene::duplicateEntity(Entity source) {
		if (!source)
			return {};

		Entity newEntity = createEntity(source.getComponent<TagComponent>().tag + " (2)");

		CopyComponentIfExists<TransformComponent>(newEntity, source);
		CopyComponentIfExists<SpriteRendererComponent>(newEntity, source);
		CopyComponentIfExists<CircleRendererComponent>(newEntity, source);
		CopyComponentIfExists<CameraComponent>(newEntity, source);
		CopyComponentIfExists<NativeScriptComponent>(newEntity, source);
		CopyComponentIfExists<RigidBody2DComponent>(newEntity, source);
		CopyComponentIfExists<BoxCollider2DComponent>(newEntity, source);
		CopyComponentIfExists<CircleCollider2DComponent>(newEntity, source);

		return newEntity;
	}

	void Scene::destroyEntity(Entity entity) {
		if (!entity)
			return;
		m_Registry.destroy(entity);
	}

	void Scene::onRuntimeStart() {
		// Init all Script objects that extend Entity
		/*for (const auto& desc : ScriptManager::getChildrenOf("Entity")) {
			//ScriptManager::createObject(desc);
			SHADO_CORE_INFO("{0}", desc.toString());
		}*/
		
		auto view = m_Registry.view<ScriptComponent>();
		for (auto e : view) {
			Entity entity = { e, this };

			auto& script = entity.getComponent<ScriptComponent>();

			// Create object based on class
			script.object = ScriptManager::createObject(script.klass);

			// TODO THIS IS A TERRIBLE SOLUTION CHANGE IT ASAP
			auto& obj = script.object;
			obj.invokeMethod(obj.getMethod("set_Id"), &entity.getComponent<IDComponent>().id);

			// create C# Scene object
			void* args[] = { this };
			SHADO_CORE_INFO("Scene: {0}", (int)this);
			auto sceneObj = ScriptManager::createObject(ScriptManager::getClassByName("Scene"), ".ctor(intptr)", args);
			obj.invokeMethod(obj.getMethod("set_Scene"), sceneObj.getNative());

			auto* create = script.object.getMethod("OnCreate");

			if (create != nullptr)
				script.object.invokeMethod(create);
		}

		// TODO make the physics adjustable
		softResetPhysics();
	}

	void Scene::onRuntimeStop() {
		auto view = m_Registry.view<ScriptComponent>();
		for (auto e : view) {
			Entity entity = { e, this };

			auto& script = entity.getComponent<ScriptComponent>();

			auto* destroy = script.object.getMethod("OnDestroyed");
			if (destroy != nullptr)
				script.object.invokeMethod(destroy);
		}
		
		delete m_World;
		m_World = nullptr;
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

		// Update physics
		if (m_PhysicsEnabled) {
			const int32_t velocityIterations = 6;
			const int32_t positionIterations = 2;
			m_World->Step(ts, velocityIterations, positionIterations);

			// Get transforms from box 2d
			auto view = m_Registry.view<RigidBody2DComponent>();
			for (auto e : view) {
				Entity entity = { e, this };

				auto& transform = entity.getComponent<TransformComponent>();
				auto& rb2D = entity.getComponent<RigidBody2DComponent>();

				b2Body* body = (b2Body*)rb2D.runtimeBody;
				transform.position.x = body->GetPosition().x;
				transform.position.y = body->GetPosition().y;
				transform.rotation.z = body->GetAngle();
			}
		}

		// Update C# script
		auto scriptView = m_Registry.view<ScriptComponent>();
		for (auto e : scriptView) {
			Entity entity = { e, this };

			auto& script = entity.getComponent<ScriptComponent>();
			void* args[] = {
				&ts
			};

			auto* update = script.object.getMethod("OnUpdate");
			if (update != nullptr)
				script.object.invokeMethod(update, args);
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
					primaryCamera = camera.camera.get();
					cameraTransform = transform.getTransform();
					break;
				}
			}
		}

		
		if (primaryCamera) {
			Renderer2D::BeginScene(*primaryCamera, cameraTransform);

			// Render stuff
			auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
			for (auto entity : group) {
				auto [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);

				if (!sprite.shader)
					Renderer2D::DrawSprite(transform.getTransform(), sprite, (int)entity);
				else
					Renderer2D::DrawQuad(transform.getTransform(), sprite.shader, (int)entity);
			}

			// Draw circles
			{
				auto view = m_Registry.view<TransformComponent, CircleRendererComponent>();
				for (auto entity : view)
				{
					auto [transform, circle] = view.get<TransformComponent, CircleRendererComponent>(entity);

					if (circle.texture)
						Renderer2D::DrawCircle(transform.getTransform(), circle.texture, circle.tilingFactor, circle.color, circle.thickness, circle.fade, (int)entity);
					else
						Renderer2D::DrawCircle(transform.getTransform(), circle.color, circle.thickness, circle.fade, (int)entity);
				}
			}

			Renderer2D::EndScene();
		}	
	}

	void Scene::onUpdateEditor(TimeStep ts, EditorCamera& camera) {
	}

	void Scene::onDrawEditor(EditorCamera& camera) {

		Renderer2D::BeginScene(camera);

		// Render Quads stuff
		auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
		for (auto entity : group) {
			auto [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);

			if (!sprite.shader)
				Renderer2D::DrawSprite(transform.getTransform(), sprite, (int)entity);
			else
				Renderer2D::DrawQuad(transform.getTransform(), sprite.shader, (int)entity);
		}

		// Draw circles
		{
			auto view = m_Registry.view<TransformComponent, CircleRendererComponent>();
			for (auto entity : view)
			{
				auto [transform, circle] = view.get<TransformComponent, CircleRendererComponent>(entity);

				if (circle.texture)
					Renderer2D::DrawCircle(transform.getTransform(), circle.texture, circle.tilingFactor, circle.color, circle.thickness, circle.fade, (int)entity);
				else
					Renderer2D::DrawCircle(transform.getTransform(), circle.color, circle.thickness, circle.fade, (int)entity);
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
				return { entity, this };			
		}
		return {};
	}

	Entity Scene::getEntityById(uint64_t __id) {
		auto ids = m_Registry.view<IDComponent>();
		for (auto entity : ids) {
			const auto& id = ids.get<IDComponent>(entity);

			if (id.id == __id)
				return { entity, this };
		}
		return {};
	}

	void Scene::softResetPhysics() {
		if (m_World) {
			delete m_World;
			m_World = nullptr;
		}

		// TODO make the physics adjustable
		m_World = new b2World({ 0.0f, -9.8f });

		auto view = m_Registry.view<RigidBody2DComponent>();
		for (auto e : view) {
			Entity entity = { e, this };

			auto& transform = entity.getComponent<TransformComponent>();
			auto& rb2D = entity.getComponent<RigidBody2DComponent>();


			b2BodyDef bodyDef;
			bodyDef.type = (b2BodyType)rb2D.type;	// TODO : maybe change this
			bodyDef.fixedRotation = rb2D.fixedRotation;
			bodyDef.position.Set(transform.position.x, transform.position.y);
			bodyDef.angle = transform.rotation.z;

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

			if (entity.hasComponent<CircleCollider2DComponent>())
			{
				auto& cc2d = entity.getComponent<CircleCollider2DComponent>();

				b2CircleShape circleShape;
				circleShape.m_p.Set(cc2d.offset.x, cc2d.offset.y);
				circleShape.m_radius = transform.scale.x * cc2d.radius;

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

	// Helpers
	template<typename Component>
	static void CopyComponent(entt::registry& dst, entt::registry& src, const std::unordered_map<UUID, entt::entity>& enttMap)
	{
		auto view = src.view<Component>();
		for (auto e : view)
		{
			UUID uuid = src.get<IDComponent>(e).id;
			SHADO_CORE_ASSERT(enttMap.find(uuid) != enttMap.end(), "");
			entt::entity dstEnttID = enttMap.at(uuid);

			auto& component = src.get<Component>(e);
			dst.emplace_or_replace<Component>(dstEnttID, component);
		}
	}

	template<typename Component>
	static void CopyComponentIfExists(Entity dst, Entity src)
	{
		if (src.hasComponent<Component>())
			dst.addOrReplaceComponent<Component>(src.getComponent<Component>());
	}
}
