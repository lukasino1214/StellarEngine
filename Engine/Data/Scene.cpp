//
// Created by lukas on 17.09.21.
//

#include "Scene.h"
#include "Components.h"
#include <glm/glm.hpp>
#include "Entity.h"
#include "glm/gtx/transform.hpp"

namespace Engine {

    Scene::Scene() {}

    Scene::~Scene() {}

    Entity Scene::CreateEntity(const std::string &name) {
        return CreateEntityWithUUID(UUID(), name);
    }

    Entity Scene::CreateEntityWithUUID(UUID uuid, const std::string &name) {
        Entity entity = {m_Registry.create(), this};
        entity.AddComponent<IDComponent>(uuid);
        entity.AddComponent<TransformComponent>();
        entity.AddComponent<RelationshipComponent>();
        auto &tag = entity.AddComponent<TagComponent>();
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

            if (entity.HasComponent<PointLightComponent>()) {
                auto light = entity.GetComponent<PointLightComponent>();
                auto position = entity.GetComponent<TransformComponent>().Translation;

                ubo.pointLights[ubo.numPointLights].color = glm::vec4(light.color, light.intensity);
                ubo.pointLights[ubo.numPointLights].position = glm::vec4(position, 1.0);

                ubo.numPointLights += 1;
            }
        });
    }

    void Scene::OnUpdate(const float &deltaTime) {
        m_Registry.each([&](auto entityID) {
            Entity entity = {entityID, this};
            if (!entity)
                return;

            if (entity.HasComponent<ScriptComponent>()) {
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

    void Scene::UpdateChildren(Entity entity) {
        for(auto& childID : entity.GetComponent<RelationshipComponent>().children) {
            Entity child = {childID, this};
            if (!entity)
                return;

            auto& e_tc = entity.GetComponent<TransformComponent>();
            glm::mat4 rotation = glm::toMat4(glm::quat(e_tc.Rotation));

            auto& tc = child.GetComponent<TransformComponent>();
            tc.ModelMatrix = tc.mat4() * rotation * glm::translate(glm::mat4(1.0f), e_tc.Translation) * glm::scale(glm::mat4(1.0f), e_tc.Scale);
            //std::cout << child.GetComponent<TagComponent>().Tag << std::endl;
            if(!child.GetComponent<RelationshipComponent>().children.empty()) {
                UpdateChildren(child);
            }
        }
    }

    void Scene::UpdateTransforms() {
        m_Registry.each([&](auto entityID) {
            Entity entity = {entityID, this};
            if (!entity)
                return;

            /*if(entity.GetComponent<TransformComponent>().isDirty) {
                auto& tc = entity.GetComponent<TransformComponent>();
                tc.ModelMatrix = tc.mat4();
                tc.isDirty = false;
                //UpdateChildren(entity);
            }*/
            if (entity.HasComponent<RelationshipComponent>()) {
                if(entity.GetComponent<TransformComponent>().isDirty) {
                    glm::mat4 parentMatrix = glm::mat4(1.0);
                    if (entity.GetComponent<RelationshipComponent>().parent != entt::null) {
                        Entity parent{entity.GetComponent<RelationshipComponent>().parent, this};
                        auto& tc = parent.GetComponent<TransformComponent>();
                        glm::mat4 rotation = glm::toMat4(glm::quat(tc.Rotation));

                        parentMatrix = rotation * glm::translate(glm::mat4(1.0f), tc.Translation) * glm::scale(glm::mat4(1.0f), tc.Scale);
                    }
                    auto& tc = entity.GetComponent<TransformComponent>();
                    tc.ModelMatrix = tc.mat4() * parentMatrix;
                    tc.isDirty = false;
                    UpdateChildren(entity);
                }
            }
        });
    }
}