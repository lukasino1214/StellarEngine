//
// Created by lukas on 06.11.21.
//

#ifndef ENGINE_INPUT_H
#define ENGINE_INPUT_H

#include "KeyCodes.h"

#include <GLFW/glfw3.h>

#include "../pgepch.h"

namespace Engine {
    class Input {
    public:
        Input(GLFWwindow *window) : m_Window(window) {}

        bool IsPressed(KeyCode key) {
            auto state = glfwGetKey(m_Window, static_cast<int32_t>(key));
            return state == GLFW_PRESS || state == GLFW_REPEAT;
        }

    private:
        GLFWwindow *m_Window;
    };

    class InputManager {
    public:
        static void Init(GLFWwindow *window);

        static std::shared_ptr<Input> m_Input;
    };
}

#endif //ENGINE_INPUT_H
