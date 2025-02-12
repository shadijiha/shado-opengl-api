#include "SceneSerializer.h"
#include "debug/Profile.h"
#include <yaml-cpp/yaml.h>
#include <fstream>

#include "Components.h"
#include "project/Project.h"
#include "asset/AssetManager.h" // <--- This is needed DO NOT REMOVE
#include "script/ScriptEngine.h"
#include "Prefab.h"
#include "math/Hash.h"

namespace YAML {
    template <>
    struct convert<glm::vec2> {
        static Node encode(const glm::vec2& rhs) {
            Node node;
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            node.SetStyle(EmitterStyle::Flow);
            return node;
        }

        static bool decode(const Node& node, glm::vec2& rhs) {
            if (!node.IsSequence() || node.size() != 2)
                return false;

            rhs.x = node[0].as<float>();
            rhs.y = node[1].as<float>();
            return true;
        }
    };

    template <>
    struct convert<glm::vec3> {
        static Node encode(const glm::vec3& rhs) {
            Node node;
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            node.push_back(rhs.z);
            node.SetStyle(EmitterStyle::Flow);
            return node;
        }

        static bool decode(const Node& node, glm::vec3& rhs) {
            if (!node.IsSequence() || node.size() != 3)
                return false;

            rhs.x = node[0].as<float>();
            rhs.y = node[1].as<float>();
            rhs.z = node[2].as<float>();
            return true;
        }
    };

    template <>
    struct convert<glm::vec4> {
        static Node encode(const glm::vec4& rhs) {
            Node node;
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            node.push_back(rhs.z);
            node.push_back(rhs.w);
            node.SetStyle(EmitterStyle::Flow);
            return node;
        }

        static bool decode(const Node& node, glm::vec4& rhs) {
            if (!node.IsSequence() || node.size() != 4)
                return false;

            rhs.x = node[0].as<float>();
            rhs.y = node[1].as<float>();
            rhs.z = node[2].as<float>();
            rhs.w = node[3].as<float>();
            return true;
        }
    };

    template <>
    struct convert<Shado::UUID> {
        static Node encode(const Shado::UUID& uuid) {
            Node node;
            node.push_back((uint64_t)uuid);
            return node;
        }

        static bool decode(const Node& node, Shado::UUID& uuid) {
            uuid = node.as<uint64_t>();
            return true;
        }
    };
}

