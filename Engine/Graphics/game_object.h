//
// Created by lukas on 13.11.21.
//

#ifndef ENGINEEDITOR_GAME_OBJECT_H
#define ENGINEEDITOR_GAME_OBJECT_H

#include "model.h"

#include <glm/gtc/matrix_transform.hpp>
#include <memory>

namespace Engine {
    struct TransformComponent {
        glm::vec3 translation{};
        glm::vec3 scale{1.0f, 1.0f, 1.0f};
        glm::vec3 rotation{};

        glm::mat4 mat4();
        glm::mat3 normalMatrix();
    };

    class GameObject {
        public:
        using id_t = uint32_t;

        static GameObject createGameObject() {
            static id_t currentID = 0;
            return GameObject{currentID++};
        }

        id_t getId() { return id; }

        GameObject(const GameObject & ) = delete;
        GameObject &operator=(const GameObject & ) = delete;
        GameObject(GameObject &&) = default;
        GameObject &operator=(GameObject &&) = default;

        std::shared_ptr<Model> model{};
        glm::vec3 color{};
        TransformComponent transform{};
    private:
        GameObject(id_t objId) : id{objId} {}
        id_t id;
    };
}


#endif //ENGINEEDITOR_GAME_OBJECT_H
