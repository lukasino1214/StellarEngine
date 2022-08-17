#include "physics_system.h"

#include "../data/entity.h"

#include "contact.h"
#include "intersect.h"

namespace Engine {
    void PhysicsSystem::update(const f32 &detla_time) {
        scene->registry.each([&](auto entityID) {
            Entity entity = {entityID, scene.get()};
            if (!entity)
                return;

            if (entity.has_component<PhysicsComponent>()) {
                auto& ph = entity.get_component<PhysicsComponent>();
                f32 mass = 1.0f / ph.inverse_mass;
                glm::vec3 impulse_gravity = glm::vec3{ 0.0f, -10.0f, 0.0f } * mass * detla_time;
                ph.apply_impulse_linear(impulse_gravity);
            }
        });

        scene->registry.each([&](auto entityID_A) {
            Entity entity_A = {entityID_A, scene.get()};
            if (!entity_A)
                return;

            if(!entity_A.has_component<PhysicsComponent>()) {
                return;
            }

            scene->registry.each([&](auto entityID_B) {
                Entity entity_B = {entityID_B, scene.get()};
                if (!entity_B)
                    return;

                if(!entity_B.has_component<PhysicsComponent>()) {
                    return;
                }

                if(entity_A.get_UUID() == entity_B.get_UUID()) {
                    return;
                }

                auto& ph_A = entity_A.get_component<PhysicsComponent>();
                auto& ph_B = entity_B.get_component<PhysicsComponent>();

                if (0.0f == ph_A.inverse_mass && 0.0f == ph_B.inverse_mass) {
                    return;
                }
                Contact contact;

                if(intersect(entity_A, entity_B, contact)) {
                    resolve_contact(contact);
                }
            });
        });

        scene->registry.each([&](auto entityID) {
            Entity entity = {entityID, scene.get()};
            if (!entity)
                return;

            if (entity.has_component<PhysicsComponent>()) {
                auto& tn = entity.get_component<TransformComponent>();
                auto& ph = entity.get_component<PhysicsComponent>();
                tn.translation += ph.linear_velocity * detla_time;
                tn.is_dirty = true;
            }
        });
    }
}