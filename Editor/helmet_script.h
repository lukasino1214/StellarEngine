#pragma once

#include "../Engine/engine.h"

namespace Engine {
    class HelmetScript : public NativeScript {
    public:
        HelmetScript(entt::entity entity, std::shared_ptr<Scene> scene) : NativeScript(entity, scene) {}
        virtual ~HelmetScript() {}

        virtual void start() override;
        virtual void stop() override;
        virtual void update(const float &deltaTime) override;
        virtual void on_event() override;

    };
}