#pragma once
#include "Components.h"
#include "SceneSerializer.h"
#include "project/Project.h"

namespace Shado {
    class Prefab : public RefCounted {
    public:
        Prefab(UUID prefabUUID) : prefabId(prefabUUID) {
        }

        Prefab(UUID prefabUUID, entt::registry& _registry, UUID rootPrefabEntityUniqueId,
               ScriptStorage& scriptStorageToCopy)
            : prefabId(prefabUUID) {
            CopyRegistries(_registry, registry, [this](const auto& tag, UUID uuid) {
                entt::entity enttId = this->registry.create();
                auto entity = Entity(enttId, &this->registry);
                entity.addComponent<TagComponent>().tag = tag;
                entity.addOrReplaceComponent<IDComponent>().id = uuid;
                return entity;
            });

            // Find root
            auto view = registry.view<PrefabInstanceComponent>();
            for (auto entity : view) {
                auto& component = view.get<PrefabInstanceComponent>(entity);
                if (component.prefabEntityUniqueId == rootPrefabEntityUniqueId) {
                    root = {entity, &registry};
                    break;
                }
            }

            // Set the new registry as context to all entities in the prefab
            root.setRegistry(&this->registry);
            scriptStorageToCopy.CopyTo(this->scriptStorage);
        }

        template <typename T>
        void addComponent(Entity& entity, const T& toCopy) const {
            entity.addOrReplaceComponent(toCopy);
        }

        Entity attachEntity(UUID entityId) {
            entt::entity handle = registry.create();
            Entity e = {handle, &registry};
            e.addComponent<IDComponent>().id = entityId;
            e.addComponent<TagComponent>();
            e.addComponent<TransformComponent>();

            auto& component = e.addComponent<PrefabInstanceComponent>();
            component.prefabId = prefabId;
            component.prefabEntityUniqueId = 0;

            return e;
        }

        UUID GetId() const { return prefabId; }

        Entity GetEntityByElementUniqueId(UUID id) {
            auto view = registry.view<PrefabInstanceComponent>();
            for (auto entity : view) {
                auto& component = view.get<PrefabInstanceComponent>(entity);
                if (component.prefabEntityUniqueId == id) {
                    return {entity, &registry};
                }
            }
            return {};
        }

        ScriptStorage& GetScriptStorage() { return scriptStorage; }

        static Ref<Prefab> CreateFromEntity(Entity entity, Scene& scene) {
            Ref<Prefab> newPrefab = CreateRef<Prefab>(UUID());
            newPrefab->root = scene.duplicateEntity(entity);

            // Copy script storage to prefab
            Scene::ActiveScene->GetScriptStorage().CopyTo(newPrefab->GetScriptStorage());

            SceneSerializer serializer(Scene::ActiveScene);
            serializer.serializePrefab(newPrefab);

            // Update the entitie's prefab component
            auto& prefabComponent = entity.addOrReplaceComponent<PrefabInstanceComponent>();
            prefabComponent.prefabId = newPrefab->GetId();
            prefabComponent.prefabEntityUniqueId = newPrefab->root.getUUID();

            return newPrefab;
        }

        static Ref<Prefab> CreateFromPath(const std::filesystem::path& path) {
            if (IsPrefabPath(path)) {
                UUID prefabId = std::stoull(path.filename().replace_extension());
                return GetPrefabById(prefabId);
            }
            return nullptr;
        }

        static Ref<Prefab> GetPrefabById(UUID id) {
            static std::mutex mutex;

            if (loadedPrefabs.find(id) == loadedPrefabs.end()) {
                // TODO: once we have an asset manager, offload this duty to it
                const std::filesystem::path path = Project::GetAssetDirectory() / (std::to_string(id) + ".prefab");
                if (std::filesystem::exists(path)) {
                    SceneSerializer serializer(Scene::ActiveScene);
                    Ref<Prefab> prefab = serializer.deserializePrefab(path.string());
                    {
                        std::lock_guard lock(mutex);
                        loadedPrefabs[id] = prefab;
                    }
                }
                else {
                    return nullptr;
                }
            }
            return loadedPrefabs[id];
        }

        static void UpdatedLoadedPrefabs(Ref<Prefab> replacementPrefab) {
            loadedPrefabs[replacementPrefab->GetId()] = replacementPrefab;
        }

        static bool IsLoaded(UUID id) {
            return loadedPrefabs.find(id) != loadedPrefabs.end();
        }

        static bool IsPrefabPath(const std::filesystem::path& path) {
            return path.extension() == ".prefab";
        }

        static UUID PrefabPathToId(const std::filesystem::path& path) {
            if (!IsPrefabPath(path)) {
                return 0;
            }
            return std::stoull(path.filename().replace_extension());
        }

    public:
        Entity root;

    private:
        entt::registry registry;
        ScriptStorage scriptStorage;
        UUID prefabId;

        inline static std::unordered_map<UUID, Ref<Prefab>> loadedPrefabs;

        friend class Scene;
        friend class SceneInfoPanel;
    };

    class PrefabEditorContextChanged : public Event {
    public:
        PrefabEditorContextChanged(Ref<Prefab> prefab) : m_Prefab(prefab) {
        }

        EVENT_CLASS_TYPE(PrefabEditorContextChanged);
        EVENT_CLASS_CATEGORY(EventCategoryEditor);

        Ref<Prefab> m_Prefab;
    };
}
