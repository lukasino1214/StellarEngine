#pragma once

#include "../data/scene.h"
#include <memory>

namespace Engine {
    class PhysicsSystem {
    public:
        PhysicsSystem(std::shared_ptr<Scene> _scene) : scene{_scene} {}

        void update(const f32& detla_time);

    private:
        std::shared_ptr<Scene> scene;
    };
}