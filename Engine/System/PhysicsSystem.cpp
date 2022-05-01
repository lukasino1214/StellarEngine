//
// Created by lukas on 01.05.22.
//

#include "PhysicsSystem.h"

namespace Engine {

    PhysicsSystem::PhysicsSystem() : temp_allocator(TempAllocatorImpl(10 * 1024 * 1024)), job_system(JobSystemThreadPool(cMaxPhysicsJobs, cMaxPhysicsBarriers, std::thread::hardware_concurrency() - 1)) {
        physics_system.Init(cMaxBodies, cNumBodyMutexes, cMaxBodyPairs, cMaxContactConstraints, broad_phase_layer_interface, MyBroadPhaseCanCollide, MyObjectCanCollide);

        physics_system.SetBodyActivationListener(&body_activation_listener);
        physics_system.SetContactListener(&contact_listener);
    }
}