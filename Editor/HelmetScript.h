//
// Created by lukas on 24.04.22.
//

#ifndef ENGINE_HELMETSCRIPT_H
#define ENGINE_HELMETSCRIPT_H

#include "../Engine/Engine.h"

namespace Engine {
    class HelmetScript : public NativeScript {
    public:
        HelmetScript(entt::entity entity, std::shared_ptr<Scene> scene) : NativeScript(entity, scene) {}

        virtual ~HelmetScript() {}

        virtual void Start() override;

        virtual void Stop() override;

        virtual void OnUpdate(const float &deltaTime) override;

        virtual void OnEvent() override;

    };
}


#endif //ENGINE_HELMETSCRIPT_H
