#pragma once

#include "../data/scene.h"

namespace Engine {
    class NativeScript {
    public:

        NativeScript(entt::entity entity, std::shared_ptr<Scene> scene) : handle(entity), registry(scene->registry) {}
        virtual ~NativeScript() = default;

        virtual void start() = 0;
        virtual void stop() = 0;
        virtual void update(const float &deltaTime) = 0;
        virtual void on_event() = 0;

    protected:
        entt::entity handle;
        entt::registry &registry;

    };
}