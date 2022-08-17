#include "components.h"

namespace Engine {
    glm::mat4 TransformComponent::calculate_matrix() const {
        return glm::translate(glm::mat4(1.0f), translation) * glm::toMat4(glm::quat(rotation)) * glm::scale(glm::mat4(1.0f), scale);
    }

    glm::mat3 TransformComponent::calculate_normal_matrix() const {
        const float c3 = glm::cos(rotation.z);
        const float s3 = glm::sin(rotation.z);
        const float c2 = glm::cos(rotation.x);
        const float s2 = glm::sin(rotation.x);
        const float c1 = glm::cos(rotation.y);
        const float s1 = glm::sin(rotation.y);
        const glm::vec3 inv_scale = 1.0f / scale;

        return glm::mat3{
                {
                        inv_scale.x * (c1 * c3 + s1 * s2 * s3),
                        inv_scale.x * (c2 * s3),
                        inv_scale.x * (c1 * s2 * s3 - c3 * s1),
                },
                {
                        inv_scale.y * (c3 * s1 * s2 - c1 * s3),
                        inv_scale.y * (c2 * c3),
                        inv_scale.y * (c1 * c3 * s2 + s1 * s3),
                },
                {
                        inv_scale.z * (c2 * s1),
                        inv_scale.z * (-s2),
                        inv_scale.z * (c1 * c2),
                },
        };
    }
}

