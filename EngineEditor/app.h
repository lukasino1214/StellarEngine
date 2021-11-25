//
// Created by lukas on 08.11.21.
//

#ifndef ENGINEEDITOR_APP_H
#define ENGINEEDITOR_APP_H

#include "../Engine/Engine.h"

#include "Panels/SceneHierarchyPanel.h"

// std
#include <memory>
#include <vector>

namespace Engine {
    class FirstApp {
    public:
        static constexpr int WIDTH = 1280;
        static constexpr int HEIGHT = 720;

        FirstApp();
        ~FirstApp();

        FirstApp(const FirstApp &) = delete;
        FirstApp &operator=(const FirstApp &) = delete;

        void run();

    private:

        glm::vec3 projectUonV(const glm::vec3& u, const glm::vec3& v) {
            glm::vec3 r = v * (glm::dot(u, v) / glm::dot(v, v));
            return r;
        }

        bool startPhysics = false;

        Window m_Window{WIDTH, HEIGHT, "Stellar Engine"};
        Device m_Device{m_Window};
        Renderer m_Renderer{m_Window, m_Device};

        std::unique_ptr<DescriptorPool> globalPool{};
        SceneHierarchyPanel HierarchyPanel;
        Ref<Scene> m_EditorScene;
        Ref<Scene> m_ActiveScene;
    };
}


#endif //ENGINEEDITOR_APP_H
