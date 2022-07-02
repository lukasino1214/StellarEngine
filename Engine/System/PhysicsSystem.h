//
// Created by lukas on 01.05.22.
//

/*#ifndef ENGINE_PHYSICSSYSTEM_H
#define ENGINE_PHYSICSSYSTEM_H

#include "../pgepch.h"

#include <thread>

#include <Jolt/Jolt.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>

using namespace JPH;

namespace Engine {

    namespace Layers {
        static constexpr uint8_t NON_MOVING = 0;
        static constexpr uint8_t MOVING = 1;
        static constexpr uint8_t NUM_LAYERS = 2;
    };

// Function that determines if two object layers can collide
    static bool MyObjectCanCollide(ObjectLayer inObject1, ObjectLayer inObject2) {
        switch (inObject1) {
            case Layers::NON_MOVING:
                return inObject2 == Layers::MOVING; // Non moving only collides with moving
            case Layers::MOVING:
                return true; // Moving collides with everything
            default:
                JPH_ASSERT(false);
                return false;
        }
    };

// Each broadphase layer results in a separate bounding volume tree in the broad phase. You at least want to have
// a layer for non-moving and moving objects to avoid having to update a tree full of static objects every frame.
// You can have a 1-on-1 mapping between object layers and broadphase layers (like in this case) but if you have
// many object layers you'll be creating many broad phase trees, which is not efficient. If you want to fine tune
// your broadphase layers define JPH_TRACK_BROADPHASE_STATS and look at the stats reported on the TTY.
    namespace BroadPhaseLayers {
        static constexpr BroadPhaseLayer NON_MOVING(0);
        static constexpr BroadPhaseLayer MOVING(1);
        static constexpr uint NUM_LAYERS(2);
    };

// BroadPhaseLayerInterface implementation
// This defines a mapping between object and broadphase layers.
    class BPLayerInterfaceImpl final : public BroadPhaseLayerInterface {
    public:
        BPLayerInterfaceImpl() {
            // Create a mapping table from object to broad phase layer
            mObjectToBroadPhase[Layers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
            mObjectToBroadPhase[Layers::MOVING] = BroadPhaseLayers::MOVING;
        }

        virtual uint GetNumBroadPhaseLayers() const override {
            return BroadPhaseLayers::NUM_LAYERS;
        }

        virtual BroadPhaseLayer GetBroadPhaseLayer(ObjectLayer inLayer) const override {
            JPH_ASSERT(inLayer < Layers::NUM_LAYERS);
            return mObjectToBroadPhase[inLayer];
        }

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)

        virtual const char *GetBroadPhaseLayerName(BroadPhaseLayer inLayer) const override {
            switch ((BroadPhaseLayer::Type) inLayer) {
                case (BroadPhaseLayer::Type) BroadPhaseLayers::NON_MOVING:
                    return "NON_MOVING";
                case (BroadPhaseLayer::Type) BroadPhaseLayers::MOVING:
                    return "MOVING";
                default:
                    JPH_ASSERT(false);
                    return "INVALID";
            }
        }

#endif // JPH_EXTERNAL_PROFILE || JPH_PROFILE_ENABLED

    private:
        BroadPhaseLayer mObjectToBroadPhase[Layers::NUM_LAYERS];
    };

// Function that determines if two broadphase layers can collide
    static bool MyBroadPhaseCanCollide(ObjectLayer inLayer1, BroadPhaseLayer inLayer2) {
        switch (inLayer1) {
            case Layers::NON_MOVING:
                return inLayer2 == BroadPhaseLayers::MOVING;
            case Layers::MOVING:
                return true;
            default:
                JPH_ASSERT(false);
                return false;
        }
    }

// An example contact listener
    class MyContactListener : public ContactListener {
    public:
        // See: ContactListener
        virtual ValidateResult OnContactValidate(const Body &inBody1, const Body &inBody2,
                                                 const CollideShapeResult &inCollisionResult) override {
            cout << "Contact validate callback" << endl;

            // Allows you to ignore a contact before it is created (using layers to not make objects collide is cheaper!)
            return ValidateResult::AcceptAllContactsForThisBodyPair;
        }

        virtual void OnContactAdded(const Body &inBody1, const Body &inBody2, const ContactManifold &inManifold,
                                    ContactSettings &ioSettings) override {
            cout << "A contact was added" << endl;
        }

        virtual void OnContactPersisted(const Body &inBody1, const Body &inBody2, const ContactManifold &inManifold,
                                        ContactSettings &ioSettings) override {
            cout << "A contact was persisted" << endl;
        }

        virtual void OnContactRemoved(const SubShapeIDPair &inSubShapePair) override {
            cout << "A contact was removed" << endl;
        }
    };

// An example activation listener
    class MyBodyActivationListener : public BodyActivationListener {
    public:
        virtual void OnBodyActivated(const BodyID &inBodyID, uint64 inBodyUserData) override {
            cout << "A body got activated" << endl;
        }

        virtual void OnBodyDeactivated(const BodyID &inBodyID, uint64 inBodyUserData) override {
            cout << "A body went to sleep" << endl;
        }
    };


    class PhysicsSystem {
    public:
        PhysicsSystem();

        void Update();

        BodyInterface &GetBodyInterface() { return physics_system.GetBodyInterface(); }

        void RemoveBody(BodyID id);

    private:
        TempAllocatorImpl temp_allocator;
        JobSystemThreadPool job_system;

        const uint cMaxBodies = 1024;
        const uint cNumBodyMutexes = 0;
        const uint cMaxBodyPairs = 1024;
        const uint cMaxContactConstraints = 1024;

        BPLayerInterfaceImpl broad_phase_layer_interface;
        JPH::PhysicsSystem physics_system;

        MyBodyActivationListener body_activation_listener;
        MyContactListener contact_listener;

        const float cDeltaTime = 1.0f / 60.0f;
    };

    class Physics {
    public:
        static std::shared_ptr<Engine::PhysicsSystem> m_PhysicsSystem;

        static void Init();
    };
}


#endif //ENGINE_PHYSICSSYSTEM_H*/
