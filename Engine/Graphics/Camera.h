//
// Created by lukas on 23.11.21.
//

#ifndef VENDOR_CAMERA_H
#define VENDOR_CAMERA_H

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <limits>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <GLFW/glfw3.h>

namespace Engine {
    class Camera {
    public:
        Camera(glm::vec3 position, glm::vec3 target);
        // Stores the main vectors of the camera
        /*glm::vec3 Position;
        glm::vec3 Orientation = glm::vec3(0.0f, 0.0f, -1.0f);
        glm::vec3 Up = glm::vec3(0.0f, 1.0f, 0.0f);*/

        // Prevents the camera from jumping around when first clicking left click
        bool firstClick = true;

        // Stores the width and height of the window
        /*int width;
        int height;*/

        // Adjust the speed of the camera and it's sensitivity when looking around
        float speed = 0.1f;
        float sensitivity = 100.0f;

        // Camera constructor to set up initial values
        //Camera(glm::vec3 position);

        glm::mat4 GetView() { return m_View; }
        glm::mat4 GetProjection() { return m_Projection; }

        void SetProjection(int width, int height) {
            m_Width = width;
            m_Height = height;
            m_Projection = glm::perspective(glm::radians(m_FOV), (float)m_Width / m_Height, nearPlane, farPlane);
        }

        // Updates and exports the camera matrix to the Vertex Shader
        void Matrix(float FOVdeg, float nearPlane, float farPlane, const char* uniform);
        // Handles camera inputs
        void Move(GLFWwindow* window, float dt);

        void setOrthographicProjection(float left, float right, float top, float bottom, float near, float far);

        void setPerspectiveProjection(float fovy, float aspect, float near, float far);

        void setViewDirection(glm::vec3 position, glm::vec3 direction, glm::vec3 up = glm::vec3{0.f, 1.f, 0.f});
        void setViewTarget(glm::vec3 position, glm::vec3 target, glm::vec3 up = glm::vec3{0.f, 1.f, 0.f});
        void setViewYXZ();

        const glm::mat4& getProjection() const { return m_Projection; }
        const glm::mat4& getView() const { return m_View; }
        glm::vec3 getPosition() { return m_Position; }

    private:
        int m_Width = 1280;
        int m_Height = 720;
        float m_FOV = { 90.0f };
        float nearPlane = { 0.01f };
        float farPlane = { 1000.0f };

        glm::mat4 m_View = glm::mat4(1.0f);
        glm::mat4 m_Projection = glm::mat4(1.0f);

        glm::vec3 m_Position = { 0.0f, -1.0f, 0.0f };
        glm::vec3 m_Rotation = { 0.0f, 0.0f, 0.0f };
        glm::vec3 m_Orientation = { -1.0f, -1.0f, -1.0f };
        glm::vec3 m_Up = { 0.0f, 1.0f, 0.0f };

        float moveSpeed{3.f};
        float lookSpeed{1.5f};

    };

}
#endif //VENDOR_CAMERA_H
