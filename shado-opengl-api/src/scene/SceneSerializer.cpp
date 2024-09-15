#include "SceneSerializer.h"
#include "debug/Profile.h"
#include <yaml-cpp/yaml.h>
#include <fstream>

#include "Components.h"
#include "project/Project.h"
#include "script/ScriptEngine.h"
#include "Prefab.h"

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
#define WRITE_SCRIPT_FIELD(FieldType, Type)           \
			case ScriptFieldType::FieldType:          \
				out << scriptField.GetValue<Type>();  \
				break

#define READ_SCRIPT_FIELD(FieldType, Type)             \
	case ScriptFieldType::FieldType:                   \
	{                                                  \
		Type data = scriptField["Data"].as<Type>();    \
		fieldInstance.SetValue(data);                  \
		break;                                         \
	}

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
        out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;
        m_Scene->m_Registry.each([&](auto entityID) {
            Entity entity = {entityID, m_Scene.Raw()};
            if (!entity)
                return;

            SerializeEntity(out, entity);
        });
        out << YAML::EndSeq;
        out << YAML::EndMap;

        std::ofstream fout(filepath);
        fout << out.c_str();
    }

    void SceneSerializer::serializeRuntime(const std::string& filepath) {
        SHADO_CORE_ASSERT(false, "Not implemented");
    }

    void SceneSerializer::serializePrefabHelper(YAML::Emitter& out, Entity& e, UUID prefabId) {
        // Add a unique ID to every entity
        if (!e.hasComponent<PrefabInstanceComponent>()) {
            auto& prefabComponent = e.addComponent<PrefabInstanceComponent>();
            prefabComponent.prefabId = prefabId;
            prefabComponent.prefabEntityUniqueId = UUID();
        }

        SerializeEntity(out, e, false);

        auto children = e.getChildren();

        if (!children.empty()) {
            out << YAML::Key << "Children" << YAML::Value << YAML::BeginSeq;
            for (auto& child : children) {
                serializePrefabHelper(out, child, prefabId);
            }
            out << YAML::EndSeq;
        }

        out << YAML::EndMap;
    }

    UUID SceneSerializer::serializePrefab(Entity entity, UUID prefabId) {
        SHADO_PROFILE_FUNCTION();

        YAML::Emitter out;
        out << YAML::BeginMap;

        out << YAML::Key << "PrefabId" << YAML::Value << (uint64_t)prefabId;
        out << YAML::Key << "Data" << YAML::Value;

        // Recursively serialize children
        serializePrefabHelper(out, entity, prefabId);

        out << YAML::EndMap;

        // TODO: Once we have an asset manager, remove the prefab ID from the filename and
        // make this the duty of the asset manager
        std::ofstream fout(
            Project::GetAssetDirectory() / (std::to_string((uint64_t)prefabId) + ".prefab"));
        fout << out.c_str();

        SHADO_CORE_ASSERT(entity.hasComponent<PrefabInstanceComponent>(),
                          "Entity must have a PrefabInstanceComponent by now");

        return prefabId;
    }

    bool SceneSerializer::deserialize(const std::string& filepath) {
        std::string ignored;
        return this->deserialize(filepath, ignored);
    }

    bool SceneSerializer::deserialize(const std::string& filepath, std::string& error) {
        SHADO_PROFILE_FUNCTION();

        YAML::Node data;
        try {
            data = YAML::LoadFile(filepath);


            if (!data["Scene"]) {
                error = "File does not contain a \"Scene\" object";
                return false;
            }

            std::string sceneName = data["Scene"].as<std::string>();
            SHADO_CORE_TRACE("Deserializing scene '{0}'", sceneName);
            m_Scene->name = sceneName;

            auto entities = data["Entities"];
            if (entities) {
                for (auto entity : entities) {
                    dererializeEntityHelper(entity, [this](std::string name, UUID uuid) {
                        return m_Scene->createEntityWithUUID(name, uuid);
                    }, m_Scene);
                }
            }
        }
        catch (const YAML::Exception& e) {
            error = e.what();
            return false;
        }

        return true;
    }

    bool SceneSerializer::deserializeRuntime(const std::string& filepath) {
        SHADO_CORE_ASSERT(false, "Not implemented");
        return false;
    }

    Entity SceneSerializer::deserializePrefabHelper(YAML::Node node, Ref<Prefab> prefab) {
        Entity entity = dererializeEntityHelper(node, [&prefab](std::string name, UUID id) {
            Entity e = prefab->attachEntity(id);
            e.getComponent<TagComponent>().tag = name;
            return e;
        }, Scene::ActiveScene);

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

            return prefab;
        }
        catch (const YAML::Exception& e) {
            SHADO_CORE_ERROR("{0}", e.what());
            return nullptr;
        }
    }

    void SceneSerializer::SerializeEntity(YAML::Emitter& out, Entity entity, bool endmap) {
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
            std::string texturePath = spriteRendererComponent.texture
                                          ? spriteRendererComponent.texture->getFilePath()
                                          : "NULL";
            std::string newPath = getPathAndCopyFileToAssets(texturePath);
            out << YAML::Key << "Color" << YAML::Value << spriteRendererComponent.color;
            out << YAML::Key << "Texture" << YAML::Value << newPath;
            out << YAML::Key << "TillingFactor" << YAML::Value << spriteRendererComponent.tilingFactor;

            if (spriteRendererComponent.shader) {
                out << YAML::Key << "Shader" << YAML::Value << Project::GetActive()->GetRelativePath(
                    spriteRendererComponent.shader->getFilepath()).string();

                auto& shader = spriteRendererComponent.shader;
                out << YAML::Key << "ShaderCustomUniforms" << YAML::Value;
                out << YAML::BeginMap;
                for (const auto& [name, value] : shader->m_CustomUniforms) {
                    auto [type, data] = value;
                    out << YAML::Key << name << YAML::Value;
                    out << YAML::BeginMap;
                    out << YAML::Key << "Type" << YAML::Value << (int)type;
                    switch (type) {
                    case ShaderDataType::Int:
                        out << YAML::Key << "Data" << YAML::Value << *(int*)data;
                        break;
                    case ShaderDataType::Float:
                        out << YAML::Key << "Data" << YAML::Value << *(float*)data;
                        break;
                    case ShaderDataType::Float2:
                        out << YAML::Key << "Data" << YAML::Value << *(glm::vec2*)data;
                        break;
                    case ShaderDataType::Float3:
                        out << YAML::Key << "Data" << YAML::Value << *(glm::vec3*)data;
                        break;
                    case ShaderDataType::Float4:
                        out << YAML::Key << "Data" << YAML::Value << *(glm::vec4*)data;
                        break;
                    }
                    out << YAML::EndMap;
                }
                out << YAML::EndMap;
            }

            out << YAML::EndMap; // SpriteRendererComponent

            // if Texture path has changed, then update the component
            if (newPath != texturePath) {
                spriteRendererComponent.texture = Texture2D::create(Project::GetProjectDirectory() / newPath);
            }
        }

        if (entity.hasComponent<CircleRendererComponent>()) {
            out << YAML::Key << "CircleRendererComponent";
            out << YAML::BeginMap; // CircleRendererComponent

            auto& circleRendererComponent = entity.getComponent<CircleRendererComponent>();
            std::string texturePath = circleRendererComponent.texture
                                          ? circleRendererComponent.texture->getFilePath()
                                          : "NULL";
            std::string newPath = getPathAndCopyFileToAssets(texturePath);

            out << YAML::Key << "Color" << YAML::Value << circleRendererComponent.color;
            out << YAML::Key << "Texture" << YAML::Value << getPathAndCopyFileToAssets(texturePath);
            out << YAML::Key << "TillingFactor" << YAML::Value << circleRendererComponent.tilingFactor;

            out << YAML::Key << "Thickness" << YAML::Value << circleRendererComponent.thickness;
            out << YAML::Key << "Fade" << YAML::Value << circleRendererComponent.fade;

            if (circleRendererComponent.shader)
                out << YAML::Key << "Shader" << YAML::Value << Project::GetActive()->GetRelativePath(
                    circleRendererComponent.shader->getFilepath()).string();

            out << YAML::EndMap; // CircleRendererComponent

            // if Texture path has changed, then update the component
            if (newPath != texturePath) {
                circleRendererComponent.texture = Texture2D::create(Project::GetProjectDirectory() / newPath);
            }
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
            auto& scriptComponent = entity.getComponent<ScriptComponent>();

            out << YAML::Key << "ScriptComponent";
            out << YAML::BeginMap; // ScriptComponent
            out << YAML::Key << "ClassName" << YAML::Value << scriptComponent.ClassName;

            // Fields
            Ref<ScriptClass> entityClass = ScriptEngine::GetEntityClass(scriptComponent.ClassName);
            if (entityClass) {
                const auto& fields = entityClass->GetFields();
                if (!fields.empty()) {
                    out << YAML::Key << "ScriptFields" << YAML::Value;
                    auto& entityFields = ScriptEngine::GetScriptFieldMap(entity);
                    out << YAML::BeginSeq;
                    for (const auto& [name, field] : fields) {
                        if (entityFields.find(name) == entityFields.end())
                            continue;

                        out << YAML::BeginMap; // ScriptField
                        out << YAML::Key << "Name" << YAML::Value << name;
                        out << YAML::Key << "Type" << YAML::Value << Utils::ScriptFieldTypeToString(field.Type);

                        out << YAML::Key << "Data" << YAML::Value;
                        ScriptFieldInstance& scriptField = entityFields.at(name);

                        switch (field.Type) {
                        WRITE_SCRIPT_FIELD(Float, float);
                        WRITE_SCRIPT_FIELD(Double, double);
                        WRITE_SCRIPT_FIELD(Bool, bool);
                        WRITE_SCRIPT_FIELD(Char, char);
                        WRITE_SCRIPT_FIELD(Byte, int8_t);
                        WRITE_SCRIPT_FIELD(Short, int16_t);
                        WRITE_SCRIPT_FIELD(Int, int32_t);
                        WRITE_SCRIPT_FIELD(Long, int64_t);
                        WRITE_SCRIPT_FIELD(UByte, uint8_t);
                        WRITE_SCRIPT_FIELD(UShort, uint16_t);
                        WRITE_SCRIPT_FIELD(UInt, uint32_t);
                        WRITE_SCRIPT_FIELD(ULong, uint64_t);
                        WRITE_SCRIPT_FIELD(Vector2, glm::vec2);
                        WRITE_SCRIPT_FIELD(Vector3, glm::vec3);
                        WRITE_SCRIPT_FIELD(Vector4, glm::vec4);
                        WRITE_SCRIPT_FIELD(Colour, glm::vec4);
                        WRITE_SCRIPT_FIELD(Entity, UUID);
                        WRITE_SCRIPT_FIELD(Prefab, UUID);
                        }
                        out << YAML::EndMap; // ScriptFields
                    }
                    out << YAML::EndSeq;
                }
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
        Ref<Scene> scene
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
            auto texturePath = spriteRendererComponent["Texture"].as<std::string>();
            src.color = spriteRendererComponent["Color"].as<glm::vec4>();
            src.texture = texturePath == "NULL" || texturePath == "null"
                              ? nullptr
                              : new Texture2D(
                                  Project::GetActive()
                                      ? (Project::GetProjectDirectory() / texturePath).string()
                                      : texturePath
                              );
            src.tilingFactor = spriteRendererComponent["TillingFactor"].as<float>();

            if (spriteRendererComponent["Shader"]) {
                try {
                    src.shader = CreateRef<Shader>(
                        Project::GetProjectDirectory() / spriteRendererComponent["Shader"].as<std::string>());

                    if (auto shaderCustomUniforms = spriteRendererComponent["ShaderCustomUniforms"]) {
                        for (auto uniform : shaderCustomUniforms) {
                            std::string name = uniform.first.as<std::string>();
                            auto uniformData = uniform.second;

                            ShaderDataType type = (ShaderDataType)uniformData["Type"].as<int>();
                            switch (type) {
                            case ShaderDataType::Int: {
                                int data = uniformData["Data"].as<int>();
                                src.shader->setInt(name, data);
                                src.shader->saveCustomUniformValue(name, type, data);
                                break;
                            }
                            case ShaderDataType::Float: {
                                float data = uniformData["Data"].as<float>();
                                src.shader->setFloat(name, data);
                                src.shader->saveCustomUniformValue(name, type, data);
                                break;
                            }
                            case ShaderDataType::Float2: {
                                auto data = uniformData["Data"].as<glm::vec2>();
                                src.shader->setFloat2(name, data);
                                src.shader->saveCustomUniformValue(name, type, data);
                                break;
                            }
                            case ShaderDataType::Float3: {
                                auto data = uniformData["Data"].as<glm::vec3>();
                                src.shader->setFloat3(name, data);
                                src.shader->saveCustomUniformValue(name, type, data);
                                break;
                            }
                            case ShaderDataType::Float4: {
                                auto data = uniformData["Data"].as<glm::vec4>();
                                src.shader->setFloat4(name, data);
                                src.shader->saveCustomUniformValue(name, type, data);
                                break;
                            }
                            }
                        }
                    }
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

            auto texturePath = circleRendererComponent["Texture"].as<std::string>();
            src.texture = texturePath == "NULL" || texturePath == "null"
                              ? nullptr
                              : new Texture2D(
                                  Project::GetActive()
                                      ? (Project::GetProjectDirectory() / texturePath).string()
                                      : texturePath
                              );
            src.tilingFactor = circleRendererComponent["TillingFactor"].as<float>();

            if (circleRendererComponent["Shader"]) {
                try {
                    src.shader = CreateRef<Shader>(
                        Project::GetProjectDirectory() / circleRendererComponent["Shader"].as<std::string>());
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
            auto& sc = deserializedEntity.addComponent<ScriptComponent>();
            sc.ClassName = scriptComponent["ClassName"].as<std::string>();

            auto scriptFields = scriptComponent["ScriptFields"];
            if (scriptFields) {
                Ref<ScriptClass> entityClass = ScriptEngine::GetEntityClass(sc.ClassName);
                if (entityClass) {
                    const auto& fields = entityClass->GetFields();
                    auto& entityFields = ScriptEngine::GetScriptFieldMap(deserializedEntity);

                    for (auto scriptField : scriptFields) {
                        std::string name = scriptField["Name"].as<std::string>();
                        std::string typeString = scriptField["Type"].as<std::string>();
                        ScriptFieldType type = Utils::ScriptFieldTypeFromString(typeString);

                        ScriptFieldInstance& fieldInstance = entityFields[name];

                        SHADO_CORE_WARN("Could not find field {0} in script class {1}",
                                        name, sc.ClassName);

                        if (fields.find(name) == fields.end())
                            continue;

                        fieldInstance.Field = fields.at(name);

                        switch (type) {
                        READ_SCRIPT_FIELD(Float, float);
                        READ_SCRIPT_FIELD(Double, double);
                        READ_SCRIPT_FIELD(Bool, bool);
                        READ_SCRIPT_FIELD(Char, char);
                        READ_SCRIPT_FIELD(Byte, int8_t);
                        READ_SCRIPT_FIELD(Short, int16_t);
                        READ_SCRIPT_FIELD(Int, int32_t);
                        READ_SCRIPT_FIELD(Long, int64_t);
                        READ_SCRIPT_FIELD(UByte, uint8_t);
                        READ_SCRIPT_FIELD(UShort, uint16_t);
                        READ_SCRIPT_FIELD(UInt, uint32_t);
                        READ_SCRIPT_FIELD(ULong, uint64_t);
                        READ_SCRIPT_FIELD(Vector2, glm::vec2);
                        READ_SCRIPT_FIELD(Vector3, glm::vec3);
                        READ_SCRIPT_FIELD(Vector4, glm::vec4);
                        READ_SCRIPT_FIELD(Colour, glm::vec4);
                        READ_SCRIPT_FIELD(Entity, UUID);
                        READ_SCRIPT_FIELD(Prefab, uint64_t);
                        }
                    }
                }
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
