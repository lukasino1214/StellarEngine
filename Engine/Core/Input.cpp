//
// Created by lukas on 14.05.22.
//

#include "Input.h"

namespace Engine {

    std::shared_ptr<Input> InputManager::m_Input;


    void InputManager::Init(GLFWwindow *window) {
        m_Input = std::make_shared<Input>(window);
    }
}