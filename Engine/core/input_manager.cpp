#include "input_manager.h"

namespace Engine {
    GLFWwindow *InputManager::window;

    void InputManager::init(GLFWwindow *_window) {
        window = _window;
    }

    bool InputManager::is_pressed(KeyCode key) {
        auto state = glfwGetKey(window, static_cast<int32_t>(key));
        return state == GLFW_PRESS || state == GLFW_REPEAT;
    }
}