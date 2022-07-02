//
// Created by lukas on 24.04.22.
//

#include "HelmetScript.h"
#include "../Engine/Data/Components.h"

namespace Engine {
    void HelmetScript::Start() {

    }

    void HelmetScript::Stop() {

    }

    void HelmetScript::OnUpdate(const float &deltaTime) {
        auto &transform = m_Registry.get<TransformComponent>(m_GameObject);
        if (InputManager::m_Input->IsPressed(Key::Left)) {
            transform.Rotation.y += 2.1f * deltaTime;
            transform.isDirty = true;
        }

        if (InputManager::m_Input->IsPressed(Key::Right)) {
            transform.Rotation.y -= 2.1f * deltaTime;
            transform.isDirty = true;
        }
        if (InputManager::m_Input->IsPressed(Key::Up)) {
            transform.Translation += 2.1f * deltaTime;
            transform.isDirty = true;
        }

        if (InputManager::m_Input->IsPressed(Key::Down)) {
            transform.Translation -= 2.1f * deltaTime;
            transform.isDirty = true;
        }
    }

    void HelmetScript::OnEvent() {

    }
}

