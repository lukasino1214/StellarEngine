//
// Created by lukas on 20.11.21.
//

#include "Components.h"

namespace Engine {
    glm::mat4 TransformComponentLegacy::mat4() {
        const float c3 = glm::cos(Rotation.z);
        const float s3 = glm::sin(Rotation.z);
        const float c2 = glm::cos(Rotation.x);
        const float s2 = glm::sin(Rotation.x);
        const float c1 = glm::cos(Rotation.y);
        const float s1 = glm::sin(Rotation.y);
        return glm::mat4{
                {
                        Scale.x * (c1 * c3 + s1 * s2 * s3),
                        Scale.x * (c2 * s3),
                        Scale.x * (c1 * s2 * s3 - c3 * s1),
                        0.0f,
                },
                {
                        Scale.y * (c3 * s1 * s2 - c1 * s3),
                        Scale.y * (c2 * c3),
                        Scale.y * (c1 * c3 * s2 + s1 * s3),
                        0.0f,
                },
                {
                        Scale.z * (c2 * s1),
                        Scale.z * (-s2),
                        Scale.z * (c1 * c2),
                        0.0f,
                }, // Ive add minus to Translation.y to make +y go up
                {Translation.x, -Translation.y, Translation.z, 1.0f}};
    }

    glm::mat3 TransformComponentLegacy::normalMatrix() {
        const float c3 = glm::cos(Rotation.z);
        const float s3 = glm::sin(Rotation.z);
        const float c2 = glm::cos(Rotation.x);
        const float s2 = glm::sin(Rotation.x);
        const float c1 = glm::cos(Rotation.y);
        const float s1 = glm::sin(Rotation.y);
        const glm::vec3 invScale = 1.0f / Scale;

        return glm::mat3{
                {
                        invScale.x * (c1 * c3 + s1 * s2 * s3),
                        invScale.x * (c2 * s3),
                        invScale.x * (c1 * s2 * s3 - c3 * s1),
                },
                {
                        invScale.y * (c3 * s1 * s2 - c1 * s3),
                        invScale.y * (c2 * c3),
                        invScale.y * (c1 * c3 * s2 + s1 * s3),
                },
                {
                        invScale.z * (c2 * s1),
                        invScale.z * (-s2),
                        invScale.z * (c1 * c2),
                },
        };
    }
}

