//
// Created by lukas on 21.11.21.
//

#ifndef ENGINE_PHYSICS_SYSTEM_H
#define ENGINE_PHYSICS_SYSTEM_H

#include "../Data/Scene.h"
#include "../Data/Entity.h"
#include "../pgepch.h"

namespace Engine {
    class PhysicsSystem {
    public:
        void Update(Ref<Scene> &scene, float deltaTime);
    };
}



#endif //ENGINE_PHYSICS_SYSTEM_H
