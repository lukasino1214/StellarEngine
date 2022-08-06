#pragma once

#include <utility>

#include "../data/scene.h"
#include "../data/entity.h"

namespace Engine {
    class SceneSerializer {
    public:
        explicit SceneSerializer(std::shared_ptr<Scene> _scene) : scene{std::move(_scene)} {};

        void serialize(const std::string &filepath);
        bool deserialize(const std::shared_ptr<Device>& device, const std::string &filepath);

    private:
        std::shared_ptr<Scene> scene;
    };
}