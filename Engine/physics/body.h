#pragma once

#include "shapes.h"

#include <glm/gtx/quaternion.hpp>

#include <memory>

namespace Engine {
    class Body {
    public:
        glm::vec3 position;
        glm::quat orientation;
        glm::vec3 linear_velocity;
        f32 inverse_mass; 
        std::unique_ptr<Shape> shape;

        glm::vec3 get_center_mass_world_space() const {
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
        }

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