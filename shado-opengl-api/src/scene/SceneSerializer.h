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
        /// @param prefab
        /// @return The UUID of the prefab
        UUID serializePrefab(Ref<Prefab> prefab);

        bool deserialize(const std::string& filepath, std::string& error);
        bool deserialize(const std::string& filepath);
        bool deserializeRuntime(const std::string& filepath);
        Ref<Prefab> deserializePrefab(const std::string& filepath);

    private:
        /// 
        /// @param out 
        /// @param entity 
        /// @param scriptStorageContext This param is needed to serialize prefabs or scene
        /// @param endmap 
        void SerializeEntity(YAML::Emitter& out, Entity entity, ScriptStorage& scriptStorageContext,
                             bool endmap = true);
        void serializePrefabHelper(YAML::Emitter& out, Entity& e, Ref<Prefab> prefab);

        Entity deserializePrefabHelper(YAML::Node node, Ref<Prefab> prefab);
        Entity dererializeEntityHelper(
            const YAML::Node& entity,
            std::function<Entity(std::string, UUID)> entityCreatorFunction,
            Ref<Scene> scene,
            ScriptStorage& scriptStorageContext
        );

    private:
        Ref<Scene> m_Scene;
    };
}
