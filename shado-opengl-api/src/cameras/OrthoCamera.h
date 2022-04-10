#pragma once
#include "glm/mat4x4.hpp"
#include "glm/vec3.hpp"
#include "../Events/Event.h"
#include "../Events/MouseEvent.h"
#include "../Events/ApplicationEvent.h"
#include "glm/ext/quaternion_float.hpp"
#include "../util/Util.h"
#include "Camera.h"

namespace Shado
{
	class OrthoCamera : public Camera {
	public:
		OrthoCamera(float left, float right, float bottom, float top);

		void setProjection(float left, float right, float bottom, float top);

		float getNearClip()	const { return -1.0f; }
		float getFarClip()	const { return 100.0f; }

	protected:
		virtual void reCalculateViewMatrix() override;
	};

	/***************** CLASS OrthographicCameraController ********************/

	class OrthoCameraController : public CameraController {
	public:
		OrthoCameraController(float aspectRatio, bool rotation = false);		// Zoom level: 1.0, X: aspect ratio * units of space in Y (default = 2 units in Y)

		virtual void onUpdate(TimeStep dt) override;
		virtual void onEvent(Event& e) override;

		void onResize(float width, float height);

	protected:
		virtual bool onMouseScrolled(MouseScrolledEvent& e) override;
		virtual bool onWindowResized(WindowResizeEvent& e) override;
	};
	
}