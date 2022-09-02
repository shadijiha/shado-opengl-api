#pragma once
#include "Camera.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../Application.h"

/*******************************************************************
 * *****************************************************************
 * ***** https://learnopengl.com/Getting-started/Camera ************
 * *****************************************************************
 * *****************************************************************
 */

namespace Shado {
    class OrbitCamera : public Camera {
        friend class OrbitCameraController;

    public:
        // constructor with vectors
        OrbitCamera(float aspectRatio);
        OrbitCamera(float aspectRatio, const glm::vec3& position,
            const glm::vec3& up,
            const glm::vec3& rotation);

        const glm::vec3& getRotation() const override;
        void setRotation(const glm::vec3& rot) override;
        void setProjection(const glm::mat4& projectionMatrix);
        void setAspectRatio(float aspectRatio);
        void setFOV(float fov) { Zoom = fov; reCalculateProjectionMatix(); }

        float getFOV()  const { return Zoom; }

    private:
        // processes input received from a mouse input system. Expects the offset value in both the x and y direction.
        void processMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);

        // calculates the front vector from the Camera's (updated) Euler Angles
        void updateCameraVectors();

        void reCalculateViewMatrix() override;

	protected:
		void reCalculateProjectionMatix() override;
	private:
        // camera Attributes
        glm::vec3 Front;
        glm::vec3 Up;
        glm::vec3 Right;
        glm::vec3 WorldUp;

        // camera options
        float MovementSpeed;
        float MouseSensitivity;
        float Zoom;
        float m_AspectRatio;
    };

    // ============================================

    class OrbitCameraController : public CameraController {
    public:
        OrbitCameraController(float aspectRatio, bool rotation = true);

        void onUpdate(TimeStep dt) override;
        void onEvent(Event& e) override;

        void setCameraMovSpeed(float ms) const;
        void setCameraMouseSens(float mouseSensitivity) const;
        void setCameraFOV(float FOV_degrees);

    protected:
        virtual bool onMouseScrolled(MouseScrolledEvent& e) override;
        virtual bool onWindowResized(WindowResizeEvent& e) override;

    protected:
        float m_FOV = 60.0f;
    };
}
