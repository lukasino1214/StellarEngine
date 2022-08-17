#pragma once

#include "../core/UUID.h"
#include "../graphics/model.h"
#include "../scripting/native_script.h"
#include <entt/entity/entity.hpp>
#include <entt/entity/fwd.hpp>
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/fwd.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/gtx/quaternion.hpp>
#include <string>

#include "../physics/shapes.h"


namespace Engine {
    struct IDComponent {
        UUID ID;

        IDComponent() = default;
        IDComponent(const UUID &uuid) : ID(uuid) {}
    };

    struct TagComponent {
        std::string tag;

        TagComponent() = default;
        TagComponent(const std::string &_tag) : tag(_tag) {}
    };

    struct TransformComponent {
        glm::vec3 translation = {0.0f, 0.0f, 0.0f};
        glm::vec3 rotation = {0.0f, 0.0f, 0.0f};
        glm::vec3 scale = {1.0f, 1.0f, 1.0f};
        glm::mat4 model_matrix = glm::mat4(1.0);
        bool is_dirty = true;

        TransformComponent() = default;
        TransformComponent(const glm::vec3 &_translation) : translation(_translation) {}

        void set_translation(const glm::vec3 &_translation) { translation = _translation; is_dirty = true; }
        void set_rotation(const glm::vec3 &_rotation) { rotation = _rotation; is_dirty = true; }
        void set_scale(const glm::vec3 &_scale) { scale = _scale; is_dirty = true; }

        glm::mat4 calculate_matrix() const;
        glm::mat3 calculate_normal_matrix() const;
    };

    struct ModelComponent {
        std::shared_ptr<Model> model{};
        std::string path;
        bool transparent = false;

        ModelComponent() = default;
        ModelComponent(const std::shared_ptr<Model> &_model) { model = _model; path = _model->getPath(); }
    };

    struct ScriptComponent {
        std::shared_ptr<NativeScript> script{};

        ScriptComponent() = default;
        ScriptComponent(const std::shared_ptr<NativeScript> &_script) { script = _script; }
    };

    struct PointLightComponent {
        glm::vec3 color;
        float intensity;
    };

    struct RigidBodyComponent {
        glm::vec3 velocity = {0.0f, 0.0f, 0.0f};
        glm::vec3 acceleration = {0.0f, 0.0f, 0.0f};
        float mass = 10.0f;
        float radius = 1.0f;
        bool is_static = true;
    };


    struct CameraComponent {
        glm::mat4 projection;
        glm::mat4 view;
        float fov = 45.0;

        CameraComponent() = default;

        glm::mat4 calculate_view(glm::vec3 position, glm::vec3 rotation) {
            glm::mat4 _view = glm::lookAt(position, (position + glm::vec3(0.0f, 0.0f, -1.0f)), glm::vec3(0.0f, 1.0f, 0.0f));
            _view = glm::rotate(_view, rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
            _view = glm::rotate(_view, rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
            _view = glm::rotate(_view, rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));

            return _view;
        }

        glm::mat4 calculate_projection(glm::vec2 size) {
            return glm::perspective(glm::radians(45.0f), size.x / size.y, 0.001f, 1000.0f);
        }
    };

    struct RelationshipComponent {
        entt::entity parent{entt::null};
        std::vector<entt::entity> children{};

        RelationshipComponent() = default;
    };

    struct PhysicsComponent {
        glm::vec3 linear_velocity;
        f32 inverse_mass = 0.0f;
        f32 elasticity = 0.0f;
        std::unique_ptr<Shape> shape;

        PhysicsComponent() = default;

        /*glm::vec3 get_center_mass_world_space() const {
            const glm::vec3 center_mass = shape->get_center_mass();
            const glm::vec3 pos = position + glm::rotate(orientation, center_mass);
            return pos;
        }
        glm::vec3 get_center_mass_model_space() const {
            const glm::vec3 center_mass = shape->get_center_mass();
            return center_mass;
        }

        glm::vec3 world_space_to_body_space(const glm::vec3& pt) const {
            glm::vec3 tmp = pt - get_center_mass_world_space();
            glm::quat inverse_orientation = glm::inverse(orientation);
            glm::vec3 body_space = glm::rotate(inverse_orientation, tmp);
            return body_space;
        }
        glm::vec3 body_space_to_world_space(const glm::vec3& pt) const {
            glm::vec3 world_space = get_center_mass_world_space() + glm::rotate(orientation, pt);
            return world_space;
        }*/

        void apply_impulse_linear(const glm::vec3& impulse) {
            if(0.0f == inverse_mass) {
                return;
            }

            // p = mv
            // dp m dv = J
            // => dv = J / m
            linear_velocity += impulse * inverse_mass;
        }
    };
}