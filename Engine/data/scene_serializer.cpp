#include "scene_serializer.h"
#include "../pgepch.h"

namespace YAML {
    template<>
    struct convert<glm::vec3> {
        static Node encode(const glm::vec3 &rhs) {
            Node node;
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            node.push_back(rhs.z);
            node.SetStyle(EmitterStyle::Flow);
            return node;
        }

        static bool decode(const Node &node, glm::vec3 &rhs) {
            if (!node.IsSequence() || node.size() != 3)
                return false;

            rhs.x = node[0].as<float>();
            rhs.y = node[1].as<float>();
            rhs.z = node[2].as<float>();
            return true;
        }
    };

    template<>
    struct convert<glm::vec4> {
        static Node encode(const glm::vec4 &rhs) {
            Node node;
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            node.push_back(rhs.z);
            node.push_back(rhs.w);
            node.SetStyle(EmitterStyle::Flow);
            return node;
        }

        static bool decode(const Node &node, glm::vec4 &rhs) {
            if (!node.IsSequence() || node.size() != 4)
                return false;

            rhs.x = node[0].as<float>();
            rhs.y = node[1].as<float>();
            rhs.z = node[2].as<float>();
            rhs.w = node[3].as<float>();
            return true;
        }
    };

}

namespace Engine {
    YAML::Emitter &operator<<(YAML::Emitter &out, const glm::vec3 &v) {
        out << YAML::Flow;
        out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
        return out;
    }

    YAML::Emitter &operator<<(YAML::Emitter &out, const glm::vec4 &v) {
        out << YAML::Flow;
        out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
        return out;
    }

    static void serialize_entity(YAML::Emitter &out, Entity entity) {
        out << YAML::BeginMap; // Entity
        out << YAML::Key << "Entity" << YAML::Value << entity.GetUUID(); // TODO: Entity ID goes here

        if (entity.has_component<TagComponent>()) {
            out << YAML::Key << "TagComponent";
            out << YAML::BeginMap; // TagComponent

            auto &tag = entity.get_component<TagComponent>().tag;
            out << YAML::Key << "Tag" << YAML::Value << tag;

            out << YAML::EndMap; // TagComponent
        }

        if (entity.has_component<TransformComponent>()) {
            out << YAML::Key << "TransformComponent";
            out << YAML::BeginMap; // TransformComponent

            auto &tc = entity.get_component<TransformComponent>();
            out << YAML::Key << "Translation" << YAML::Value << tc.translation;
            out << YAML::Key << "Rotation" << YAML::Value << tc.rotation;
            out << YAML::Key << "Scale" << YAML::Value << tc.scale;

            out << YAML::EndMap; // TransformComponent
        }

        if (entity.has_component<RigidBodyComponent>()) {
            out << YAML::Key << "RigidBodyComponent";
            out << YAML::BeginMap; // RigidBodyComponent

            auto &rb = entity.get_component<RigidBodyComponent>();
            out << YAML::Key << "Velocity" << YAML::Value << rb.velocity;
            out << YAML::Key << "Acceleration" << YAML::Value << rb.velocity;
            out << YAML::Key << "Mass" << YAML::Value << rb.mass;
            out << YAML::Key << "Radius" << YAML::Value << rb.radius;
            out << YAML::Key << "isStatic" << YAML::Value << rb.is_static;

            out << YAML::EndMap; // RigidBodyComponent
        }

        if (entity.has_component<ModelComponent>()) {
            out << YAML::Key << "ModelComponent";
            out << YAML::BeginMap; // ModelComponent

            out << YAML::Key << "Path" << YAML::Value << entity.get_component<ModelComponent>().path;

            out << YAML::EndMap; // ModelComponent
        }

        if (entity.has_component<PointLightComponent>()) {
            out << YAML::Key << "PointLightComponent";
            out << YAML::BeginMap; // PointLightComponent

            auto &light = entity.get_component<PointLightComponent>();
            out << YAML::Key << "Color" << YAML::Value << light.color;
            out << YAML::Key << "Intensity" << YAML::Value << light.intensity;

            out << YAML::EndMap; // PointLightComponent
        }

        out << YAML::EndMap; // Entity
    }

    void SceneSerializer::serialize(const std::string &filepath) {
        YAML::Emitter out;
        out << YAML::BeginMap;
        out << YAML::Key << "Scene" << YAML::Value << "Untitled";
        out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;
        scene->registry.each([&](auto entityID) {
            Entity entity = {entityID, scene.get()};
            if (!entity)
                return;

            serialize_entity(out, entity);
        });

        out << YAML::EndSeq;
        out << YAML::EndMap;

        std::ofstream fout(filepath);
        fout << out.c_str();
    }

    bool SceneSerializer::deserialize(std::shared_ptr<Device> device, const std::string &filepath) {
        YAML::Node data;
        try {
            data = YAML::LoadFile(filepath);
        }
        catch (YAML::ParserException e) {
            return false;
        }

        if (!data["Scene"])
            return false;

        std::string sceneName = data["Scene"].as<std::string>();

        auto entities = data["Entities"];
        if (entities) {
            for (auto entity : entities) {
                uint64_t uuid = entity["Entity"].as<uint64_t>(); // TODO

                std::string name;
                auto tag_component = entity["TagComponent"];
                if (tag_component)
                    name = tag_component["Tag"].as<std::string>();

                Entity deserialized_entity = scene->create_entity_with_UUID(uuid, name);

                auto transform_component = entity["TransformComponent"];
                if (transform_component) {
                    auto &tc = deserialized_entity.get_component<TransformComponent>();
                    tc.translation = transform_component["Translation"].as<glm::vec3>();
                    tc.rotation = transform_component["Rotation"].as<glm::vec3>();
                    tc.scale = transform_component["Scale"].as<glm::vec3>();
                    tc.is_dirty = true;
                }

                auto rigidbody_component = entity["RigidBodyComponent"];
                if (rigidbody_component) {
                    deserialized_entity.add_component<RigidBodyComponent>();

                    deserialized_entity.get_component<RigidBodyComponent>().velocity = rigidbody_component["Velocity"].as<glm::vec3>();
                    deserialized_entity.get_component<RigidBodyComponent>().acceleration = rigidbody_component["Acceleration"].as<glm::vec3>();
                    deserialized_entity.get_component<RigidBodyComponent>().mass = rigidbody_component["Mass"].as<float>();
                    deserialized_entity.get_component<RigidBodyComponent>().radius = rigidbody_component["Radius"].as<float>();
                    deserialized_entity.get_component<RigidBodyComponent>().is_static = rigidbody_component["isStatic"].as<bool>();
                }

                auto model_component = entity["ModelComponent"];
                if (model_component) {
                    auto model = std::make_shared<Model>(device, model_component["Path"].as<std::string>());
                    deserialized_entity.add_component<ModelComponent>(model);
                }

                auto light_point_component = entity["PointLightComponent"];
                if (light_point_component) {
                    deserialized_entity.add_component<PointLightComponent>();

                    deserialized_entity.get_component<PointLightComponent>().color = light_point_component["Color"].as<glm::vec3>();
                    deserialized_entity.get_component<PointLightComponent>().intensity = light_point_component["Intensity"].as<float>();
                }
            }
        }

        return true;
    }
}