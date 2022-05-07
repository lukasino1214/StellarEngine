//
// Created by lukas on 08.11.21.
//

#ifndef ENGINEEDITOR_APP_H
#define ENGINEEDITOR_APP_H

#include "../Engine/Engine.h"

#include "Panels/SceneHierarchyPanel.h"
#include "Panels/ContentBrowserPanel.h"
#include "Panels/DockSpacePanel.h"

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
        std::shared_ptr<Scene> m_EditorScene;
        std::shared_ptr<Scene> m_ActiveScene;

        std::shared_ptr<Window> m_Window;
        std::unique_ptr<Renderer> m_Renderer;
        std::shared_ptr<SceneHierarchyPanel> m_SceneHierarchyPanel;

        std::unique_ptr<OffScreen> m_OffScreenRenderingSystem;
        std::unique_ptr<RenderSystem> m_SimpleRenderSystem;
        std::unique_ptr<PointLightSystem> m_PointLightSystem;
        std::unique_ptr<PostProcessingSystem> m_PostProcessingSystem;
        std::unique_ptr<GridSystem> m_GridSystem;
        std::unique_ptr<ShadowSystem> m_ShadowSystem;

        std::shared_ptr<Camera> m_Camera;

        std::unique_ptr<ImGuiLayer> m_Imgui;

        std::unique_ptr<ContentBrowserPanel> m_ContentBrowserPanel;
        std::unique_ptr<DockSpacePanel> m_DockSpacePanel;

        bool startPhysics = false;
        VkDescriptorSet PostProcessingSet;

        glm::vec2 m_ViewportSize = {500, 200};
        BodyID floor_id;
        BodyID sphere_id;
    };
}


#endif //ENGINEEDITOR_APP_H
