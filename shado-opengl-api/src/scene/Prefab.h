#pragma once
#include "Components.h"
#include "SceneSerializer.h"
#include "project/Project.h"

namespace Shado {

	class Prefab : public RefCounted {
	public:
		Prefab(UUID id) : prefabId(id) {}

		template<typename T>
		void addComponent(Entity& entity, const T& toCopy) const {
			entity.addOrReplaceComponent(toCopy);
		}

		Entity attachEntity(UUID id) {
			entt::entity handle = registry.create();
			Entity e = { handle, &registry };
			e.addComponent<IDComponent>().id = id;
			e.addComponent<TagComponent>();
			e.addComponent<TransformComponent>();
			e.addComponent<PrefabInstanceComponent>().prefabId = prefabId;
			SHADO_CORE_WARN("Attached entity {0}", (uint64_t)e.getUUID());
			return e;
		}

		static Ref<Prefab> Create(UUID id) {
			return CreateRef<Prefab>(id);
		}

		static Ref<Prefab> GetPrefabById(UUID id) {
			if (loadedPrefabs.find(id) == loadedPrefabs.end()) {
				// TODO: once we have an asset manager, offload this duty to it
				const std::filesystem::path path = Project::GetAssetDirectory() / (std::to_string(id) + ".prefab");
				if (std::filesystem::exists(path)) {
					SceneSerializer serializer(Scene::ActiveScene);
					Ref<Prefab> prefab = serializer.deserializePrefab(path.string());
					loadedPrefabs[id] = prefab;
				} else {
					return nullptr;
				}
			}
			return loadedPrefabs[id];
		}

	public:
		Entity root;
	private:
		entt::registry registry;
		
		UUID prefabId;

		inline static std::unordered_map<UUID, Ref<Prefab>> loadedPrefabs;

		friend class Scene;
	};

}