namespace Shado {
    YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec2& v) {
        out << YAML::Flow;
        out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
        return out;
    }

    YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec3& v) {
        out << YAML::Flow;
        out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
        return out;
    }

    YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec4& v) {
        out << YAML::Flow;
        out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
        return out;
    }

    static std::string getPathAndCopyFileToAssets(const std::string& path);

    SceneSerializer::SceneSerializer(const Ref<Scene>& scene)
        : m_Scene(scene) {
    }

    void SceneSerializer::serialize(const std::string& filepath) {
        SHADO_PROFILE_FUNCTION();

        YAML::Emitter out;
        out << YAML::BeginMap;
        out << YAML::Key << "Scene" << YAML::Value << m_Scene->name;
        out << YAML::Key << "Version" << YAML::Value << SHADO_SCENE_VERSION;
        out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;
        m_Scene->m_Registry.each([&](auto entityID) {
            Entity entity = {entityID, m_Scene.Raw()};
            if (!entity)
                return;

            SerializeEntity(out, entity, m_Scene->m_ScriptStorage);
        });
        out << YAML::EndSeq;
        out << YAML::EndMap;

        std::ofstream fout(filepath);
        fout << out.c_str();
    }

    void SceneSerializer::serializeRuntime(const std::string& filepath) {
        SHADO_CORE_ASSERT(false, "Not implemented");
    }

    void SceneSerializer::serializePrefabHelper(YAML::Emitter& out, Entity& e, Ref<Prefab> prefab) {
        // Add a unique ID to every entity
        if (!e.hasComponent<PrefabInstanceComponent>()) {
            auto& prefabComponent = e.addComponent<PrefabInstanceComponent>();
            prefabComponent.prefabId = prefab->GetId();
            prefabComponent.prefabEntityUniqueId = UUID();
        }

        SerializeEntity(out, e, prefab->GetScriptStorage(), false);

        auto children = e.getChildren();

        if (!children.empty()) {
            out << YAML::Key << "Children" << YAML::Value << YAML::BeginSeq;
            for (auto& child : children) {
                serializePrefabHelper(out, child, prefab);
            }
            out << YAML::EndSeq;
        }

        out << YAML::EndMap;
    }

    UUID SceneSerializer::serializePrefab(Ref<Prefab> prefab) {
        SHADO_PROFILE_FUNCTION();

        YAML::Emitter out;
        out << YAML::BeginMap;

        out << YAML::Key << "PrefabId" << YAML::Value << prefab->GetId();
        out << YAML::Key << "Data" << YAML::Value;

        // Recursively serialize children
        serializePrefabHelper(out, prefab->root, prefab);

        out << YAML::EndMap;

        // TODO: Once we have an asset manager, remove the prefab ID from the filename and
        // make this the duty of the asset manager
        std::ofstream fout(
            Project::GetAssetDirectory() / (std::to_string(prefab->GetId()) + ".prefab"));
        fout << out.c_str();

        SHADO_CORE_ASSERT(prefab->root.hasComponent<PrefabInstanceComponent>(),
                          "Entity must have a PrefabInstanceComponent by now");

        return prefab->GetId();
    }

    bool SceneSerializer::deserialize(const std::filesystem::path& filepath) {
        std::string ignored;
        return this->deserialize(filepath, ignored);
    }

    bool SceneSerializer::deserialize(const std::filesystem::path& filepath, std::string& error) {
        SHADO_PROFILE_FUNCTION();

        YAML::Node data;
        try {
            data = YAML::LoadFile(filepath.string());


            if (!data["Scene"]) {
                error = "File does not contain a \"Scene\" object";
                return false;
            }

            std::string sceneName = data["Scene"].as<std::string>();
            SHADO_CORE_TRACE("Deserializing scene '{0}'", sceneName);
            m_Scene->name = sceneName;

            std::string version = data["Version"].as<std::string>();
            SHADO_CORE_ASSERT(version == SHADO_SCENE_VERSION, "Scene version mismatch. Expected {} got {}", SHADO_SCENE_VERSION, version);

            auto entities = data["Entities"];
            if (entities) {
                for (auto entity : entities) {
                    dererializeEntityHelper(entity, [this](std::string name, UUID uuid) {
                        return m_Scene->createEntityWithUUID(name, uuid);
                    }, m_Scene, m_Scene->m_ScriptStorage);
                }
            }
        }
        catch (const YAML::Exception& e) {
            error = e.what();
            return false;
        }

        return true;
    }

    bool SceneSerializer::deserializeRuntime(const std::filesystem::path& filepath) {
        SHADO_CORE_ASSERT(false, "Not implemented");
        return false;
    }

    Entity SceneSerializer::deserializePrefabHelper(YAML::Node node, Ref<Prefab> prefab) {
        Entity entity = dererializeEntityHelper(node, [&prefab](std::string name, UUID id) {
            Entity e = prefab->attachEntity(id);
            e.getComponent<TagComponent>().tag = name;
            return e;
        }, Scene::ActiveScene, prefab->GetScriptStorage());

        // Deserialize children
        auto children = node["Children"];
        if (children) {
            for (auto child : children) {
                deserializePrefabHelper(child, prefab);
            }
        }

        return entity;
    }

    Ref<Prefab> SceneSerializer::deserializePrefab(const std::string& filepath) {
        SHADO_PROFILE_FUNCTION();

        YAML::Node data;
        try {
            data = YAML::LoadFile(filepath);
            UUID prefabId = data["PrefabId"].as<uint64_t>();
            Ref<Prefab> prefab = CreateRef<Prefab>(prefabId);

            prefab->root = deserializePrefabHelper(data["Data"], prefab);

            // Make sure the prefab root component has the correct prefab id
            prefab->root.getComponent<PrefabInstanceComponent>().prefabId = prefabId;

            return prefab;
        }
        catch (const YAML::Exception& e) {
            SHADO_CORE_ERROR("{0}", e.what());
            return nullptr;
        }
    }

    void SceneSerializer::SerializeEntity(YAML::Emitter& out, Entity entity, ScriptStorage& scriptStorageContext,
                                          bool endmap) {
        SHADO_CORE_ASSERT(entity.hasComponent<IDComponent>(), "No ID component");

        out << YAML::BeginMap; // Entity
        out << YAML::Key << "Entity" << YAML::Value << entity.getComponent<IDComponent>().id;
        // TODO: Entity ID goes here

        if (entity.hasComponent<TagComponent>()) {
            out << YAML::Key << "TagComponent";
            out << YAML::BeginMap; // TagComponent

            auto& tag = entity.getComponent<TagComponent>().tag;
            out << YAML::Key << "Tag" << YAML::Value << tag;

            out << YAML::EndMap; // TagComponent
        }

        if (entity.hasComponent<TransformComponent>()) {
            out << YAML::Key << "TransformComponent";
            out << YAML::BeginMap; // TransformComponent

            auto& tc = entity.getComponent<TransformComponent>();
            out << YAML::Key << "Translation" << YAML::Value << tc.position;
            out << YAML::Key << "Rotation" << YAML::Value << tc.rotation;
            out << YAML::Key << "Scale" << YAML::Value << tc.scale;
            out << YAML::Key << "ParentEntityId" << YAML::Value << tc.parentId;

            out << YAML::EndMap; // TransformComponent
        }

        if (entity.hasComponent<CameraComponent>()) {
            out << YAML::Key << "CameraComponent";
            out << YAML::BeginMap; // CameraComponent

            auto& cameraComponent = entity.getComponent<CameraComponent>();
            auto& camera = cameraComponent.camera;

            out << YAML::Key << "Camera" << YAML::Value;
            out << YAML::BeginMap; // Camera
            out << YAML::Key << "ProjectionType" << YAML::Value << (int)cameraComponent.type;

            switch (cameraComponent.type) {
            case CameraComponent::Type::Orthographic:
                out << YAML::Key << "OrthographicSize" << YAML::Value << cameraComponent.getSize();
                break;
            case CameraComponent::Type::Orbit:

                OrbitCamera* orbitCam = (OrbitCamera*)camera.Raw();
                out << YAML::Key << "PerspectiveFOV" << YAML::Value << orbitCam->getFOV();
                break;
            }
            out << YAML::Key << "Near" << YAML::Value << camera->getNearClip();
            out << YAML::Key << "Far" << YAML::Value << camera->getFarClip();
            out << YAML::EndMap; // Camera

            out << YAML::Key << "Primary" << YAML::Value << cameraComponent.primary;
            out << YAML::Key << "FixedAspectRatio" << YAML::Value << cameraComponent.fixedAspectRatio;

            out << YAML::EndMap; // CameraComponent
        }

        if (entity.hasComponent<SpriteRendererComponent>()) {
            out << YAML::Key << "SpriteRendererComponent";
            out << YAML::BeginMap; // SpriteRendererComponent

            auto& spriteRendererComponent = entity.getComponent<SpriteRendererComponent>();
            out << YAML::Key << "Color" << YAML::Value << spriteRendererComponent.color;
            out << YAML::Key << "TextureHandle" << YAML::Value << spriteRendererComponent.texture;
            out << YAML::Key << "TilingFactor" << YAML::Value << spriteRendererComponent.tilingFactor;
            out << YAML::Key << "ShaderHandle" << YAML::Value << spriteRendererComponent.shader;

            //     auto& shader = spriteRendererComponent.shader;
            //     out << YAML::Key << "ShaderCustomUniforms" << YAML::Value;
            //     out << YAML::BeginMap;
            //     for (const auto& [name, value] : shader->m_CustomUniforms) {
            //         auto [type, data] = value;
            //         out << YAML::Key << name << YAML::Value;
            //         out << YAML::BeginMap;
            //         out << YAML::Key << "Type" << YAML::Value << (int)type;
            //         switch (type) {
            //         case ShaderDataType::Int:
            //             out << YAML::Key << "Data" << YAML::Value << *(int*)data;
            //             break;
            //         case ShaderDataType::Float:
            //             out << YAML::Key << "Data" << YAML::Value << *(float*)data;
            //             break;
            //         case ShaderDataType::Float2:
            //             out << YAML::Key << "Data" << YAML::Value << *(glm::vec2*)data;
            //             break;
            //         case ShaderDataType::Float3:
            //             out << YAML::Key << "Data" << YAML::Value << *(glm::vec3*)data;
            //             break;
            //         case ShaderDataType::Float4:
            //             out << YAML::Key << "Data" << YAML::Value << *(glm::vec4*)data;
            //             break;
            //         }
            //         out << YAML::EndMap;
            //     }
            //     out << YAML::EndMap;
            // }

            out << YAML::EndMap; // SpriteRendererComponent
        }

        if (entity.hasComponent<CircleRendererComponent>()) {
            out << YAML::Key << "CircleRendererComponent";
            out << YAML::BeginMap; // CircleRendererComponent

            auto& circleRendererComponent = entity.getComponent<CircleRendererComponent>();

            out << YAML::Key << "Color" << YAML::Value << circleRendererComponent.color;
            out << YAML::Key << "TextureHandle" << YAML::Value << circleRendererComponent.texture;
            out << YAML::Key << "TilingFactor" << YAML::Value << circleRendererComponent.tilingFactor;

            out << YAML::Key << "Thickness" << YAML::Value << circleRendererComponent.thickness;
            out << YAML::Key << "Fade" << YAML::Value << circleRendererComponent.fade;

            out << YAML::Key << "ShaderHandle" << YAML::Value << circleRendererComponent.shader;

            out << YAML::EndMap; // CircleRendererComponent
        }

        if (entity.hasComponent<LineRendererComponent>()) {
            out << YAML::Key << "LineRendererComponent";
            out << YAML::BeginMap; // LineRendererComponent

            auto& lineRendererComponent = entity.getComponent<LineRendererComponent>();
            out << YAML::Key << "Color" << YAML::Value << lineRendererComponent.color;
            out << YAML::Key << "Target" << YAML::Value << lineRendererComponent.target;

            out << YAML::EndMap; // LineRendererComponent
        }

        if (entity.hasComponent<RigidBody2DComponent>()) {
            out << YAML::Key << "RigidBody2DComponent";
            out << YAML::BeginMap; // RigidBody2DComponent

            auto& rigidBocyComponent = entity.getComponent<RigidBody2DComponent>();
            out << YAML::Key << "Type" << YAML::Value << (int)rigidBocyComponent.type;
            out << YAML::Key << "FixedRotation" << YAML::Value << rigidBocyComponent.fixedRotation;

            out << YAML::EndMap; // RigidBody2DComponent
        }

        if (entity.hasComponent<BoxCollider2DComponent>()) {
            out << YAML::Key << "BoxCollider2DComponent";
            out << YAML::BeginMap; // BoxCollider2DComponent

            auto& boxColliderComponent = entity.getComponent<BoxCollider2DComponent>();
            out << YAML::Key << "Offset" << YAML::Value << boxColliderComponent.offset;
            out << YAML::Key << "Size" << YAML::Value << boxColliderComponent.size;
            out << YAML::Key << "Density" << YAML::Value << boxColliderComponent.density;
            out << YAML::Key << "Friction" << YAML::Value << boxColliderComponent.friction;
            out << YAML::Key << "Restitution" << YAML::Value << boxColliderComponent.restitution;
            out << YAML::Key << "RestitutionThreshold" << YAML::Value << boxColliderComponent.restitutionThreshold;

            out << YAML::EndMap; // BoxCollider2DComponent
        }

        if (entity.hasComponent<CircleCollider2DComponent>()) {
            out << YAML::Key << "CircleCollider2DComponent";
            out << YAML::BeginMap; // BoxCollider2DComponent

            auto& circleCollider2DComponent = entity.getComponent<CircleCollider2DComponent>();
            out << YAML::Key << "Offset" << YAML::Value << circleCollider2DComponent.offset;
            out << YAML::Key << "Radius" << YAML::Value << circleCollider2DComponent.radius;
            out << YAML::Key << "Density" << YAML::Value << circleCollider2DComponent.density;
            out << YAML::Key << "Friction" << YAML::Value << circleCollider2DComponent.friction;
            out << YAML::Key << "Restitution" << YAML::Value << circleCollider2DComponent.restitution;
            out << YAML::Key << "RestitutionThreshold" << YAML::Value << circleCollider2DComponent.restitutionThreshold;

            out << YAML::EndMap; // circleCollider2DComponent
        }

        if (entity.hasComponent<ScriptComponent>()) {
            out << YAML::Key << "ScriptComponent";
            out << YAML::BeginMap; // ScriptComponent

            const auto& scriptEngine = ScriptEngine::GetInstance();
            const auto& sc = entity.getComponent<ScriptComponent>();

            if (scriptEngine.IsValidScript(sc.ScriptID)) {
                const auto& scriptMetadata = scriptEngine.GetScriptMetadata(sc.ScriptID);
                const auto& entityStorage = scriptStorageContext.EntityStorage.at(entity.getUUID());

                out << YAML::Key << "ScriptID" << YAML::Value << sc.ScriptID;
                out << YAML::Key << "ScriptName" << YAML::Value << scriptMetadata.FullName;

                out << YAML::Key << "Fields" << YAML::Value << YAML::BeginSeq;
                for (const auto& [fieldID, fieldStorage] : entityStorage.Fields) {
                    const auto& fieldMetadata = scriptMetadata.Fields.at(fieldID);

                    out << YAML::BeginMap;
                    out << YAML::Key << "ID" << YAML::Value << fieldID;
                    out << YAML::Key << "Name" << YAML::Value << fieldMetadata.Name;
                    out << YAML::Key << "Type" << YAML::Value << (int)fieldMetadata.Type;
                    //std::string(magic_enum::enum_name(fieldMetadata.Type));
                    out << YAML::Key << "Value" << YAML::Value;

                    if (fieldStorage.IsArray()) {
                        out << YAML::BeginSeq;

                        for (int32_t i = 0; i < fieldStorage.GetLength(); i++) {
                            switch (fieldMetadata.Type) {
                            case DataType::Bool:
                                out << fieldStorage.GetValue<bool>(i);
                                break;
                            case DataType::SByte:
                                out << fieldStorage.GetValue<int8_t>(i);
                                break;
                            case DataType::Byte:
                                out << fieldStorage.GetValue<uint8_t>(i);
                                break;
                            case DataType::Short:
                                out << fieldStorage.GetValue<int16_t>(i);
                                break;
                            case DataType::UShort:
                                out << fieldStorage.GetValue<uint16_t>(i);
                                break;
                            case DataType::Int:
                                out << fieldStorage.GetValue<int32_t>(i);
                                break;
                            case DataType::UInt:
                                out << fieldStorage.GetValue<uint32_t>(i);
                                break;
                            case DataType::Long:
                                out << fieldStorage.GetValue<int64_t>(i);
                                break;
                            case DataType::ULong:
                                out << fieldStorage.GetValue<uint64_t>(i);
                                break;
                            case DataType::Float:
                                out << fieldStorage.GetValue<float>(i);
                                break;
                            case DataType::Double:
                                out << fieldStorage.GetValue<double>(i);
                                break;
                            case DataType::Vector2:
                                out << fieldStorage.GetValue<glm::vec2>(i);
                                break;
                            case DataType::Vector3:
                                out << fieldStorage.GetValue<glm::vec3>(i);
                                break;
                            case DataType::Vector4:
                                out << fieldStorage.GetValue<glm::vec4>(i);
                                break;
                            case DataType::String:
                                out << fieldStorage.GetValue<std::string>(i);
                                break;
                            case DataType::Entity:
                                out << fieldStorage.GetValue<uint64_t>(i);
                                break;
                            case DataType::Prefab:
                                out << fieldStorage.GetValue<uint64_t>(i);
                                break;
                            case DataType::Mesh:
                                out << fieldStorage.GetValue<uint64_t>(i);
                                break;
                            case DataType::StaticMesh:
                                out << fieldStorage.GetValue<uint64_t>(i);
                                break;
                            case DataType::Material:
                                out << fieldStorage.GetValue<uint64_t>(i);
                                break;
                            case DataType::Texture2D:
                                out << fieldStorage.GetValue<uint64_t>(i);
                                break;
                            case DataType::Scene:
                                out << fieldStorage.GetValue<uint64_t>(i);
                                break;
                            default:
                                break;
                            }
                        }

                        out << YAML::EndSeq;
                    }
                    else {
                        switch (fieldMetadata.Type) {
                        case DataType::Bool:
                            out << fieldStorage.GetValue<bool>();
                            break;
                        case DataType::SByte:
                            out << fieldStorage.GetValue<int8_t>();
                            break;
                        case DataType::Byte:
                            out << fieldStorage.GetValue<uint8_t>();
                            break;
                        case DataType::Short:
                            out << fieldStorage.GetValue<int16_t>();
                            break;
                        case DataType::UShort:
                            out << fieldStorage.GetValue<uint16_t>();
                            break;
                        case DataType::Int:
                            out << fieldStorage.GetValue<int32_t>();
                            break;
                        case DataType::UInt:
                            out << fieldStorage.GetValue<uint32_t>();
                            break;
                        case DataType::Long:
                            out << fieldStorage.GetValue<int64_t>();
                            break;
                        case DataType::ULong:
                            out << fieldStorage.GetValue<uint64_t>();
                            break;
                        case DataType::Float:
                            out << fieldStorage.GetValue<float>();
                            break;
                        case DataType::Double:
                            out << fieldStorage.GetValue<double>();
                            break;
                        case DataType::Vector2:
                            out << fieldStorage.GetValue<glm::vec2>();
                            break;
                        case DataType::Vector3:
                            out << fieldStorage.GetValue<glm::vec3>();
                            break;
                        case DataType::Vector4:
                            out << fieldStorage.GetValue<glm::vec4>();
                            break;
                        // TODO(Emily): This appears to write a spurious `\x00`
                        case DataType::String:
                            out << fieldStorage.GetValue<std::string>();
                            break;
                        case DataType::Entity:
                            out << fieldStorage.GetValue<uint64_t>();
                            break;
                        case DataType::Prefab:
                            out << fieldStorage.GetValue<uint64_t>();
                            break;
                        case DataType::Mesh:
                            out << fieldStorage.GetValue<uint64_t>();
                            break;
                        case DataType::StaticMesh:
                            out << fieldStorage.GetValue<uint64_t>();
                            break;
                        case DataType::Material:
                            out << fieldStorage.GetValue<uint64_t>();
                            break;
                        case DataType::Texture2D:
                            out << fieldStorage.GetValue<uint64_t>();
                            break;
                        case DataType::Scene:
                            out << fieldStorage.GetValue<uint64_t>();
                            break;
                        default:
                            break;
                        }
                    }

                    out << YAML::EndMap;
                }
                out << YAML::EndSeq;
            }

            out << YAML::EndMap; // ScriptComponent
        }

        if (entity.hasComponent<PrefabInstanceComponent>()) {
            auto& prefabComponent = entity.getComponent<PrefabInstanceComponent>();

            out << YAML::Key << "PrefabInstanceComponent";
            out << YAML::BeginMap; // PrefabInstanceComponent
            out << YAML::Key << "PrefabId" << YAML::Value << prefabComponent.prefabId;
            out << YAML::Key << "PrefabEntityUniqueId" << YAML::Value << prefabComponent.prefabEntityUniqueId;

            out << YAML::EndMap; // PrefabInstanceComponent
        }

        if (entity.hasComponent<TextComponent>()) {
            out << YAML::Key << "TextRendererComponent";
            out << YAML::BeginMap; // TextRendererComponent

            out << YAML::Key << "Text" << YAML::Value << entity.getComponent<TextComponent>().text;
            out << YAML::Key << "Color" << YAML::Value << entity.getComponent<TextComponent>().color;
            out << YAML::Key << "LineSpacing" << YAML::Value << entity.getComponent<TextComponent>().
                                                                       lineSpacing;
            out << YAML::Key << "Kerning" << YAML::Value << entity.getComponent<TextComponent>().kerning;

            // TODO: Once we have asset handles, this should be a UUID
            out << YAML::Key << "Font" << YAML::Value << entity.getComponent<TextComponent>().font->getPath().
                                                                string();

            out << YAML::EndMap; // TextRendererComponent
        }

        if (endmap)
            out << YAML::EndMap; // Entity
    }

    static std::string getPathAndCopyFileToAssets(const std::string& path) {
        // Convert absolute path to a relative path to Project::GetAtive()->GetProjectPath()
        if (!Project::GetActive() || path == "NULL")
            return path;

        // If file is abolute path
        auto absoluePath = std::filesystem::absolute(path);
        auto projectPath = Project::GetActive()->GetProjectDirectory();

        // Check if file is already in project dir
        if (absoluePath.string().rfind(projectPath.string()) == 0) {
            return absoluePath.string().substr(projectPath.string().length() + 1);
        }

        // Otherwise, copy it to Assets and return the new path
        std::error_code ec;
        auto filename = absoluePath.filename();
        auto newFilePath = Project::GetAssetDirectory() / filename;

        // Check if new file already exists
        if (std::filesystem::exists(newFilePath)) {
            newFilePath = Project::GetAssetDirectory() / (filename.stem().string() + "_" +
                std::to_string(std::time(nullptr)) + filename.extension().string());
        }
        std::filesystem::copy_file(absoluePath, newFilePath, std::filesystem::copy_options::overwrite_existing, ec);

        // Return newFilePath relative to the projectPath
        return newFilePath.string().substr(projectPath.string().length() + 1);
    }

    Entity SceneSerializer::dererializeEntityHelper(
        const YAML::Node& entity,
        std::function<Entity(std::string, UUID)> entityCreatorFunction,
        Ref<Scene> scene, ScriptStorage& scriptStorageContext
    ) {
        uint64_t uuid = entity["Entity"].as<uint64_t>(); // TODO

        std::string name;
        auto tagComponent = entity["TagComponent"];
        if (tagComponent)
            name = tagComponent["Tag"].as<std::string>();

        SHADO_CORE_TRACE("Deserialized entity with ID = {0}, name = {1}", uuid, name);

        Entity deserializedEntity = entityCreatorFunction(name, uuid);

        auto transformComponent = entity["TransformComponent"];
        if (transformComponent) {
            // Entities always have transforms
            auto& tc = deserializedEntity.getComponent<TransformComponent>();
            tc.position = transformComponent["Translation"].as<glm::vec3>();
            tc.rotation = transformComponent["Rotation"].as<glm::vec3>();
            tc.scale = transformComponent["Scale"].as<glm::vec3>();

            // Note: the reason this is this way is because if the serialization order is random then 
            // the child will be loaded before the parent and getEntityById will return invalid entity
            // even though the child had a valid parent on serialization
            if (transformComponent["ParentEntityId"]) {
                tc.parentId = transformComponent["ParentEntityId"].as<uint64_t>();
            }
        }

        auto cameraComponent = entity["CameraComponent"];
        if (cameraComponent) {
            auto cameraProps = cameraComponent["Camera"];
            CameraComponent::Type type = (CameraComponent::Type)cameraProps["ProjectionType"].as<int>();

            auto& cc = deserializedEntity.addComponent<CameraComponent>(type, scene->getViewport().x,
                                                                        scene->getViewport().y);
            cc.camera->setNearClip(cameraProps["Near"].as<float>());
            cc.camera->setFarClip(cameraProps["Far"].as<float>());


            switch (type) {
            case CameraComponent::Type::Orthographic:
                cc.setSize(cameraProps["OrthographicSize"].as<float>());
                break;
            case CameraComponent::Type::Orbit:
                OrbitCamera* orbitCam = (OrbitCamera*)cc.camera.Raw();
                orbitCam->setFOV(cameraProps["PerspectiveFOV"].as<float>());
                break;
            }

            cc.primary = cameraComponent["Primary"].as<bool>();
            cc.fixedAspectRatio = cameraComponent["FixedAspectRatio"].as<bool>();
        }

        auto spriteRendererComponent = entity["SpriteRendererComponent"];
        if (spriteRendererComponent) {
            auto& src = deserializedEntity.addComponent<SpriteRendererComponent>();
            src.color = spriteRendererComponent["Color"].as<glm::vec4>();
            src.tilingFactor = spriteRendererComponent["TilingFactor"].as<float>();

            if (spriteRendererComponent["TextureHandle"])
                src.texture = spriteRendererComponent["TextureHandle"].as<AssetHandle>();

            if (spriteRendererComponent["ShaderHandle"]) {
                try {
                    src.shader = spriteRendererComponent["ShaderHandle"].as<AssetHandle>();

                    // if (auto shaderCustomUniforms = spriteRendererComponent["ShaderCustomUniforms"]) {
                    //     for (auto uniform : shaderCustomUniforms) {
                    //         std::string name = uniform.first.as<std::string>();
                    //         auto uniformData = uniform.second;
                    //
                    //         ShaderDataType type = (ShaderDataType)uniformData["Type"].as<int>();
                    //         switch (type) {
                    //         case ShaderDataType::Int: {
                    //             int data = uniformData["Data"].as<int>();
                    //             src.shader->setInt(name, data);
                    //             src.shader->saveCustomUniformValue(name, type, data);
                    //             break;
                    //         }
                    //         case ShaderDataType::Float: {
                    //             float data = uniformData["Data"].as<float>();
                    //             src.shader->setFloat(name, data);
                    //             src.shader->saveCustomUniformValue(name, type, data);
                    //             break;
                    //         }
                    //         case ShaderDataType::Float2: {
                    //             auto data = uniformData["Data"].as<glm::vec2>();
                    //             src.shader->setFloat2(name, data);
                    //             src.shader->saveCustomUniformValue(name, type, data);
                    //             break;
                    //         }
                    //         case ShaderDataType::Float3: {
                    //             auto data = uniformData["Data"].as<glm::vec3>();
                    //             src.shader->setFloat3(name, data);
                    //             src.shader->saveCustomUniformValue(name, type, data);
                    //             break;
                    //         }
                    //         case ShaderDataType::Float4: {
                    //             auto data = uniformData["Data"].as<glm::vec4>();
                    //             src.shader->setFloat4(name, data);
                    //             src.shader->saveCustomUniformValue(name, type, data);
                    //             break;
                    //         }
                    //         }
                    //     }
                    // }
                }
                catch (const std::runtime_error& e) {
                    SHADO_CORE_ERROR("Error loading shader: {0}", e.what());
                }
            }
        }

        auto circleRendererComponent = entity["CircleRendererComponent"];
        if (circleRendererComponent) {
            auto& src = deserializedEntity.addComponent<CircleRendererComponent>();

            src.color = circleRendererComponent["Color"].as<glm::vec4>();
            src.thickness = circleRendererComponent["Thickness"].as<float>();
            src.fade = circleRendererComponent["Fade"].as<float>();
            src.tilingFactor = circleRendererComponent["TilingFactor"].as<float>();

            if (circleRendererComponent["TextureHandle"])
                src.texture = circleRendererComponent["TextureHandle"].as<AssetHandle>();

            if (circleRendererComponent["ShaderHandle"]) {
                try {
                    src.shader = circleRendererComponent["ShaderHandle"].as<AssetHandle>();
                }
                catch (const std::runtime_error& e) {
                    SHADO_CORE_ERROR("Error loading shader: {0}", e.what());
                }
            }
        }

        auto lineRendererComponent = entity["LineRendererComponent"];
        if (lineRendererComponent) {
            auto& src = deserializedEntity.addComponent<LineRendererComponent>();
            src.color = lineRendererComponent["Color"].as<glm::vec4>();
            src.target = lineRendererComponent["Target"].as<glm::vec3>();
        }

        auto rigidBodyComponent = entity["RigidBody2DComponent"];
        if (rigidBodyComponent) {
            auto& src = deserializedEntity.addComponent<RigidBody2DComponent>();
            src.type = (RigidBody2DComponent::BodyType)rigidBodyComponent["Type"].as<int>();
            src.fixedRotation = rigidBodyComponent["FixedRotation"].as<bool>();
        }

        auto boxColliderComponent = entity["BoxCollider2DComponent"];
        if (boxColliderComponent) {
            auto& src = deserializedEntity.addComponent<BoxCollider2DComponent>();
            src.offset = boxColliderComponent["Offset"].as<glm::vec2>();
            src.size = boxColliderComponent["Size"].as<glm::vec2>();
            src.density = boxColliderComponent["Density"].as<float>();
            src.friction = boxColliderComponent["Friction"].as<float>();
            src.restitution = boxColliderComponent["Restitution"].as<float>();
            src.restitutionThreshold = boxColliderComponent["RestitutionThreshold"].as<float>();
        }

        auto circleCollider2DComponent = entity["CircleCollider2DComponent"];
        if (circleCollider2DComponent) {
            auto& src = deserializedEntity.addComponent<CircleCollider2DComponent>();
            src.offset = circleCollider2DComponent["Offset"].as<glm::vec2>();

            glm::vec2 tempRadius = circleCollider2DComponent["Radius"].as<glm::vec2>();
            src.radius = glm::vec2(tempRadius);

            src.density = circleCollider2DComponent["Density"].as<float>();
            src.friction = circleCollider2DComponent["Friction"].as<float>();
            src.restitution = circleCollider2DComponent["Restitution"].as<float>();
            src.restitutionThreshold = circleCollider2DComponent["RestitutionThreshold"].as<float>();
        }

        auto scriptComponent = entity["ScriptComponent"];
        if (scriptComponent) {
            try {
                uint64_t scriptID = scriptComponent["ScriptID"].as<uint64_t>(0);

                if (scriptID == 0) {
                    scriptID = scriptComponent["ClassHandle"].as<uint64_t>(0);
                }

                if (scriptID != 0) {
                    auto& scriptEngine = ScriptEngine::GetMutable();

                    if (scriptEngine.IsValidScript(scriptID)) {
                        const auto& scriptMetadata = scriptEngine.GetScriptMetadata(scriptID);

                        ScriptComponent& sc = deserializedEntity.addComponent<ScriptComponent>();
                        sc.ScriptID = scriptID;

                        scriptStorageContext.InitializeEntityStorage(scriptID, deserializedEntity.getUUID());;

                        auto fieldsArray = scriptComponent["Fields"];
                        for (auto field : fieldsArray) {
                            uint32_t fieldID = field["ID"].as<uint32_t>(0);
                            auto fieldName = field["Name"].as<std::string>("");

                            if (scriptMetadata.Fields.contains(fieldID)) {
                                const auto& fieldMetadata = scriptMetadata.Fields.at(fieldID);
                                auto& fieldStorage = scriptStorageContext.EntityStorage.at(
                                    deserializedEntity.getUUID()).Fields[fieldID];

                                auto valueNode = field["Value"];

                                if (fieldStorage.IsArray()) {
                                    SHADO_CORE_ASSERT(valueNode.IsSequence(), "");
                                    fieldStorage.Resize(valueNode.size());

                                    for (int32_t i = 0; i < valueNode.size(); i++) {
                                        switch (fieldMetadata.Type) {
                                        case DataType::Bool: {
                                            fieldStorage.SetValue(valueNode[i].as<bool>());
                                            break;
                                        }
                                        case DataType::SByte: {
                                            fieldStorage.SetValue(valueNode[i].as<int8_t>(), i);
                                            break;
                                        }
                                        case DataType::Byte: {
                                            fieldStorage.SetValue(valueNode[i].as<uint8_t>(), i);
                                            break;
                                        }
                                        case DataType::Short: {
                                            fieldStorage.SetValue(valueNode[i].as<int16_t>(), i);
                                            break;
                                        }
                                        case DataType::UShort: {
                                            fieldStorage.SetValue(valueNode[i].as<uint16_t>(), i);
                                            break;
                                        }
                                        case DataType::Int: {
                                            fieldStorage.SetValue(valueNode[i].as<int32_t>(), i);
                                            break;
                                        }
                                        case DataType::UInt: {
                                            fieldStorage.SetValue(valueNode[i].as<uint32_t>(), i);
                                            break;
                                        }
                                        case DataType::Long: {
                                            fieldStorage.SetValue(valueNode[i].as<int64_t>(), i);
                                            break;
                                        }
                                        case DataType::ULong: {
                                            fieldStorage.SetValue(valueNode[i].as<uint64_t>(), i);
                                            break;
                                        }
                                        case DataType::Float: {
                                            fieldStorage.SetValue(valueNode[i].as<float>(), i);
                                            break;
                                        }
                                        case DataType::Double: {
                                            fieldStorage.SetValue(valueNode[i].as<double>(), i);
                                            break;
                                        }
                                        case DataType::Vector2: {
                                            fieldStorage.SetValue(valueNode[i].as<glm::vec2>(), i);
                                            break;
                                        }
                                        case DataType::Vector3: {
                                            fieldStorage.SetValue(valueNode[i].as<glm::vec3>(), i);
                                            break;
                                        }
                                        case DataType::Vector4: {
                                            fieldStorage.SetValue(valueNode[i].as<glm::vec4>(), i);
                                            break;
                                        }
                                        case DataType::String: {
                                            fieldStorage.SetValue(valueNode[i].as<std::string>(), i);
                                            break;
                                        }
                                        case DataType::Entity: {
                                            fieldStorage.SetValue(valueNode[i].as<uint64_t>(), i);
                                            break;
                                        }
                                        case DataType::Prefab: {
                                            fieldStorage.SetValue(valueNode[i].as<uint64_t>(), i);
                                            break;
                                        }
                                        case DataType::Mesh: {
                                            fieldStorage.SetValue(valueNode[i].as<uint64_t>(), i);
                                            break;
                                        }
                                        case DataType::StaticMesh: {
                                            fieldStorage.SetValue(valueNode[i].as<uint64_t>(), i);
                                            break;
                                        }
                                        case DataType::Material: {
                                            fieldStorage.SetValue(valueNode[i].as<uint64_t>(), i);
                                            break;
                                        }
                                        case DataType::Texture2D: {
                                            fieldStorage.SetValue(valueNode[i].as<uint64_t>(), i);
                                            break;
                                        }
                                        case DataType::Scene: {
                                            fieldStorage.SetValue(valueNode[i].as<uint64_t>(), i);
                                            break;
                                        }
                                        default:
                                            break;
                                        }
                                    }
                                }
                                else {
                                    switch (fieldMetadata.Type) {
                                    case DataType::Bool: {
                                        fieldStorage.SetValue(valueNode.as<bool>());
                                        break;
                                    }
                                    case DataType::SByte: {
                                        fieldStorage.SetValue(valueNode.as<int8_t>());
                                        break;
                                    }
                                    case DataType::Byte: {
                                        fieldStorage.SetValue(valueNode.as<uint8_t>());
                                        break;
                                    }
                                    case DataType::Short: {
                                        fieldStorage.SetValue(valueNode.as<int16_t>());
                                        break;
                                    }
                                    case DataType::UShort: {
                                        fieldStorage.SetValue(valueNode.as<uint16_t>());
                                        break;
                                    }
                                    case DataType::Int: {
                                        fieldStorage.SetValue(valueNode.as<int32_t>());
                                        break;
                                    }
                                    case DataType::UInt: {
                                        fieldStorage.SetValue(valueNode.as<uint32_t>());
                                        break;
                                    }
                                    case DataType::Long: {
                                        fieldStorage.SetValue(valueNode.as<int64_t>());
                                        break;
                                    }
                                    case DataType::ULong: {
                                        fieldStorage.SetValue(valueNode.as<uint64_t>());
                                        break;
                                    }
                                    case DataType::Float: {
                                        fieldStorage.SetValue(valueNode.as<float>());
                                        break;
                                    }
                                    case DataType::Double: {
                                        fieldStorage.SetValue(valueNode.as<double>());
                                        break;
                                    }
                                    case DataType::Vector2: {
                                        fieldStorage.SetValue(valueNode.as<glm::vec2>());
                                        break;
                                    }
                                    case DataType::Vector3: {
                                        fieldStorage.SetValue(valueNode.as<glm::vec3>());
                                        break;
                                    }
                                    case DataType::Vector4: {
                                        fieldStorage.SetValue(valueNode.as<glm::vec4>());
                                        break;
                                    }
                                    case DataType::String: {
                                        fieldStorage.SetValue(valueNode.as<std::string>());
                                        break;
                                    }
                                    case DataType::Entity: {
                                        fieldStorage.SetValue(valueNode.as<uint64_t>());
                                        break;
                                    }
                                    case DataType::Prefab: {
                                        fieldStorage.SetValue(valueNode.as<uint64_t>());
                                        break;
                                    }
                                    case DataType::Mesh: {
                                        fieldStorage.SetValue(valueNode.as<uint64_t>());
                                        break;
                                    }
                                    case DataType::StaticMesh: {
                                        fieldStorage.SetValue(valueNode.as<uint64_t>());
                                        break;
                                    }
                                    case DataType::Material: {
                                        fieldStorage.SetValue(valueNode.as<uint64_t>());
                                        break;
                                    }
                                    case DataType::Texture2D: {
                                        fieldStorage.SetValue(valueNode.as<uint64_t>());
                                        break;
                                    }
                                    case DataType::Scene: {
                                        fieldStorage.SetValue(valueNode.as<uint64_t>());
                                        break;
                                    }
                                    default:
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
                else {
                    SHADO_CORE_ERROR(
                        "Failed to deserialize ScriptComponent on entity '{}', script id of 0 is not valid.",
                        uuid);
                }
            }
            catch (const std::exception& e) {
                SHADO_CORE_ERROR("Failed to deserialize ScriptComponent on entity '{}' : {}", uuid,
                                 e.what());
            }
        }

        auto prefabComponent = entity["PrefabInstanceComponent"];
        if (prefabComponent) {
            auto& sc = deserializedEntity.addOrReplaceComponent<PrefabInstanceComponent>();
            sc.prefabId = prefabComponent["PrefabId"].as<uint64_t>();
            sc.prefabEntityUniqueId = prefabComponent["PrefabEntityUniqueId"].as<uint64_t>();
        }

        auto textRendererComponent = entity["TextRendererComponent"];
        if (textRendererComponent) {
            auto& trc = deserializedEntity.addComponent<TextComponent>();
            trc.text = textRendererComponent["Text"].as<std::string>();
            trc.color = textRendererComponent["Color"].as<glm::vec4>();
            trc.lineSpacing = textRendererComponent["LineSpacing"].as<float>();
            trc.kerning = textRendererComponent["Kerning"].as<float>();
            trc.font = CreateRef<Font>(textRendererComponent["Font"].as<std::string>());
        }

        return deserializedEntity;
    }
}
