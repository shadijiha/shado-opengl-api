#pragma once
#include "Scene.h"

namespace YAML {
    class Emitter;
    class Node;
}

namespace Shado {
    class Prefab;

    class SceneSerializer {
    public:
        SceneSerializer(const Ref<Scene>& scene);
        SceneSerializer(const SceneSerializer&) = delete;

        void serialize(const std::string& filepath);
        void serializeRuntime(const std::string& filepath);

        /// Saves prefab to disk. If prefabId is not provided, a new UUID is generated
        /// otherwise old prefab is overwritten
        /// @param entity The root entity of the prefab
        /// @param prefabId The UUID of the prefab
        /// @return The UUID of the prefab
        UUID serializePrefab(Entity entity, UUID prefabId = UUID());

        bool deserialize(const std::string& filepath, std::string& error);
        bool deserialize(const std::string& filepath);
        bool deserializeRuntime(const std::string& filepath);
        Ref<Prefab> deserializePrefab(const std::string& filepath);

    private:
        void SerializeEntity(YAML::Emitter& out, Entity entity, bool endmap = true);
        void serializePrefabHelper(YAML::Emitter& out, Entity& e, UUID prefabId);

        Entity deserializePrefabHelper(YAML::Node node, Ref<Prefab> prefab);
        Entity dererializeEntityHelper(
            const YAML::Node& entity,
            std::function<Entity(std::string, UUID)> entityCreatorFunction,
            Ref<Scene> scene
        );

    private:
        Ref<Scene> m_Scene;
    };
}
