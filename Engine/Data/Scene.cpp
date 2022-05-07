//
// Created by lukas on 17.09.21.
//

#include "Scene.h"

#include "Components.h"

#include <glm/glm.hpp>

#include <glm/glm.hpp>

#include "Entity.h"

namespace Engine {

    Scene::Scene() {}

    Scene::~Scene() {}

    Entity Scene::CreateEntity(const std::string& name) {
        return CreateEntityWithUUID(UUID(), name);
    }

    Entity Scene::CreateEntityWithUUID(UUID uuid, const std::string& name) {
        Entity entity = { m_Registry.create(), this };
        entity.AddComponent<IDComponent>(uuid);
        entity.AddComponent<TransformComponent>();
        auto& tag = entity.AddComponent<TagComponent>();
        tag.Tag = name.empty() ? "Entity" : name;
        return entity;
    }

    void Scene::DestroyEntity(Entity entity) {
        m_Registry.destroy(entity);
    }

    void Scene::UpdateLightsUbo(GlobalUbo &ubo) {
        m_Registry.each([&](auto entityID) {
            Entity entity = {entityID, this};
            if (!entity)
                return;

            if(entity.HasComponent<PointLightComponent>()) {
                auto light = entity.GetComponent<PointLightComponent>();
                auto position = entity.GetComponent<TransformComponent>().Translation;

                ubo.pointLights[ubo.numPointLights].color = glm::vec4(light.color, light.intensity);
                ubo.pointLights[ubo.numPointLights].position = glm::vec4(position, 1.0);

                ubo.numPointLights += 1;
            }
        });
    }

    void Scene::OnUpdate(const float& deltaTime) {
        m_Registry.each([&](auto entityID) {
            Entity entity = {entityID, this};
            if (!entity)
                return;

            if(entity.HasComponent<ScriptComponent>()) {
                auto script = entity.GetComponent<ScriptComponent>();
                script.m_Script->OnUpdate(deltaTime);
            }

            /*if(entity.template HasComponent<PhysicsComponent>()) {
                auto id = entity.GetComponent<PhysicsComponent>().id;
                BodyInterface& body_interface = Physics::m_PhysicsSystem->GetBodyInterface();
               if(body_interface.IsActive(id)) {
                    auto position = body_interface.GetCenterOfMassPosition(id);
                    glm::vec3& changed_position = entity.GetComponent<TransformComponent>().Translation;
                    changed_position.x = position.GetX();
                    changed_position.y = position.GetY();
                    changed_position.z = position.GetZ();
                }
            }*/
        });
    }
}