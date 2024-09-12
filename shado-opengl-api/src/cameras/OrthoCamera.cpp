#include "OrthoCamera.h"


#include "../Events/input.h"
#include "../Events/KeyCodes.h"
#include <algorithm>

#include "Application.h"

/***************** CLASS OrthographicCamera ********************/
namespace Shado {
    OrthoCamera::OrthoCamera(float left, float right, float bottom, float top)
        : left(left), right(right), bottom(bottom), top(top) {
        nearClip = -2.0f;
        m_ProjectionMatrix = glm::ortho(left, right, bottom, top, getNearClip(), getFarClip());
        m_ViewMatrix = glm::mat4(1.0);
        m_viewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
    }

    void OrthoCamera::setProjection(float left, float right, float bottom, float top) {
        this->left = left;
        this->right = right;
        this->bottom = bottom;
        this->top = top;

        m_ProjectionMatrix = glm::ortho(left, right, bottom, top, getNearClip(), getFarClip());
        m_viewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
    }

    void OrthoCamera::reCalculateViewMatrix() {
        glm::mat4 transform = translate(glm::mat4(1.0f), m_Position) * rotate(
            glm::mat4(1.0f), glm::radians(m_Rotation.z), glm::vec3(0, 0, 1));

        m_ViewMatrix = inverse(transform);
        m_viewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
    }

    void OrthoCamera::reCalculateProjectionMatix() {
        m_ProjectionMatrix = glm::ortho(left, right, bottom, top, getNearClip(), getFarClip());
        m_viewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
    }

    /***************** CLASS OrthoCameraController ********************/

    OrthoCameraController::OrthoCameraController(float aspectRatio, bool rotation) {
        m_aspectRatio = aspectRatio;
        m_ZoomLevel = 1.0f;
        m_Rotation = rotation;

        m_Camera = new OrthoCamera(-m_aspectRatio * m_ZoomLevel, m_aspectRatio * m_ZoomLevel, -m_ZoomLevel,
                                   m_ZoomLevel);
    }

    void OrthoCameraController::onUpdate(TimeStep dt) {
        // Movement
        if (Input::isKeyPressed(KeyCode::A))
            m_CameraPosition.x -= m_CameraTranslationSpeed * dt;
        else if (Input::isKeyPressed(KeyCode::D))
            m_CameraPosition.x += m_CameraTranslationSpeed * dt;

        if (Input::isKeyPressed(KeyCode::W))
            m_CameraPosition.y += m_CameraTranslationSpeed * dt;
        else if (Input::isKeyPressed(KeyCode::S))
            m_CameraPosition.y -= m_CameraTranslationSpeed * dt;

        // Rotation
        if (m_Rotation) {
            if (Input::isKeyPressed(KeyCode::Q))
                m_CameraRotation.z += m_CameraRotationSpeed * dt;
            else if (Input::isKeyPressed(KeyCode::E))
                m_CameraRotation.z -= m_CameraRotationSpeed * dt;

            m_Camera->setRotation(m_CameraRotation);
        }

        m_Camera->setPosition(m_CameraPosition);
        m_CameraTranslationSpeed = m_ZoomLevel;
    }

    void OrthoCameraController::onEvent(Event& e) {
        EventDispatcher dispatcher(e);
        dispatcher.dispatch<MouseScrolledEvent>(SHADO_BIND_EVENT_FN(OrthoCameraController::onMouseScrolled));
        dispatcher.dispatch<WindowResizeEvent>(SHADO_BIND_EVENT_FN(OrthoCameraController::onWindowResized));
    }

    void OrthoCameraController::onResize(float width, float height) {
        m_aspectRatio = width / height;
        static_cast<OrthoCamera*>(m_Camera)->setProjection(-m_aspectRatio * m_ZoomLevel, m_aspectRatio * m_ZoomLevel,
                                                           -m_ZoomLevel, m_ZoomLevel);
    }

    bool OrthoCameraController::onMouseScrolled(MouseScrolledEvent& e) {
        m_ZoomLevel -= e.getYOffset() * 0.25f;
        m_ZoomLevel = std::max(m_ZoomLevel, 0.25f);

        static_cast<OrthoCamera*>(m_Camera)->setProjection(-m_aspectRatio * m_ZoomLevel, m_aspectRatio * m_ZoomLevel,
                                                           -m_ZoomLevel, m_ZoomLevel);
        return false;
    }

    bool OrthoCameraController::onWindowResized(WindowResizeEvent& e) {
        onResize(static_cast<float>(e.getWidth()), static_cast<float>(e.getHeight()));
        return false;
    }
}
