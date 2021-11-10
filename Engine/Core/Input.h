//
// Created by lukas on 06.11.21.
//

#ifndef ENGINE_INPUT_H
#define ENGINE_INPUT_H

#include "KeyCodes.h"

#include <GLFW/glfw3.h>

namespace Engine {
    class Input {
    public:
        Input(GLFWwindow* window) : m_Window(window) {}

        bool IsPressed(KeyCode key) {
            auto state = glfwGetKey(m_Window, static_cast<int32_t>(key));
            return state == GLFW_PRESS || state == GLFW_REPEAT;
        }
    private:
        GLFWwindow* m_Window;
    };
}

#endif //ENGINE_INPUT_H
