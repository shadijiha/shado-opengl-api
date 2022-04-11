#pragma once
#include <glm/glm.hpp>

#include "Entity.h"
#include "cameras/Camera.h"
#include "cameras/OrbitCamera.h"
#include "cameras/OrthoCamera.h"

namespace Shado {

	struct TagComponent {
		std::string tag;

		TagComponent() = default;
		TagComponent(const TagComponent&) = default;
		TagComponent(const std::string& tag) : tag(tag) {}
	};

	struct TransformComponent {
		glm::mat4 transform = glm::mat4(1);

		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(const glm::mat4& transform) : transform(transform) {}

		operator glm::mat4& () { return transform; }
		operator const glm::mat4& () { return transform; }
	};
	
	struct SpriteRendererComponent {
		glm::vec4 color= {1, 1, 1, 1};

		SpriteRendererComponent() = default;
		SpriteRendererComponent(const SpriteRendererComponent&) = default;
		SpriteRendererComponent(const glm::vec4& color) : color(color) {}
	};

	struct CameraComponent {
		enum class Type {
			Orthographic = 0, Orbit = 1
		};

		Camera* camera;
		bool primary = true;
		bool fixedAspectRatio = false;
		Type type = Type::Orthographic;

		CameraComponent(Type type, uint32_t width, uint32_t height): type(type) {
			cachedWidth = width;
			cachedHeight = height;

			init(width, height);
		}

		CameraComponent(const CameraComponent&) = default;
		~CameraComponent() { delete camera; }

		void setViewportSize(uint32_t width, uint32_t height) {
			if (type == Type::Orthographic) {
				auto* cam = (OrthoCamera*)camera;
				float aspectRatio = (float)width / (float)height;
				float left = -size * aspectRatio * 0.5f;
				float right = size * aspectRatio * 0.5f;
				float bottom = -size * 0.5f;
				float top = size * 0.5f;
				cam->setProjection(left, right, bottom, top);
			} else {
				auto* cam = (OrbitCamera*)camera;
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
			delete camera;
			this->type = type;
			init(cachedWidth, cachedHeight);
		}

	private:
		float size = 5.0f;
		uint32_t cachedWidth, cachedHeight;

	private:
		void init(uint32_t width, uint32_t height) {
			if (type == Type::Orthographic) {
				camera = new OrthoCamera(-16.0f, 16.0f, -9.0f, 9.0f);
				setViewportSize(width, height);
			} else
				camera = new OrbitCamera((float)width / (float)height);
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

}
