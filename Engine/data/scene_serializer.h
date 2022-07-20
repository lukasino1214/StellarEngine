#pragma once

#include <yaml-cpp/yaml.h>

#include "../data/scene.h"
#include "../data/entity.h"

namespace Engine {
    class SceneSerializer {
    public:
        SceneSerializer(const std::shared_ptr<Scene> &_scene) : scene{_scene} {};

        void serialize(const std::string &filepath);
        bool deserialize(std::shared_ptr<Device> device, const std::string &filepath);

    private:
        std::shared_ptr<Scene> scene;
    };
}