#include "OrthoCamera.h"


#include "../Events/input.h"
#include "../Events/KeyCodes.h"
#include <algorithm>

/***************** CLASS OrthographicCamera ********************/
namespace Shado {
	OrthoCamera::OrthoCamera(float left, float right, float bottom, float top)
	{
		m_ProjectionMatrix = glm::ortho(left, right, bottom, top, -1.0f, 100.0f);
		m_ViewMatrix = glm::mat4(1.0);
		m_viewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}

	void OrthoCamera::setProjection(float left, float right, float bottom, float top) {
		m_ProjectionMatrix = glm::ortho(left, right, bottom, top, -1.0f, 100.0f);
		m_viewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}

	void OrthoCamera::reCalculateViewMatrix() {

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), m_Position) * glm::rotate(glm::mat4(1.0f), glm::radians(m_Rotation.z), glm::vec3(0, 0, 1));

		m_ViewMatrix = glm::inverse(transform);
		m_viewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}

	/***************** CLASS OrthoCameraController ********************/

	OrthoCameraController::OrthoCameraController(float aspectRatio, bool rotation)
	{
		m_aspectRatio = aspectRatio;
		m_ZoomLevel = 1.0f;
		m_Rotation = rotation;

		m_Camera = new OrthoCamera(-m_aspectRatio * m_ZoomLevel, m_aspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel);
	}

	void OrthoCameraController::onUpdate(TimeStep dt)
	{
		// Movement
		if (Input::isKeyPressed(SHADO_KEY_A))
			m_CameraPosition.x -= m_CameraTranslationSpeed * dt;
		else if (Input::isKeyPressed(SHADO_KEY_D))
			m_CameraPosition.x += m_CameraTranslationSpeed * dt;

		if (Input::isKeyPressed(SHADO_KEY_W))
			m_CameraPosition.y += m_CameraTranslationSpeed * dt;
		else if (Input::isKeyPressed(SHADO_KEY_S))
			m_CameraPosition.y -= m_CameraTranslationSpeed * dt;

		// Rotation
		if (m_Rotation) {
			if (Input::isKeyPressed(SHADO_KEY_Q))
				m_CameraRotation.z += m_CameraRotationSpeed * dt;
			else if (Input::isKeyPressed(SHADO_KEY_E))
				m_CameraRotation.z -= m_CameraRotationSpeed * dt;

			m_Camera->setRotation(m_CameraRotation);
		}

		m_Camera->setPosition(m_CameraPosition);
		m_CameraTranslationSpeed = m_ZoomLevel;
	}

	void OrthoCameraController::onEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.dispatch<MouseScrolledEvent>(SHADO_BIND_EVENT_FN(OrthoCameraController::onMouseScrolled));
		dispatcher.dispatch<WindowResizeEvent>(SHADO_BIND_EVENT_FN(OrthoCameraController::onWindowResized));
	}

	void OrthoCameraController::onResize(float width, float height) {
		m_aspectRatio = width / height;
		((OrthoCamera*)m_Camera)->setProjection(-m_aspectRatio * m_ZoomLevel, m_aspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel);
	}

	bool OrthoCameraController::onMouseScrolled(MouseScrolledEvent& e)
	{
		m_ZoomLevel -= e.getYOffset() * 0.25f;
		m_ZoomLevel = max(m_ZoomLevel, 0.25f);

		((OrthoCamera*)m_Camera)->setProjection(-m_aspectRatio * m_ZoomLevel, m_aspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel);
		return false;
	}

	bool OrthoCameraController::onWindowResized(WindowResizeEvent& e)
	{
		onResize((float)e.getWidth(), (float)e.getHeight());
		return false;
	}
}
