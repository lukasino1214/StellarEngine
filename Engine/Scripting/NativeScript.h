//
// Created by lukas on 24.04.22.
//

#ifndef VENDOR_NATIVESCRIPT_H
#define VENDOR_NATIVESCRIPT_H

#include "../Data/Scene.h"

namespace Engine {
    class NativeScript {
    public:

        NativeScript(entt::entity entity, std::shared_ptr<Scene> scene) : m_Registry(scene->m_Registry),
                                                                          m_GameObject(entity) {}

        ~NativeScript() {}

        virtual void Start() = 0;

        virtual void Stop() = 0;

        virtual void OnUpdate(const float &deltaTime) = 0;

        virtual void OnEvent() = 0;

    protected:
        entt::entity m_GameObject;
        entt::registry &m_Registry;

    };
}


#endif //VENDOR_NATIVESCRIPT_H
