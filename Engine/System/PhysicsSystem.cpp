//
// Created by lukas on 01.05.22.
//

#include "PhysicsSystem.h"

namespace Engine {
    std::shared_ptr<Engine::PhysicsSystem> Physics::m_PhysicsSystem;

    void Physics::Init() {
        m_PhysicsSystem = std::make_shared<PhysicsSystem>();
    }

    PhysicsSystem::PhysicsSystem() : temp_allocator(TempAllocatorImpl(10 * 1024 * 1024)), job_system(JobSystemThreadPool(cMaxPhysicsJobs, cMaxPhysicsBarriers, std::thread::hardware_concurrency() - 1)) {
        physics_system.Init(cMaxBodies, cNumBodyMutexes, cMaxBodyPairs, cMaxContactConstraints, broad_phase_layer_interface, MyBroadPhaseCanCollide, MyObjectCanCollide);

        physics_system.SetBodyActivationListener(&body_activation_listener);
        physics_system.SetContactListener(&contact_listener);
    }

    void PhysicsSystem::Update() {
        physics_system.Update(cDeltaTime, 1, 1, &temp_allocator, &job_system);
    }

    void PhysicsSystem::RemoveBody(BodyID id) {
        BodyInterface &body_interface = physics_system.GetBodyInterface();
        body_interface.RemoveBody(id);
        body_interface.DestroyBody(id);
    }
}