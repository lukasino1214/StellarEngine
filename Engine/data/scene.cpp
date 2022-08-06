#include "scene.h"
#include "components.h"
#include "entity.h"

#include <glm/glm.hpp>

namespace Engine {
    Scene::Scene() = default;
    Scene::~Scene() = default;

    Entity Scene::create_entity(const std::string &name) {
        return create_entity_with_UUID(UUID(), name);
    }

    Entity Scene::create_entity_with_UUID(UUID uuid, const std::string &name) {
        Entity entity = {registry.create(), this};
        entity.add_component<IDComponent>(uuid);
        entity.add_component<TransformComponent>();
        entity.add_component<RelationshipComponent>();
        auto &tag = entity.add_component<TagComponent>();
        tag.tag = name.empty() ? "Entity" : name;
        return entity;
    }

    void Scene::destroy_entity(Entity entity) {
        registry.destroy(entity);
    }

    void Scene::update_lights_ubo(GlobalUbo &ubo) {
        registry.each([&](auto entityID) {
            Entity entity = {entityID, this};
            if (!entity)
                return;

            if (entity.has_component<PointLightComponent>()) {
                auto light = entity.get_component<PointLightComponent>();
                auto position = entity.get_component<TransformComponent>().translation;

                ubo.point_lights[ubo.num_point_lights].color = glm::vec4(light.color, light.intensity);
                ubo.point_lights[ubo.num_point_lights].position = glm::vec4(position, 1.0);

                ubo.num_point_lights += 1;
            }
        });
    }

    void Scene::update(const float &deltaTime) {
        registry.each([&](auto entityID) {
            Entity entity = {entityID, this};
            if (!entity)
                return;

            if (entity.has_component<ScriptComponent>()) {
                auto script = entity.get_component<ScriptComponent>();
                script.script->update(deltaTime);
            }
        });
    }

    void Scene::update_children(Entity entity) {
        for(auto& childID : entity.get_component<RelationshipComponent>().children) {
            Entity child = {childID, this};
            if (!entity)
                return;

            auto& e_tc = entity.get_component<TransformComponent>();
            glm::mat4 rotation = glm::toMat4(glm::quat(e_tc.rotation));

            auto& tc = child.get_component<TransformComponent>();
            tc.model_matrix = tc.calculate_matrix() * rotation * glm::translate(glm::mat4(1.0f), e_tc.translation) * glm::scale(glm::mat4(1.0f), e_tc.scale);
            if(!child.get_component<RelationshipComponent>().children.empty()) {
                update_children(child);
            }
        }
    }

    void Scene::update_transforms() {
        registry.each([&](auto entityID) {
            Entity entity = {entityID, this};
            if (!entity)
                return;

            if (entity.has_component<RelationshipComponent>()) {
                if(entity.get_component<TransformComponent>().is_dirty) {
                    glm::mat4 parentMatrix = glm::mat4(1.0);
                    if (entity.get_component<RelationshipComponent>().parent != entt::null) {
                        Entity parent{entity.get_component<RelationshipComponent>().parent, this};
                        auto& tc = parent.get_component<TransformComponent>();
                        glm::mat4 rotation = glm::toMat4(glm::quat(tc.rotation));

                        parentMatrix = rotation * glm::translate(glm::mat4(1.0f), tc.translation) * glm::scale(glm::mat4(1.0f), tc.scale);
                    }
                    auto& tc = entity.get_component<TransformComponent>();
                    tc.model_matrix = tc.calculate_matrix() * parentMatrix;
                    tc.is_dirty = false;
                    update_children(entity);
                }
            }
        });
    }
}