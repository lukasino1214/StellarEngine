//
// Created by lukas on 17.09.21.
//

#include "SceneSerializer.h"

#include <fstream>
#include <iostream>

namespace YAML {
    template<>
    struct convert<glm::vec3> {
        static Node encode(const glm::vec3& rhs) {
            Node node;
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            node.push_back(rhs.z);
            node.SetStyle(EmitterStyle::Flow);
            return node;
        }

        static bool decode(const Node& node, glm::vec3& rhs) {
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
        static Node encode(const glm::vec4& rhs) {
            Node node;
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            node.push_back(rhs.z);
            node.push_back(rhs.w);
            node.SetStyle(EmitterStyle::Flow);
            return node;
        }

        static bool decode(const Node& node, glm::vec4& rhs) {
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
    YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec3& v) {
        out << YAML::Flow;
        out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
        return out;
    }

    YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec4& v) {
        out << YAML::Flow;
        out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
        return out;
    }

    SceneSerializer::SceneSerializer(const Ref<Scene> &scene) : m_Scene(scene) {

    }

    static void SerializeEntity(YAML::Emitter& out, Entity entity) {
        out << YAML::BeginMap; // Entity
        out << YAML::Key << "Entity" << YAML::Value << entity.GetUUID(); // TODO: Entity ID goes here

        if (entity.HasComponent<TagComponent>()) {
            out << YAML::Key << "TagComponent";
            out << YAML::BeginMap; // TagComponent

            auto& tag = entity.GetComponent<TagComponent>().Tag;
            out << YAML::Key << "Tag" << YAML::Value << tag;

            out << YAML::EndMap; // TagComponent
        }

        if (entity.HasComponent<TransformComponent>()) {
            out << YAML::Key << "TransformComponent";
            out << YAML::BeginMap; // TransformComponent

            auto& tc = entity.GetComponent<TransformComponent>();
            out << YAML::Key << "Translation" << YAML::Value << tc.Translation;
            out << YAML::Key << "Rotation" << YAML::Value << tc.Rotation;
            out << YAML::Key << "Scale" << YAML::Value << tc.Scale;

            out << YAML::EndMap; // TransformComponent
        }

        if (entity.HasComponent<RigidBodyComponent>()) {
            out << YAML::Key << "RigidBodyComponent";
            out << YAML::BeginMap; // RigidBodyComponent

            auto& rb = entity.GetComponent<RigidBodyComponent>();
            out << YAML::Key << "Velocity" << YAML::Value << rb.velocity;
            out << YAML::Key << "Acceleration" << YAML::Value << rb.velocity;
            out << YAML::Key << "Mass" << YAML::Value << rb.mass;
            out << YAML::Key << "Radius" << YAML::Value << rb.radius;
            out << YAML::Key << "isStatic" << YAML::Value << rb.isStatic;

            out << YAML::EndMap; // RigidBodyComponent
        }

        if (entity.HasComponent<ModelComponent>()) {
            out << YAML::Key << "ModelComponent";
            out << YAML::BeginMap; // ModelComponent

            auto& m = entity.GetComponent<ModelComponent>();
            out << YAML::Key << "Path" << YAML::Value << m.path;

            out << YAML::EndMap; // ModelComponent
        }

        if (entity.HasComponent<PointLightComponent>()) {
            out << YAML::Key << "PointLightComponent";
            out << YAML::BeginMap; // PointLightComponent

            auto& light = entity.GetComponent<PointLightComponent>();
            out << YAML::Key << "Color" << YAML::Value << light.color;
            out << YAML::Key << "Intensity" << YAML::Value << light.intensity;

            out << YAML::EndMap; // PointLightComponent
        }

        out << YAML::EndMap; // Entity
    }

    void SceneSerializer::Serialize(const std::string& filepath) {
        YAML::Emitter out;
        out << YAML::BeginMap;
        out << YAML::Key << "Scene" << YAML::Value << "Untitled";
        out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;
        m_Scene->m_Registry.each([&](auto entityID) {
            Entity entity = { entityID, m_Scene.get() };
            if (!entity)
                return;

            SerializeEntity(out, entity);
        });

        out << YAML::EndSeq;
        out << YAML::EndMap;

        std::ofstream fout(filepath);
        fout << out.c_str();
    }

    bool SceneSerializer::Deserialize(const std::string &filepath) {
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
                auto tagComponent = entity["TagComponent"];
                if (tagComponent)
                    name = tagComponent["Tag"].as<std::string>();


                Entity deserializedEntity = m_Scene->CreateEntityWithUUID(uuid, name);

                auto transformComponent = entity["TransformComponent"];
                if (transformComponent) {
                    // Entities always have transforms
                    auto& tc = deserializedEntity.GetComponent<TransformComponent>();
                    tc.Translation = transformComponent["Translation"].as<glm::vec3>();
                    tc.Rotation = transformComponent["Rotation"].as<glm::vec3>();
                    tc.Scale = transformComponent["Scale"].as<glm::vec3>();
                }

                auto rigidBodyComponent = entity["RigidBodyComponent"];
                if (rigidBodyComponent) {
                    deserializedEntity.AddComponent<RigidBodyComponent>();

                    deserializedEntity.GetComponent<RigidBodyComponent>().velocity = rigidBodyComponent["Velocity"].as<glm::vec3>();
                    deserializedEntity.GetComponent<RigidBodyComponent>().acceleration = rigidBodyComponent["Acceleration"].as<glm::vec3>();
                    deserializedEntity.GetComponent<RigidBodyComponent>().mass = rigidBodyComponent["Mass"].as<float>();
                    deserializedEntity.GetComponent<RigidBodyComponent>().radius = rigidBodyComponent["Radius"].as<float>();
                    deserializedEntity.GetComponent<RigidBodyComponent>().isStatic = rigidBodyComponent["isStatic"].as<bool>();
                }

                auto modelComponent = entity["ModelComponent"];
                if (modelComponent) {
                    auto model = std::make_shared<Model>(modelComponent["Path"].as<std::string>());
                    deserializedEntity.AddComponent<ModelComponent>(model);
                }

                auto lightPointComponent = entity["PointLightComponent"];
                if (lightPointComponent) {
                    deserializedEntity.AddComponent<PointLightComponent>();

                    deserializedEntity.GetComponent<PointLightComponent>().color = lightPointComponent["Color"].as<glm::vec3>();
                    deserializedEntity.GetComponent<PointLightComponent>().intensity = lightPointComponent["Intensity"].as<float>();
                }

                /*auto CubeComponent = entity["CubeComponent"];
                if (CubeComponent) {
                    // Entities always have transforms
                    auto& bruh = deserializedEntity.AddComponent<CubeComponent>();
                }*/

                /*auto Color = entity["ColorComponent"];
                if (Color) {
                    // Entities always have transforms
                    auto& color = deserializedEntity.AddComponent<ColorComponent>();
                    //deserializedEntity.AddComponent<TransformComponent>();
                    color.color = Color["Color"].as<glm::vec3>();

                }

                auto Cube = entity["CubeComponent"];
                if(Cube) {
                    deserializedEntity.AddComponent<CubeComponent>();
                }*/
            }
        }

        return true;
    }
}