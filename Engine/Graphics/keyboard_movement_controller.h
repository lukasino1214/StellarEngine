//
// Created by lukas on 15.11.21.
//

#ifndef ENGINE_KEYBOARD_MOVEMENT_CONTROLLER_H
#define ENGINE_KEYBOARD_MOVEMENT_CONTROLLER_H

#include "game_object.h"
#include "../Core/Window.h"

namespace Engine {
    class KeyboardMovementController {
    public:
        struct KeyMappings {
            int moveLeft = GLFW_KEY_A;
            int moveRight = GLFW_KEY_D;
            int moveForward = GLFW_KEY_W;
            int moveBackward = GLFW_KEY_S;
            int moveUp = GLFW_KEY_E;
            int moveDown = GLFW_KEY_Q;
            int lookLeft = GLFW_KEY_LEFT;
            int lookRight = GLFW_KEY_RIGHT;
            int lookUp = GLFW_KEY_UP;
            int lookDown = GLFW_KEY_DOWN;
        };

        void moveInPlaneXZ(GLFWwindow* window, float dt, GameObject& gameObject);

        KeyMappings keys{};
        float moveSpeed{3.f};
        float lookSpeed{1.5f};
    };
}


#endif //ENGINE_KEYBOARD_MOVEMENT_CONTROLLER_H
