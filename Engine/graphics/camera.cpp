#include "camera.h"
#include "../core/input_manager.h"

namespace Engine {
    Camera::Camera(glm::vec3 position, glm::vec3 target) : m_Position(position) {
        m_View = glm::lookAt(m_Position, target, m_Up);
        m_Projection = glm::perspective(glm::radians(m_FOV), (float) m_Width / m_Height, nearPlane, farPlane);
    }

    void Camera::Move(GLFWwindow *window, float dt) {
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
            // Hides mouse cursor
            //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

            int width;
            int height;
            glfwGetWindowSize(window, &width, &height);

            // Prevents camera from jumping on the first click
            if (firstClick) {
                glfwSetCursorPos(window, (width / 2), (height / 2));
                firstClick = false;
            }

            // Handles key inputs
            if (InputManager::is_pressed(Key::W)) {
                m_Position += speed * m_Orientation;
            }
            if (InputManager::is_pressed(Key::A)) {
                m_Position += speed * -glm::normalize(glm::cross(m_Orientation, m_Up));
            }
            if (InputManager::is_pressed(Key::S)) {
                m_Position += speed * -m_Orientation;
            }
            if (InputManager::is_pressed(Key::D)) {
                m_Position += speed * glm::normalize(glm::cross(m_Orientation, m_Up));
            }
            if (InputManager::is_pressed(Key::Space)) {
                m_Position += speed * m_Up;
            }
            if (InputManager::is_pressed(Key::LeftControl)) {
                m_Position += speed * -m_Up;
            }
            if (InputManager::is_pressed(Key::LeftShift)) {
                speed = 0.5f;
            } else {
                speed = 0.1f;
            }

            // Stores the coordinates of the cursor
            double mouseX;
            double mouseY;
            // Fetches the coordinates of the cursor
            glfwGetCursorPos(window, &mouseX, &mouseY);

            // Normalizes and shifts the coordinates of the cursor such that they begin in the middle of the screen
            // and then "transforms" them into degrees
            float rotX = sensitivity * (float) (mouseY - (height / 2)) / height;
            float rotY = sensitivity * (float) (mouseX - (width / 2)) / width;

            // Calculates upcoming vertical change in the Orientation
            glm::vec3 newOrientation = glm::rotate(m_Orientation, glm::radians(-rotX),glm::normalize(glm::cross(m_Orientation, m_Up)));


            if(std::abs(glm::angle(newOrientation, m_Up) - glm::radians(90.0f)) <= glm::radians(90.0f)) {
                m_Orientation = newOrientation;
            }

            // Rotates the Orientation left and right
            m_Orientation = glm::rotate(m_Orientation, glm::radians(-rotY), m_Up);

            // Sets mouse cursor to the middle of the screen so that it doesn't end up roaming around
            glfwSetCursorPos(window, (width / 2), (height / 2));
        }

        m_View = glm::lookAt(m_Position, m_Position + m_Orientation, m_Up);
    }
}