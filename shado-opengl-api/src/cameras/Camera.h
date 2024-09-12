#pragma once
#include "glm/mat4x4.hpp"
#include "glm/vec3.hpp"
#include "../Events/Event.h"
#include "../Events/MouseEvent.h"
#include "../Events/ApplicationEvent.h"
#include "../util/TimeStep.h"
#include "util/Memory.h"

namespace Shado {
    class Camera : public RefCounted {
    public:
        Camera() {
        }

        virtual ~Camera() {
        }

        virtual const glm::vec3& getPosition() const { return m_Position; }
        virtual const glm::vec3& getRotation() const { return m_Rotation; }

        virtual void setPosition(const glm::vec3& position) {
            m_Position = position;
            reCalculateViewMatrix();
        }

        virtual void setRotation(const glm::vec3& rotation) {
            m_Rotation = rotation;
            reCalculateViewMatrix();
        }

        virtual void setNearClip(float f) {
            nearClip = f;
            reCalculateProjectionMatix();
        }

        virtual void setFarClip(float f) {
            farClip = f;
            reCalculateProjectionMatix();
        }

        virtual const glm::mat4& getProjectionMatrix() const { return m_ProjectionMatrix; }
        virtual const glm::mat4& getViewMatrix() const { return m_ViewMatrix; }
        virtual glm::mat4 getViewProjectionMatrix() const { return m_viewProjectionMatrix; }

        float getNearClip() const { return nearClip; }
        float getFarClip() const { return farClip; }

    protected:
        virtual void reCalculateViewMatrix() = 0;
        virtual void reCalculateProjectionMatix() = 0;

        glm::mat4 m_ProjectionMatrix;
        glm::mat4 m_ViewMatrix;
        glm::mat4 m_viewProjectionMatrix;

        glm::vec3 m_Position = {0.0f, 0.0f, 0.0f};
        glm::vec3 m_Rotation = {0.0, 0.0, 0.0};

        float nearClip = 1.0f, farClip = 100.0f;
    };

    // ========================= CONTROLLER ==============================

    class CameraController {
    public:
        CameraController() {
        } // Zoom level: 1.0, X: aspect ratio * units of space in Y (default = 2 units in Y)
        virtual ~CameraController() { sdelete(m_Camera); }

        virtual void onUpdate(TimeStep dt) = 0;
        virtual void onEvent(Event& e) = 0;

        Camera& getCamera() { return *m_Camera; }
        const Camera& getCamera() const { return *m_Camera; }

        // Setters
        void setRotationSpeed(float degrees_per_second) { m_CameraRotationSpeed = degrees_per_second; }
        void setTranslationSpeed(float camera_speed) { m_CameraTranslationSpeed = camera_speed; }

    protected:
        virtual bool onMouseScrolled(MouseScrolledEvent& e) = 0;
        virtual bool onWindowResized(WindowResizeEvent& e) = 0;

        float m_aspectRatio;
        float m_ZoomLevel;
        Camera* m_Camera;

        float m_Rotation = false;

        glm::vec3 m_CameraRotation = {0.0f, 0.0f, 0.0f};
        glm::vec3 m_CameraPosition = {0.0f, 0.0f, 0.0f};

        float m_CameraTranslationSpeed = 1.0f;
        float m_CameraRotationSpeed = 180.0f;
    };
}
