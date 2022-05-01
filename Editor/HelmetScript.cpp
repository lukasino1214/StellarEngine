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
        auto& transform = m_Registry.get<TransformComponent>(m_GameObject);
        static constexpr float rotationAmount = glm::radians(90.0f);
        transform.Rotation.y += rotationAmount * deltaTime;
    }

    void HelmetScript::OnEvent() {

    }
}

