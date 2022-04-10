#pragma once
#include <glm/glm.hpp>

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

	struct OrthoCameraComponent {
		OrthoCamera camera;
		bool primary = true;
		bool fixedAspectRatio = false;
		float size = 5.0f;

		OrthoCameraComponent() = default;
		OrthoCameraComponent(const OrthoCameraComponent&) = default;
		OrthoCameraComponent(float left, float right, float bottom, float top): camera(left, right, bottom, top) {}

		void setViewportSize(uint32_t width, uint32_t height) {
			float aspectRatio = (float)width / (float)height;
			float left = -size * aspectRatio * 0.5f;
			float right = size * aspectRatio * 0.5f;
			float bottom = -size * 0.5f;
			float top = size * 0.5f;

			camera.setProjection(left ,right, bottom, top);
		}
	};

	struct OrbitCameraComponent {
		OrbitCamera camera;
		bool primary = true;
		bool fixedAspectRatio = false;

		OrbitCameraComponent() = default;
		OrbitCameraComponent(const OrbitCameraComponent&) = default;
		OrbitCameraComponent(float aspectRatio) : camera(aspectRatio) {}

		void setViewportSize(uint32_t width, uint32_t height) {
			camera.setAspectRatio((float)width / (float)height);
		}
	};

}
