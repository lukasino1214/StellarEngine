#pragma once 

#include "../data/entity.h"

namespace Engine {
    struct Contact {
        glm::vec3 pos_world_space_A;
        glm::vec3 pos_world_space_B;
        glm::vec3 pos_local_space_A;
        glm::vec3 pos_local_space_B;

        glm::vec3 normal;	// In World Space coordinates
        f32 separation_distance;	// positive when non-penetrating, negative when penetrating
        f32 time_of_impact;

        Entity entity_A;
        Entity entity_B;
    };

    inline void resolve_contact(Contact& contact) {
        /*auto& ph_A = contact.entity_A.get_component<PhysicsComponent>();
        auto& ph_B = contact.entity_B.get_component<PhysicsComponent>();

        ph_A.linear_velocity = { 0.0f, 0.0f, 0.0f };
        ph_B.linear_velocity = { 0.0f, 0.0f, 0.0f };

        f32 tA = ph_A.inverse_mass / ( ph_A.inverse_mass + ph_B.inverse_mass);
        f32 tB = ph_B.inverse_mass / ( ph_A.inverse_mass + ph_B.inverse_mass);

        glm::vec3 ds = contact.pos_world_space_B - contact.pos_world_space_A;
        contact.entity_A.get_component<TransformComponent>().translation += ds * tA;
        contact.entity_B.get_component<TransformComponent>().translation -= ds * tB;*/

        auto& ph_A = contact.entity_A.get_component<PhysicsComponent>();
        auto& ph_B = contact.entity_B.get_component<PhysicsComponent>();

        f32 elasticity = ph_A.elasticity * ph_B.elasticity;

        glm::vec3 n = contact.normal;
        glm::vec3 vab = ph_A.linear_velocity - ph_B.linear_velocity;
        f32 impulse_J = -(1.0f + elasticity) * glm::dot(vab, n) / (ph_A.inverse_mass + ph_B.inverse_mass);
        glm::vec3 vec_impulse_J = n * impulse_J;

        ph_A.apply_impulse_linear(vec_impulse_J * 1.0f);
        ph_B.apply_impulse_linear(vec_impulse_J * -1.0f);

        f32 tA = ph_A.inverse_mass / ( ph_A.inverse_mass + ph_B.inverse_mass);
        f32 tB = ph_B.inverse_mass / ( ph_A.inverse_mass + ph_B.inverse_mass);

        glm::vec3 ds = contact.pos_world_space_B - contact.pos_world_space_A;
        contact.entity_A.get_component<TransformComponent>().translation += ds * tA;
        contact.entity_B.get_component<TransformComponent>().translation -= ds * tB;
    };
}