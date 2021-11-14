//
// Created by lukas on 13.11.21.
//

#ifndef ENGINEEDITOR_GAME_OBJECT_H
#define ENGINEEDITOR_GAME_OBJECT_H

#include "model.h"

#include <memory>

struct Transform2dComponent {
    glm::vec2 translation{};
    glm::vec2 scale{1.0f, 1.0f};
    float rotation;


    glm::mat2 mat2() {
        const float s = glm::sin(rotation);
        const float c = glm::cos(rotation);
        glm::mat2 rotMatrix{{c, s}, {-s, c}};

        glm::mat2 scaleMat{{scale.x, 0.0f}, {0.0f, scale.y}};
        return rotMatrix * scaleMat; }
};

namespace Engine {
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
        Transform2dComponent transform2d{};
    private:
        GameObject(id_t objId) : id{objId} {}
        id_t id;
    };
}


#endif //ENGINEEDITOR_GAME_OBJECT_H
