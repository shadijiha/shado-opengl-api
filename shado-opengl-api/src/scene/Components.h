#pragma once
#include <glm/glm.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include "Entity.h"
#include "cameras/Camera.h"
#include "cameras/OrbitCamera.h"
#include "cameras/OrthoCamera.h"
#include "script/ScriptManager.h"

namespace Shado {
	struct IDComponent {
		UUID id;
	};

	struct TagComponent {
		std::string tag;

		TagComponent() = default;
		TagComponent(const TagComponent&) = default;
		TagComponent(const std::string& tag) : tag(tag) {}
	};

	struct TransformComponent {
		glm::vec3 position = { 0, 0, 0 };
		glm::vec3 rotation = {0, 0, 0};
		glm::vec3 scale = { 1.0f, 1.0f, 1.0f };

		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(const glm::vec3& position) : position(position) {}

		glm::mat4 getTransform() const {
			glm::mat4 _rotation = glm::toMat4(glm::quat(rotation));
			return glm::translate(glm::mat4(1.0f), position) * _rotation * glm::scale(glm::mat4(1.0f), scale);
		}
	};
	
	struct SpriteRendererComponent {
		glm::vec4 color= {1, 1, 1, 1};
		Ref<Texture2D> texture = nullptr;
		float tilingFactor = 1.0f;
		Ref<Shader> shader = nullptr;


		SpriteRendererComponent() = default;
		SpriteRendererComponent(const SpriteRendererComponent&) = default;
		SpriteRendererComponent(const glm::vec4& color) : color(color) {}
	};

	struct CircleRendererComponent {
		glm::vec4 color = { 1, 1, 1, 1 };
		Ref<Texture2D> texture = nullptr;
		float tilingFactor = 1.0f;
		Ref<Shader> shader = nullptr;
		float thickness = 1.0f;
		float fade = 0.005f;
	};

	struct CameraComponent {
		enum class Type {
			Orthographic = 0, Orbit = 1
		};

		Ref<Camera> camera;
		bool primary = true;
		bool fixedAspectRatio = false;
		Type type = Type::Orthographic;

		CameraComponent(Type type, uint32_t width, uint32_t height): type(type) {
			cachedWidth = width;
			cachedHeight = height;

			init(width, height);
		}
		CameraComponent(const CameraComponent&) = default;
		CameraComponent()
			: CameraComponent(Type::Orthographic,
					Application::get().getWindow().getWidth(), 
					Application::get().getWindow().getHeight()) {}
		~CameraComponent() { }

		void setViewportSize(uint32_t width, uint32_t height) {
			if (type == Type::Orthographic) {
				auto* cam = (OrthoCamera*)camera.get();
				float aspectRatio = (float)width / (float)height;
				float left = -size * aspectRatio * 0.5f;
				float right = size * aspectRatio * 0.5f;
				float bottom = -size * 0.5f;
				float top = size * 0.5f;
				cam->setProjection(left, right, bottom, top);
			} else {
				auto* cam = (OrbitCamera*)camera.get();
				cam->setAspectRatio((float)width / (float)height);
			}

			cachedWidth = width;
			cachedHeight = height;
		}

		float getSize() const {
			if (type == Type::Orbit)
				SHADO_CORE_WARN("Attempting to get the size of an orbit camera?");
			return size;
		}

		void setSize(float size) {
			if (type == Type::Orthographic) {
				this->size = size;
				this->setViewportSize(cachedWidth, cachedHeight);
			}
		}

		void setType(Type type) {
			camera.reset();
			this->type = type;
			init(cachedWidth, cachedHeight);
		}

	private:
		float size = 5.0f;
		uint32_t cachedWidth, cachedHeight;

	private:
		void init(uint32_t width, uint32_t height) {
			if (type == Type::Orthographic) {
				camera = CreateRef<OrthoCamera>(-16.0f, 16.0f, -9.0f, 9.0f);
				setViewportSize(width, height);
			} else
				camera = CreateRef<OrbitCamera>((float)width / (float)height);
		}
	};

	struct NativeScriptComponent {
		ScriptableEntity* script;

		ScriptableEntity* (*instantiateScript)();
		void (*destroyScript)(NativeScriptComponent*);

		template<typename T>
		void bind() {
			instantiateScript = []() {return (ScriptableEntity*)new T(); };
			destroyScript = [](NativeScriptComponent* nsc) {delete nsc->script; nsc->script = nullptr; };
		}
	};

	struct ScriptComponent {
		std::string className;
		ScriptClassDesc klass;
		ScriptClassInstance object;

		ScriptComponent(){}
		ScriptComponent(const ScriptComponent&) = default;
	};

	// Physics
	struct RigidBody2DComponent {
		enum class BodyType	{STATIC = 0, KINEMATIC, DYNAMIC	};

		BodyType type = BodyType::STATIC;
		bool fixedRotation = false;

		// b2 body
		void* runtimeBody = nullptr;

		RigidBody2DComponent() = default;
		RigidBody2DComponent(const RigidBody2DComponent&) = default;
	};

	struct BoxCollider2DComponent {
		glm::vec2 offset = { 0, 0 };
		glm::vec2 size = { 0.5f, 0.5f };

		// TODO : Should be moved to a physics material
		float density = 1.0f;
		float friction = 0.5f;
		float restitution = 0;
		float restitutionThreshold = 0.5f;

		// b2 Fixture
		void* runtimeFixture = nullptr;

		BoxCollider2DComponent() = default;
		BoxCollider2DComponent(const BoxCollider2DComponent&) = default;
	};

	struct CircleCollider2DComponent {
		glm::vec2 offset = { 0.0f, 0.0f };
		float radius = 0.5f;

		// TODO: move into physics material in the future maybe
		float density = 1.0f;
		float friction = 0.5f;
		float restitution = 0.0f;
		float restitutionThreshold = 0.5f;

		// Storage for runtime
		void* runtimeFixture = nullptr;

		CircleCollider2DComponent() = default;
		CircleCollider2DComponent(const CircleCollider2DComponent&) = default;
	};

}
