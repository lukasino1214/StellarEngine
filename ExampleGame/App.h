#pragma once

#include "../Engine/Engine.h"

// std
#include <memory>
#include <string>
#include <vector>

namespace Engine {
    struct AppConfig  {
        std::string name;
    };

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
        std::shared_ptr<Device> m_Device;
        std::unique_ptr<Renderer> m_Renderer;

        std::unique_ptr<OffScreen> m_OffScreenRenderingSystem;
        std::unique_ptr<RenderSystem> m_SimpleRenderSystem;
        std::unique_ptr<PointLightSystem> m_PointLightSystem;
        std::unique_ptr<PostProcessingSystem> m_PostProcessingSystem;
        std::unique_ptr<ShadowSystem> m_ShadowSystem;

        std::shared_ptr<Camera> m_Camera;

        //std::unique_ptr<ImGuiLayer> m_Imgui;

        bool startPhysics = false;
        VkDescriptorSet PostProcessingSet;
        bool m_StartPhysics = false;
    };
}