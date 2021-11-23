//
// Created by lukas on 23.11.21.
//

#include "Camera.h"

namespace Engine {
    /*Camera::Camera(glm::vec3 position) : m_Position(position) {
        m_View = glm::lookAt(m_Position, m_Position + m_Orientation, m_Up);
        m_Projection = glm::perspective(glm::radians(m_FOV), (float)m_Width / m_Height, nearPlane, farPlane);
    }*/

    void Camera::setOrthographicProjection(float left, float right, float top, float bottom, float near, float far) {
        projectionMatrix = glm::mat4{1.0f};
        projectionMatrix[0][0] = 2.f / (right - left);
        projectionMatrix[1][1] = 2.f / (bottom - top);
        projectionMatrix[2][2] = 1.f / (far - near);
        projectionMatrix[3][0] = -(right + left) / (right - left);
        projectionMatrix[3][1] = -(bottom + top) / (bottom - top);
        projectionMatrix[3][2] = -near / (far - near);
    }

    void Camera::setPerspectiveProjection(float fovy, float aspect, float near, float far) {
        assert(glm::abs(aspect - std::numeric_limits<float>::epsilon()) > 0.0f);
        const float tanHalfFovy = tan(fovy / 2.f);
        projectionMatrix = glm::mat4{0.0f};
        projectionMatrix[0][0] = 1.f / (aspect * tanHalfFovy);
        projectionMatrix[1][1] = 1.f / (tanHalfFovy);
        projectionMatrix[2][2] = far / (far - near);
        projectionMatrix[2][3] = 1.f;
        projectionMatrix[3][2] = -(far * near) / (far - near);
    }

    void Camera::setViewDirection(glm::vec3 position, glm::vec3 direction, glm::vec3 up) {
        const glm::vec3 w{glm::normalize(direction)};
        const glm::vec3 u{glm::normalize(glm::cross(w, up))};
        const glm::vec3 v{glm::cross(w, u)};

        viewMatrix = glm::mat4{1.f};
        viewMatrix[0][0] = u.x;
        viewMatrix[1][0] = u.y;
        viewMatrix[2][0] = u.z;
        viewMatrix[0][1] = v.x;
        viewMatrix[1][1] = v.y;
        viewMatrix[2][1] = v.z;
        viewMatrix[0][2] = w.x;
        viewMatrix[1][2] = w.y;
        viewMatrix[2][2] = w.z;
        viewMatrix[3][0] = -glm::dot(u, position);
        viewMatrix[3][1] = -glm::dot(v, position);
        viewMatrix[3][2] = -glm::dot(w, position);
    }

    void Camera::setViewTarget(glm::vec3 position, glm::vec3 target, glm::vec3 up) {
        setViewDirection(position, target - position, up);
    }

    void Camera::setViewYXZ() {
        const float c3 = glm::cos(m_Rotation.z);
        const float s3 = glm::sin(m_Rotation.z);
        const float c2 = glm::cos(m_Rotation.x);
        const float s2 = glm::sin(m_Rotation.x);
        const float c1 = glm::cos(m_Rotation.y);
        const float s1 = glm::sin(m_Rotation.y);
        const glm::vec3 u{(c1 * c3 + s1 * s2 * s3), (c2 * s3), (c1 * s2 * s3 - c3 * s1)};
        const glm::vec3 v{(c3 * s1 * s2 - c1 * s3), (c2 * c3), (c1 * c3 * s2 + s1 * s3)};
        const glm::vec3 w{(c2 * s1), (-s2), (c1 * c2)};
        viewMatrix = glm::mat4{1.f};
        viewMatrix[0][0] = u.x;
        viewMatrix[1][0] = u.y;
        viewMatrix[2][0] = u.z;
        viewMatrix[0][1] = v.x;
        viewMatrix[1][1] = v.y;
        viewMatrix[2][1] = v.z;
        viewMatrix[0][2] = w.x;
        viewMatrix[1][2] = w.y;
        viewMatrix[2][2] = w.z;
        viewMatrix[3][0] = -glm::dot(u, m_Position);
        viewMatrix[3][1] = -glm::dot(v, m_Position);
        viewMatrix[3][2] = -glm::dot(w, m_Position);
    }

    void Camera::Move(GLFWwindow* window, float dt) {
        glm::vec3 rotate{0};
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) rotate.y += 1.f;
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) rotate.y -= 1.f;
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) rotate.x += 1.f;
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) rotate.x -= 1.f;

        if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon()) {
            m_Rotation += lookSpeed * dt * glm::normalize(rotate);
        }

        // limit pitch values between about +/- 85ish degrees
        m_Rotation.x = glm::clamp(m_Rotation.x, -1.5f, 1.5f);
        m_Rotation.y = glm::mod(m_Rotation.y, glm::two_pi<float>());

        float yaw = m_Rotation.y;
        const glm::vec3 forwardDir{sin(yaw), 0.f, cos(yaw)};
        const glm::vec3 rightDir{forwardDir.z, 0.f, -forwardDir.x};
        const glm::vec3 upDir{0.f, -1.f, 0.f};

        glm::vec3 moveDir{0.f};
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) moveDir += forwardDir;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) moveDir -= forwardDir;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) moveDir += rightDir;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) moveDir -= rightDir;
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) moveDir += upDir;
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) moveDir -= upDir;

        if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon()) {
            m_Position += moveSpeed * dt * glm::normalize(moveDir);
        }

        setViewYXZ();
    }
}