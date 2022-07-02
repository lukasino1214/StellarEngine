//
// Created by lukas on 20.09.21.
//

#ifndef ENGINE_COMPONENTS_H
#define ENGINE_COMPONENTS_H

#include "../Core/UUID.h"
#include "../Core/Base.h"
#include "../Graphics/Model.h"
#include "../Scripting/NativeScript.h"
#include <entt/entity/entity.hpp>
#include <entt/entity/fwd.hpp>
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/fwd.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/gtx/quaternion.hpp>
#include <string>
//#include "../System/PhysicsSystem.h"


namespace Engine {

    struct IDComponent {
        UUID ID;

        IDComponent() = default;

        IDComponent(const UUID &uuid) : ID(uuid) {}

        IDComponent(const IDComponent &) = default;
    };

    struct TagComponent {
        std::string Tag;

        TagComponent() = default;

        TagComponent(const TagComponent &) = default;

        TagComponent(const std::string &tag) : Tag(tag) {}
    };

    struct TransformComponent {
        glm::vec3 Translation = {0.0f, 0.0f, 0.0f};
        glm::vec3 Rotation = {0.0f, 0.0f, 0.0f};
        glm::vec3 Scale = {1.0f, 1.0f, 1.0f};
        glm::mat4 ModelMatrix = glm::mat4(1.0);
        bool isDirty = true;

        TransformComponent() = default;

        TransformComponent(const TransformComponent &) = default;

        TransformComponent(const glm::vec3 &translation) : Translation(translation) {}

        glm::vec3 GetTranslation() { return Translation; }

        glm::vec3 GetRotation() { return Rotation; }

        glm::vec3 GetScale() { return Scale; }

        void SetTranslation(const glm::vec3 &translation) { Translation = translation; isDirty = true; }

        void SetRotation(const glm::vec3 &rotation) { Rotation = rotation; isDirty = true; }

        void SetScale(const glm::vec3 &scale) { Scale = scale; isDirty = true; }

        glm::mat4 mat4();

        glm::mat3 normalMatrix();
    };

    struct ModelComponent {
        std::shared_ptr<Model> model{};
        std::string path;

        ModelComponent() = default;

        //ModelComponent(const ModelComponent&) = default;
        ModelComponent(const std::shared_ptr<Model> &bruh) {
            model = bruh;
            path = bruh->getPath();
        }

        std::shared_ptr<Model> GetModel() { return model; }
    };

    struct ScriptComponent {
        std::shared_ptr<NativeScript> m_Script{};

        ScriptComponent() = default;

        ScriptComponent(const std::shared_ptr<NativeScript> &script) { m_Script = script; }
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
        bool isStatic = true;
    };

    /*struct PhysicsComponent {
        BodyID id;
    };*/

    struct CameraComponent {
        glm::mat4 projection;
        glm::mat4 view;
        float fov = 45.0;
        bool mainCamera = true;

        CameraComponent() = default;

        glm::mat4 View(glm::vec3 position, glm::vec3 rotation) {
            glm::mat4 view = glm::lookAt(position, (position + glm::vec3(0.0f, 0.0f, -1.0f)),
                                        glm::vec3(0.0f, 1.0f, 0.0f));
            view = glm::rotate(view, rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
            view = glm::rotate(view, rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
            view = glm::rotate(view, rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));

            return view;
        }

        glm::mat4 Projection(glm::vec2 size) {
            return glm::perspective(glm::radians(45.0f), size.x / size.y, 0.001f, 1000.0f);
        }
    };

    struct RelationshipComponent {
        entt::entity parent{entt::null};
        std::vector<entt::entity> children{};

        RelationshipComponent() = default;
    };
}

#endif //ENGINE_COMPONENTS_H
