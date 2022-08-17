#include "intersect.h"
#include <glm/ext/quaternion_geometric.hpp>

namespace Engine {
    bool intersect(Entity& entity_A, Entity& entity_B, Contact& contact) {
        contact.entity_A = entity_A;
        contact.entity_B = entity_B;

        auto& ph_A = entity_A.get_component<PhysicsComponent>();
        auto& ph_B = entity_B.get_component<PhysicsComponent>();

        if (ph_A.shape->get_type() == Shape::SPHERE && ph_B.shape->get_type() == Shape::SPHERE) {
            glm::vec3 ab = entity_A.get_component<TransformComponent>().translation - entity_B.get_component<TransformComponent>().translation;

            contact.normal = glm::normalize(ab);

            Sphere* sphere_A = reinterpret_cast<Sphere*>(ph_A.shape.get());
            Sphere* sphere_B = reinterpret_cast<Sphere*>(ph_B.shape.get());

            contact.pos_world_space_A = entity_A.get_component<TransformComponent>().translation + contact.normal * sphere_A->radius;
            contact.pos_world_space_B = entity_B.get_component<TransformComponent>().translation - contact.normal * sphere_B->radius;

            f32 radius_ab = sphere_A->radius + sphere_B->radius;
            f32 length_squared = glm::length(ab) * glm::length(ab);
            if (length_squared <= (radius_ab * radius_ab)) {
                return true;
            }

            return false;
        }

        return false;
    }
}