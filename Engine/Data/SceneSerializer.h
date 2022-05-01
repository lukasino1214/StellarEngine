//
// Created by lukas on 17.09.21.
//

#ifndef ARISE_ENGINE_SCENESERIALIZER_H
#define ARISE_ENGINE_SCENESERIALIZER_H

#include <yaml-cpp/yaml.h>

#include "../Data/Scene.h"
#include "../Data/Entity.h"
#include "../Core/Base.h"

namespace Engine {
    class SceneSerializer {
    public:
        SceneSerializer(const Ref<Scene>& scene);

        void Serialize(const std::string& filepath);
        bool Deserialize(const std::string& filepath);
    private:
        Ref<Scene> m_Scene;
    };
}

#endif //ARISE_ENGINE_SCENESERIALIZER_H
