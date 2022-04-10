#include "OrbitCamera.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include "../Debug.h"
#include "../Events/input.h"
#include "../Events/KeyCodes.h"

namespace Shado {
	OrbitCamera::OrbitCamera(float aspectRatio)
		: OrbitCamera(aspectRatio, { 0.0f, 0.0f, 0.0f },
			{ 0.0f, 1.0f, 0.0f },
			{ 0.0f, -90.0f, 0.0f })
	{
	}

	OrbitCamera::OrbitCamera(float aspectRatio, const glm::vec3& position, const glm::vec3& up, const glm::vec3& rotation)
		: Front({ 0.0f, 0.0f, -1.0f }), MovementSpeed(2.5f), MouseSensitivity(0.1f), Zoom(60.0f)
	{
		m_Position = position;
		WorldUp = up;
		m_Rotation = rotation;
		m_AspectRatio = aspectRatio;
		m_ProjectionMatrix = glm::perspective(glm::radians(Zoom), m_AspectRatio, 0.1f, 100.0f);
		updateCameraVectors();
		reCalculateViewMatrix();
	}

	const glm::vec3& OrbitCamera::getRotation() const {
		return m_Rotation;
	}

	void OrbitCamera::setRotation(const glm::vec3& rot) {
		m_Rotation = rot;

		// update Front, Right and Up Vectors using the updated Euler angles
		updateCameraVectors();
		reCalculateViewMatrix();
	}

	void OrbitCamera::setProjection(const glm::mat4& projectionMatrix) {
		m_ProjectionMatrix = projectionMatrix;
		reCalculateViewMatrix();
	}

	void OrbitCamera::setAspectRatio(float aspectRatio) {
		this->m_AspectRatio = aspectRatio;
		m_ProjectionMatrix = glm::perspective(glm::radians(Zoom), m_AspectRatio, 0.1f, 100.0f);
		reCalculateViewMatrix();
	}

	void OrbitCamera::processMouseMovement(float xoffset, float yoffset, bool constrainPitch) {
		xoffset *= MouseSensitivity;
		yoffset *= MouseSensitivity;

		m_Rotation.y += xoffset;
		m_Rotation.x += yoffset;

		// make sure that when pitch is out of bounds, screen doesn't get flipped
		if (constrainPitch)
		{
			if (m_Rotation.x > 89.0f)
				m_Rotation.x = 89.0f;
			if (m_Rotation.x < -89.0f)
				m_Rotation.x = -89.0f;
		}

		// update Front, Right and Up Vectors using the updated Euler angles
		updateCameraVectors();
	}

	void OrbitCamera::updateCameraVectors() {
		// calculate the new Front vector
		glm::vec3 front;
		front.x = cos(glm::radians(m_Rotation.y)) * cos(glm::radians(m_Rotation.x));
		front.y = sin(glm::radians(m_Rotation.x));
		front.z = sin(glm::radians(m_Rotation.y)) * cos(glm::radians(m_Rotation.x));
		Front = glm::normalize(front);

		// also re-calculate the Right and Up vector
		// normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.    	
		Right = glm::normalize(glm::cross(Front, WorldUp));
		Up = glm::normalize(glm::cross(Right, Front));
	}

	void OrbitCamera::reCalculateViewMatrix() {

		m_ViewMatrix = glm::lookAt(m_Position, m_Position + Front, Up) * glm::rotate(
			glm::mat4(1.0f), glm::radians(m_Rotation.z), glm::vec3(0, 0, 1));
		m_viewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}

	//=========================================================

	OrbitCameraController::OrbitCameraController(float aspectRatio, bool rotation)
	{
		m_aspectRatio = aspectRatio;
		m_Rotation = rotation;


		m_Camera = new OrbitCamera(aspectRatio);

		OrbitCamera& cam = *dynamic_cast<OrbitCamera*>(m_Camera);
		m_CameraRotation = { cam.m_Rotation.x, cam.m_Rotation.y, 0 };

		m_ZoomLevel = cam.Zoom;	// Don't use this
	}

