﻿#include "SceneSerializer.h"
#include "Debug.h"
#include <yaml-cpp/yaml.h>
#include <fstream>

#include "Components.h"

namespace YAML {

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

}

namespace Shado {

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
		YAML::Node data;
		try
		{
			data = YAML::LoadFile(filepath);
		} catch (YAML::ParserException e)
		{
			return false;
		}

		if (!data["Scene"])
			return false;

		std::string sceneName = data["Scene"].as<std::string>();
		SHADO_CORE_TRACE("Deserializing scene '{0}'", sceneName);
		m_Scene->name = sceneName;
		
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

				Entity deserializedEntity = m_Scene->createEntity(name);

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
		out << YAML::BeginMap; // Entity
		out << YAML::Key << "Entity" << YAML::Value << "12837192831273"; // TODO: Entity ID goes here

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

		out << YAML::EndMap; // Entity
	}
}