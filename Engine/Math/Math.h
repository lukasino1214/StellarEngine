//
// Created by lukas on 15.04.22.
//

#ifndef ENGINE_MATH_H
#define ENGINE_MATH_H


#include <glm/glm.hpp>

namespace Engine::Math {

    bool DecomposeTransform(const glm::mat4& transform, glm::vec3& translation, glm::vec3& rotation, glm::vec3& scale);

}


#endif //ENGINE_MATH_H
