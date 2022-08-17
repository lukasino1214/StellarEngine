#pragma once

#include "key_codes.h"
#include <GLFW/glfw3.h>
#include "../pgepch.h"

namespace Engine {
    class InputManager {
    public:
        static void init(GLFWwindow *_window);
        static bool is_pressed(KeyCode key);
    private:
        static GLFWwindow *window;
    };
}