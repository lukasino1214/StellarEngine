//
// Created by lukas on 20.09.21.
//

#ifndef ENGINE_COMPONENTS_H
#define ENGINE_COMPONENTS_H

#include "../Core/UUID.h"
#include "../Core/Base.h"
#include "../Graphics/model.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <string>

namespace Engine {

    struct IDComponent {
        UUID ID;

        IDComponent() = default;
        IDComponent(const UUID& uuid) : ID(uuid) {}
        IDComponent(const IDComponent&) = default;
    };

    struct TagComponent {
        std::string Tag;

        TagComponent() = default;
        TagComponent(const TagComponent&) = default;
        TagComponent(const std::string& tag) : Tag(tag) {}
    };

    struct TransformComponent {
        glm::vec3 Translation = { 0.0f, 0.0f, 0.0f };
        glm::vec3 Rotation = { 0.0f, 0.0f, 0.0f };
        glm::vec3 Scale = { 1.0f, 1.0f, 1.0f };

        TransformComponent() = default;
        TransformComponent(const TransformComponent&) = default;
        TransformComponent(const glm::vec3& translation) : Translation(translation) {}

        glm::vec3 GetTranslation() { return Translation; }
        glm::vec3 GetRotation() { return Rotation; }
        glm::vec3 GetScale() { return Scale; }

        void SetTranslation(const glm::vec3& translation) { Translation = translation; }
        void SetRotation(const glm::vec3& rotation) { Rotation = rotation; }
        void SetScale(const glm::vec3& scale) { Scale = scale; }

        glm::mat4 mat4();
        glm::mat3 normalMatrix();
    };

    struct ModelComponent {
        std::shared_ptr<Model> model{};
        std::string path;

        ModelComponent() = default;
        //ModelComponent(const ModelComponent&) = default;
        ModelComponent(const std::shared_ptr<Model>& bruh) { model = bruh; path = bruh->getPath(); }

        std::shared_ptr<Model> GetModel() { return model; }
    };

    struct RigidBodyComponent {
        glm::vec3 velocity = { 0.0f, 0.0f, 0.0f };
        glm::vec3 acceleration = { 0.0f, 0.0f, 0.0f };
        float mass = 10.0f;
        float radius = 1.0f;
        bool isStatic = true;
    };
}

#endif //ENGINE_COMPONENTS_H
