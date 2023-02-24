#include "SceneSerializer.h"
#include "debug/Profile.h"
#include <yaml-cpp/yaml.h>
#include <fstream>

#include "Components.h"
#include "project/Project.h"
#include "script/ScriptEngine.h"

namespace YAML {
	template<>
	struct convert<glm::vec2>
	{
		static Node encode(const glm::vec2& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, glm::vec2& rhs)
		{
			if (!node.IsSequence() || node.size() != 2)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec3>
	{
		static Node encode(const glm::vec3& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, glm::vec3& rhs)
		{
			if (!node.IsSequence() || node.size() != 3)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec4>
	{
		static Node encode(const glm::vec4& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.push_back(rhs.w);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, glm::vec4& rhs)
		{
			if (!node.IsSequence() || node.size() != 4)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			rhs.w = node[3].as<float>();
			return true;
		}
	};
	
	template<>
	struct convert<Shado::UUID>
	{
		static Node encode(const Shado::UUID& uuid)
		{
			Node node;
			node.push_back((uint64_t)uuid);
			return node;
		}

		static bool decode(const Node& node, Shado::UUID& uuid)
		{
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

	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec2& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec3& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec4& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
		return out;
	}

	static void SerializeEntity(YAML::Emitter& out, Entity entity);

	SceneSerializer::SceneSerializer(const Ref<Scene>& scene)
		: m_Scene(scene)
	{
		
	}
	
	void SceneSerializer::serialize(const std::string& filepath) {
		SHADO_PROFILE_FUNCTION();

		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Scene" << YAML::Value << m_Scene->name;
		out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;
		m_Scene->m_Registry.each([&](auto entityID)
			{
				Entity entity = { entityID, m_Scene.get() };
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

	bool SceneSerializer::deserialize(const std::string& filepath) {
		SHADO_PROFILE_FUNCTION();

		YAML::Node data;
		try
		{
			data = YAML::LoadFile(filepath);
		} catch (YAML::Exception e)
		{
			return false;
		}

		if (!data["Scene"])
			return false;

		std::string sceneName = data["Scene"].as<std::string>();
		SHADO_CORE_TRACE("Deserializing scene '{0}'", sceneName);
		m_Scene->name = sceneName;

		// Load C# assembly
		/*auto assembly = data["C#DLL"];
		if (assembly) {
			ScriptManager::reload(data["C#DLL"].as<std::string>());
		}*/
		
		auto entities = data["Entities"];
		if (entities)
		{
			for (auto entity : entities)
			{
				uint64_t uuid = entity["Entity"].as<uint64_t>(); // TODO

				std::string name;
				auto tagComponent = entity["TagComponent"];
				if (tagComponent)
					name = tagComponent["Tag"].as<std::string>();

				SHADO_CORE_TRACE("Deserialized entity with ID = {0}, name = {1}", uuid, name);

				Entity deserializedEntity = m_Scene->createEntityWithUUID(name, uuid);

				auto transformComponent = entity["TransformComponent"];
				if (transformComponent)
				{
					// Entities always have transforms
					auto& tc = deserializedEntity.getComponent<TransformComponent>();
					tc.position = transformComponent["Translation"].as<glm::vec3>();
					tc.rotation = transformComponent["Rotation"].as<glm::vec3>();
					tc.scale = transformComponent["Scale"].as<glm::vec3>();
				}

				auto cameraComponent = entity["CameraComponent"];
				if (cameraComponent)
				{
					auto& cameraProps = cameraComponent["Camera"];
					CameraComponent::Type type = (CameraComponent::Type)cameraProps["ProjectionType"].as<int>();

					auto& cc = deserializedEntity.addComponent<CameraComponent>(type, m_Scene->m_ViewportWidth, m_Scene->m_ViewportHeight);
					cc.camera->setNearClip(cameraProps["Near"].as<float>());
					cc.camera->setFarClip(cameraProps["Far"].as<float>());
					

					switch (type) {
						case CameraComponent::Type::Orthographic:
							cc.setSize(cameraProps["OrthographicSize"].as<float>());
							break;
						case CameraComponent::Type::Orbit:
							OrbitCamera* orbitCam = (OrbitCamera*)cc.camera.get();
							orbitCam->setFOV(cameraProps["PerspectiveFOV"].as<float>());
							break;
					}

					cc.primary = cameraComponent["Primary"].as<bool>();
					cc.fixedAspectRatio = cameraComponent["FixedAspectRatio"].as<bool>();
				}

				auto spriteRendererComponent = entity["SpriteRendererComponent"];
				if (spriteRendererComponent)
				{
					auto& src = deserializedEntity.addComponent<SpriteRendererComponent>();
					auto texturePath = spriteRendererComponent["Texture"].as<std::string>();
					src.color = spriteRendererComponent["Color"].as<glm::vec4>();
					src.texture = texturePath == "NULL" || texturePath == "null" ? nullptr : CreateRef<Texture2D>(texturePath);
					src.tilingFactor = spriteRendererComponent["TillingFactor"].as<float>();
				}

				auto circleRendererComponent = entity["CircleRendererComponent"];
				if (circleRendererComponent)
				{
					auto& src = deserializedEntity.addComponent<CircleRendererComponent>();
					
					src.color = circleRendererComponent["Color"].as<glm::vec4>();
					src.thickness = circleRendererComponent["Thickness"].as<float>();
					src.fade = circleRendererComponent["Fade"].as<float>();

					auto texturePath = circleRendererComponent["Texture"].as<std::string>();
					src.texture = texturePath == "NULL" || texturePath == "null" ? nullptr : CreateRef<Texture2D>(texturePath);
					src.tilingFactor = circleRendererComponent["TillingFactor"].as<float>();
				}

				auto rigidBodyComponent = entity["RigidBody2DComponent"];
				if (rigidBodyComponent)
				{
					auto& src = deserializedEntity.addComponent<RigidBody2DComponent>();
					src.type = (RigidBody2DComponent::BodyType)rigidBodyComponent["Type"].as<int>();
					src.fixedRotation = rigidBodyComponent["FixedRotation"].as<bool>();
				}

				auto boxColliderComponent = entity["BoxCollider2DComponent"];
				if (boxColliderComponent)
				{
					auto& src = deserializedEntity.addComponent<BoxCollider2DComponent>();
					src.offset = boxColliderComponent["Offset"].as<glm::vec2>();
					src.size = boxColliderComponent["Size"].as<glm::vec2>();
					src.density = boxColliderComponent["Density"].as<float>();
					src.friction = boxColliderComponent["Friction"].as<float>();
					src.restitution = boxColliderComponent["Restitution"].as<float>();
					src.restitutionThreshold = boxColliderComponent["RestitutionThreshold"].as<float>();
				}

				auto circleCollider2DComponent = entity["CircleCollider2DComponent"];
				if (circleCollider2DComponent)
				{
					auto& src = deserializedEntity.addComponent<CircleCollider2DComponent>();
					src.offset = circleCollider2DComponent["Offset"].as<glm::vec2>();

					float tempRadius = circleCollider2DComponent["Radius"].as<float>();
					src.radius = glm::vec2(tempRadius);

					src.density = circleCollider2DComponent["Density"].as<float>();
					src.friction = circleCollider2DComponent["Friction"].as<float>();
					src.restitution = circleCollider2DComponent["Restitution"].as<float>();
					src.restitutionThreshold = circleCollider2DComponent["RestitutionThreshold"].as<float>();
				}

				auto scriptComponent = entity["ScriptComponent"];
				if (scriptComponent)
				{
					auto& sc = deserializedEntity.addComponent<ScriptComponent>();
					sc.ClassName = scriptComponent["ClassName"].as<std::string>();

					auto scriptFields = scriptComponent["ScriptFields"];
					if (scriptFields)
					{
						Ref<ScriptClass> entityClass = ScriptEngine::GetEntityClass(sc.ClassName);
						if (entityClass)
						{
							const auto& fields = entityClass->GetFields();
							auto& entityFields = ScriptEngine::GetScriptFieldMap(deserializedEntity);

							for (auto scriptField : scriptFields)
							{
								std::string name = scriptField["Name"].as<std::string>();
								std::string typeString = scriptField["Type"].as<std::string>();
								ScriptFieldType type = Utils::ScriptFieldTypeFromString(typeString);

								ScriptFieldInstance& fieldInstance = entityFields[name];

								// TODO(Yan): turn this assert into Hazelnut log warning
								SHADO_CORE_ASSERT(fields.find(name) != fields.end(), "");

								if (fields.find(name) == fields.end())
									continue;

								fieldInstance.Field = fields.at(name);

								switch (type)
								{
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
								}
							}
						}
					}
				}
			}
		}

		return true;
	}

	bool SceneSerializer::deserializeRuntime(const std::string& filepath) {
		SHADO_CORE_ASSERT(false, "Not implemented");
		return false;
	}

	// Helpers
	static void SerializeEntity(YAML::Emitter& out, Entity entity)
	{
		SHADO_CORE_ASSERT(entity.hasComponent<IDComponent>(), "No ID component");

		out << YAML::BeginMap; // Entity
		out << YAML::Key << "Entity" << YAML::Value << entity.getComponent<IDComponent>().id; // TODO: Entity ID goes here

		if (entity.hasComponent<TagComponent>())
		{
			out << YAML::Key << "TagComponent";
			out << YAML::BeginMap; // TagComponent

			auto& tag = entity.getComponent<TagComponent>().tag;
			out << YAML::Key << "Tag" << YAML::Value << tag;

			out << YAML::EndMap; // TagComponent
		}

		if (entity.hasComponent<TransformComponent>())
		{
			out << YAML::Key << "TransformComponent";
			out << YAML::BeginMap; // TransformComponent

			auto& tc = entity.getComponent<TransformComponent>();
			out << YAML::Key << "Translation" << YAML::Value << tc.position;
			out << YAML::Key << "Rotation" << YAML::Value << tc.rotation;
			out << YAML::Key << "Scale" << YAML::Value << tc.scale;

			out << YAML::EndMap; // TransformComponent
		}

		if (entity.hasComponent<CameraComponent>())
		{
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

					OrbitCamera* orbitCam = (OrbitCamera*)camera.get();
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

		if (entity.hasComponent<SpriteRendererComponent>())
		{
			out << YAML::Key << "SpriteRendererComponent";
			out << YAML::BeginMap; // SpriteRendererComponent

			auto& spriteRendererComponent = entity.getComponent<SpriteRendererComponent>();
			std::string texturePath = spriteRendererComponent.texture ? spriteRendererComponent.texture->getFilePath() : "NULL";
			out << YAML::Key << "Color" << YAML::Value << spriteRendererComponent.color;
			out << YAML::Key << "Texture" << YAML::Value << texturePath;
			out << YAML::Key << "TillingFactor" << YAML::Value << spriteRendererComponent.tilingFactor;

			out << YAML::EndMap; // SpriteRendererComponent
		}

		if (entity.hasComponent<CircleRendererComponent>())
		{
			out << YAML::Key << "CircleRendererComponent";
			out << YAML::BeginMap; // CircleRendererComponent

			auto& circleRendererComponent = entity.getComponent<CircleRendererComponent>();
			std::string texturePath = circleRendererComponent.texture ? circleRendererComponent.texture->getFilePath() : "NULL";
			out << YAML::Key << "Color" << YAML::Value << circleRendererComponent.color;
			out << YAML::Key << "Texture" << YAML::Value << texturePath;
			out << YAML::Key << "TillingFactor" << YAML::Value << circleRendererComponent.tilingFactor;

			out << YAML::Key << "Thickness" << YAML::Value << circleRendererComponent.thickness;
			out << YAML::Key << "Fade" << YAML::Value << circleRendererComponent.fade;

			out << YAML::EndMap; // CircleRendererComponent
		}

		if (entity.hasComponent<RigidBody2DComponent>())
		{
			out << YAML::Key << "RigidBody2DComponent";
			out << YAML::BeginMap; // RigidBody2DComponent

			auto& rigidBocyComponent = entity.getComponent<RigidBody2DComponent>();
			out << YAML::Key << "Type" << YAML::Value << (int)rigidBocyComponent.type;
			out << YAML::Key << "FixedRotation" << YAML::Value << rigidBocyComponent.fixedRotation;

			out << YAML::EndMap; // RigidBody2DComponent
		}

		if (entity.hasComponent<BoxCollider2DComponent>())
		{
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

		if (entity.hasComponent<CircleCollider2DComponent>())
		{
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

		if (entity.hasComponent<ScriptComponent>())
		{
			auto& scriptComponent = entity.getComponent<ScriptComponent>();

			out << YAML::Key << "ScriptComponent";
			out << YAML::BeginMap; // ScriptComponent
			out << YAML::Key << "ClassName" << YAML::Value << scriptComponent.ClassName;

			// Fields
			Ref<ScriptClass> entityClass = ScriptEngine::GetEntityClass(scriptComponent.ClassName);
			const auto& fields = entityClass->GetFields();
			if (fields.size() > 0)
			{
				out << YAML::Key << "ScriptFields" << YAML::Value;
				auto& entityFields = ScriptEngine::GetScriptFieldMap(entity);
				out << YAML::BeginSeq;
				for (const auto& [name, field] : fields)
				{
					if (entityFields.find(name) == entityFields.end())
						continue;

					out << YAML::BeginMap; // ScriptField
					out << YAML::Key << "Name" << YAML::Value << name;
					out << YAML::Key << "Type" << YAML::Value << Utils::ScriptFieldTypeToString(field.Type);

					out << YAML::Key << "Data" << YAML::Value;
					ScriptFieldInstance& scriptField = entityFields.at(name);

					switch (field.Type)
					{
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
					}
					out << YAML::EndMap; // ScriptFields
				}
				out << YAML::EndSeq;
			}

			out << YAML::EndMap; // ScriptComponent
		}

		out << YAML::EndMap; // Entity
	}
}