	void OrbitCameraController::onUpdate(TimeStep dt) {

		OrbitCamera& cam = *dynamic_cast<OrbitCamera*>(m_Camera);

		// Movement
		float velocity = cam.MovementSpeed * dt;
		bool hasChangedPos = false;
		if (Input::isKeyPressed(KeyCode::W)) {
			m_CameraPosition += cam.Front * velocity;
			hasChangedPos = true;
		}
		if (Input::isKeyPressed(KeyCode::S)) {
			m_CameraPosition -= cam.Front * velocity;
			hasChangedPos = true;
		}
		if (Input::isKeyPressed(KeyCode::A)) {
			m_CameraPosition -= cam.Right * velocity;
			hasChangedPos = true;
		}
		if (Input::isKeyPressed(KeyCode::D)) {
			m_CameraPosition += cam.Right * velocity;
			hasChangedPos = true;
		}

		// Translation
		if (hasChangedPos) {
			cam.setPosition(m_CameraPosition);
		}

		// Rotation	
		if (m_Rotation) {

			bool hasChangedRot = false;

			if (Input::isKeyPressed(KeyCode::UpArrow)) {
				m_CameraRotation.x += m_CameraRotationSpeed * dt;
				hasChangedRot = true;
			}
			if (Input::isKeyPressed(KeyCode::DownArrow)) {
				m_CameraRotation.x -= m_CameraRotationSpeed * dt;
				hasChangedRot = true;
			}
			if (Input::isKeyPressed(KeyCode::RightArrow)) {
				m_CameraRotation.y += m_CameraRotationSpeed * dt;
				hasChangedRot = true;
			}
			if (Input::isKeyPressed(KeyCode::LeftArrow)) {
				m_CameraRotation.y -= m_CameraRotationSpeed * dt;
				hasChangedRot = true;
			}
			if (Input::isKeyPressed(KeyCode::Q)) {
				m_CameraRotation.z -= m_CameraRotationSpeed * dt;
				hasChangedRot = true;
			}
			if (Input::isKeyPressed(KeyCode::E)) {
				m_CameraRotation.z += m_CameraRotationSpeed * dt;
				hasChangedRot = true;
			}

			if (hasChangedRot) {
				cam.setRotation(m_CameraRotation);
			}
		}
	}

	void OrbitCameraController::onEvent(Event& e) {
		EventDispatcher dispatcher(e);
		dispatcher.dispatch<MouseScrolledEvent>(SHADO_BIND_EVENT_FN(OrbitCameraController::onMouseScrolled));
		dispatcher.dispatch<WindowResizeEvent>(SHADO_BIND_EVENT_FN(OrbitCameraController::onWindowResized));
	}

	void OrbitCameraController::setCameraMovSpeed(float ms) const {

		OrbitCamera& cam = *dynamic_cast<OrbitCamera*>(m_Camera);
		cam.MovementSpeed = ms;

	}

	void OrbitCameraController::setCameraMouseSens(float mouseSensitivity) const {
		OrbitCamera& cam = *dynamic_cast<OrbitCamera*>(m_Camera);
		cam.MouseSensitivity = mouseSensitivity;
	}

	void OrbitCameraController::setCameraFOV(float FOV_degrees) {
		OrbitCamera& cam = *dynamic_cast<OrbitCamera*>(m_Camera);
		m_FOV = FOV_degrees;
		cam.Zoom = FOV_degrees;
	}

	bool OrbitCameraController::onMouseScrolled(MouseScrolledEvent& e)
	{
		OrbitCamera& cam = *dynamic_cast<OrbitCamera*>(m_Camera);

		m_ZoomLevel -= e.getYOffset();
		if (m_ZoomLevel < 1.0f) {
			m_ZoomLevel = 1.0f;
		}
		if (m_ZoomLevel > m_FOV) {
			m_ZoomLevel = m_FOV;
		}

		cam.Zoom = m_ZoomLevel;
		cam.setProjection(glm::perspective(glm::radians(m_ZoomLevel), m_aspectRatio, 0.1f, 100.0f));

		return false;
	}

	bool OrbitCameraController::onWindowResized(WindowResizeEvent& e)
	{
		OrbitCamera& cam = *dynamic_cast<OrbitCamera*>(m_Camera);
		cam.setProjection(glm::perspective(glm::radians(cam.Zoom), (float)e.getWidth() / (float)e.getHeight(), 0.1f, 100.0f));

		return false;
	}
}
