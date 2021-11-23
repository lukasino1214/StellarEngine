//
// Created by lukas on 21.11.21.
//

#include "physics_system.h"

const float G = 6.6742E-11;

namespace Engine {
    void PhysicsSystem::Update(Ref<Scene> &scene, float deltaTime) {
        auto DoSphereOverlap = [](Entity entity, Entity target) {
            auto transfrom1 = entity.GetComponent<TransformComponentLegacy>().GetTranslation();
            auto transfrom2 = target.GetComponent<TransformComponentLegacy>().GetTranslation();

            auto radius1 = entity.GetComponent<RigidBodyComponent>().getRadius();
            auto radius2 = target.GetComponent<RigidBodyComponent>().getRadius();

            return glm::abs((transfrom2.x - transfrom1.x) * (transfrom2.x - transfrom1.x) + (transfrom2.y - transfrom1.y) * (transfrom2.y - transfrom1.y) + (transfrom2.z - transfrom1.z) * (transfrom2.z - transfrom1.z)) <= (radius1 + radius2) * (radius1 + radius2);
        };


        scene->m_Registry.each([&](auto entityID) {
            Entity entity = { entityID, scene.get() };
            if (!entity)
                return;

            scene->m_Registry.each([&](auto targetID) {
                Entity target = {targetID, scene.get()};
                if (!target)
                    return;

                if(entity.GetComponent<IDComponent>().ID != target.GetComponent<IDComponent>().ID) {
                    auto entity_t = entity.GetComponent<TransformComponentLegacy>().GetTranslation();
                    auto target_t = target.GetComponent<TransformComponentLegacy>().GetTranslation();

                    auto entity_mass = entity.GetComponent<RigidBodyComponent>().mass;
                    auto target_mass = entity.GetComponent<RigidBodyComponent>().mass;

                    float distance = (target_t.x - entity_t.x) * (target_t.x - entity_t.x) + (target_t.y - entity_t.y) * (target_t.y - entity_t.y) + (target_t.z - entity_t.z) * (target_t.z - entity_t.z);
                    auto forceDirection =  (target_t - entity_t) / distance;
                    auto force = forceDirection * 1.2f * entity_mass * target_mass / distance;
                    auto acceleration = force / entity_mass;
                    entity.GetComponent<RigidBodyComponent>().acceleration += acceleration * 1.0f;

                }
            });
        });

        scene->m_Registry.each([&](auto entityID) {
            Entity entity = {entityID, scene.get()};
            if (!entity)
                return;

            auto acceleration = entity.GetComponent<RigidBodyComponent>().acceleration;
            auto velocity = entity.GetComponent<RigidBodyComponent>().velocity;

            // DRAG

            acceleration.x = acceleration.x * (1 - 0.99f * deltaTime);
            acceleration.y = acceleration.y * (1 - 0.99f * deltaTime);
            acceleration.z = acceleration.z * (1 - 0.99f * deltaTime);

            // UPDATE VELOCITY

            velocity.x = acceleration.x * deltaTime;
            velocity.y = acceleration.y * deltaTime;
            velocity.z = acceleration.z * deltaTime;

            auto position = entity.GetComponent<TransformComponentLegacy>().Translation;

            // UPDATE POSITION

            position.x += velocity.x;
            position.y += velocity.y;
            position.z += velocity.z;

            entity.GetComponent<RigidBodyComponent>().acceleration = acceleration;
            entity.GetComponent<RigidBodyComponent>().velocity = velocity;
            entity.GetComponent<TransformComponentLegacy>().SetTranslation(position);
        });

        scene->m_Registry.each([&](auto entityID) {
            Entity entity = { entityID, scene.get() };
            if (!entity)
                return;


            scene->m_Registry.each([&](auto targetID) {
                Entity target = {targetID, scene.get()};
                if (!target)
                    return;

                if(entity.GetComponent<IDComponent>().ID != target.GetComponent<IDComponent>().ID) {
                    //std::cout << entity.GetComponent<RigidBodyComponent>().hasStatic() << std::endl;
                    if(DoSphereOverlap(entity, target)) {

                        // STATIC COLISSION

                        auto transfrom1 = entity.GetComponent<TransformComponentLegacy>().GetTranslation();
                        auto transfrom2 = target.GetComponent<TransformComponentLegacy>().GetTranslation();

                        auto radius1 = entity.GetComponent<RigidBodyComponent>().getRadius();
                        auto radius2 = target.GetComponent<RigidBodyComponent>().getRadius();

                        float distance = glm::sqrt((transfrom2.x - transfrom1.x) * (transfrom2.x - transfrom1.x) + (transfrom2.y - transfrom1.y) * (transfrom2.y - transfrom1.y) + (transfrom2.z - transfrom1.z) * (transfrom2.z - transfrom1.z));
                        float overlap = 0.5 * (distance - radius1 - radius2);

                        auto entity_t = entity.GetComponent<TransformComponentLegacy>().GetTranslation();
                        auto target_t = target.GetComponent<TransformComponentLegacy>().GetTranslation();

                        entity_t.x -= overlap * (entity_t.x - target_t.x) / distance;
                        entity_t.y -= overlap * (entity_t.y - target_t.y) / distance;
                        entity_t.z -= overlap * (entity_t.z - target_t.z) / distance;

                        target_t.x += overlap * (entity_t.x - target_t.x) / distance;
                        target_t.y += overlap * (entity_t.y - target_t.y) / distance;
                        target_t.z += overlap * (entity_t.z - target_t.z) / distance;

                        entity.GetComponent<TransformComponentLegacy>().SetTranslation(entity_t);
                        target.GetComponent<TransformComponentLegacy>().SetTranslation(target_t);




                        // DYNAMIC COLISSION

                        auto entity_v = entity.GetComponent<RigidBodyComponent>().acceleration;
                        auto target_v = target.GetComponent<RigidBodyComponent>().acceleration;


                        /*glm::vec3 projectUonV(const glm::vec3& u, const glm::vec3& v) {
                            glm::vec3 r = v * (glm::dot(u, v) / glm::dot(v, v));
                            return r;
                        }*/

                        auto value1 = target_t - entity_t;
                        auto value2 = entity_t - target_t;
                        //glm::vec3 r = value1 * (glm::dot(target_v, value1) / glm::dot(value1, value1));


                        /*auto nv1 = entity.GetComponent<RigidBodyComponent>().acceleration;
                        nv1 += projectUonV(target_v, target_t - entity_t);
                        nv1 -= projectUonV(entity_v, entity_t - target_t);
                        auto nv2 = target.GetComponent<RigidBodyComponent>().acceleration;
                        nv2 += projectUonV(entity_v, target_t - entity_t);
                        nv2 -= projectUonV(target_v, entity_t - target_t);*/

                        auto nv1 = entity.GetComponent<RigidBodyComponent>().acceleration;
                        nv1 += value1 * (glm::dot(target_v, value1) / glm::dot(value1, value1));
                        nv1 -= value2 * (glm::dot(entity_v, value2) / glm::dot(value2, value2));
                        auto nv2 = target.GetComponent<RigidBodyComponent>().acceleration;
                        nv2 += value1 * (glm::dot(entity_v, value1) / glm::dot(value1, value1));
                        nv2 -= value2 * (glm::dot(target_v, value2) / glm::dot(value2, value2));

                        entity.GetComponent<RigidBodyComponent>().acceleration = nv1;
                        target.GetComponent<RigidBodyComponent>().acceleration = nv2;
                    }
                }
            });
        });
    }
}
