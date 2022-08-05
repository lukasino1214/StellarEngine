#pragma once

#include "../Engine/engine.h"

#include "panels/scene_hierarchy_panel.h"
#include "panels/content_browser_panel.h"
#include "panels/dock_space_panel.h"
#include "panels/viewport_panel.h"
#include "panels/preferences_panel.h"

namespace Engine {
    struct AppConfig {
        std::string name;
    };

    class App {
    public:
        static constexpr int WIDTH = 1280;
        static constexpr int HEIGHT = 720;

        App();
        ~App();

        App(const App &) = delete;
        App &operator=(const App &) = delete;

        void run();

    private:
        std::shared_ptr<Scene> editor_scene;

        std::shared_ptr<Window> window;
        std::shared_ptr<Device> device;
        std::unique_ptr<Renderer> renderer;

        //std::unique_ptr<ShadowSystem> shadow_system;
        std::unique_ptr<OffScreenSystem> offscreen_system;
        std::unique_ptr<RenderSystem> rendering_system;
        std::unique_ptr<PointLightSystem> point_light_system;
        std::unique_ptr<PostProcessingSystem> postprocessing_system;
        std::unique_ptr<GridSystem> grid_system;
        std::unique_ptr<DeferredRenderingSystem> deferred_rendering_system;
        std::unique_ptr<PBRSystem> pbr_system;
        std::unique_ptr<BloomRenderingSystem> bloom_system;
        std::unique_ptr<PreferencesPanel> preferences_panel;

        std::shared_ptr<Camera> camera;

        std::unique_ptr<ImGuiLayer> imgui_layer;

        std::unique_ptr<ContentBrowserPanel> content_browser_panel;
        std::unique_ptr<DockSpacePanel> dock_space_panel;
        std::shared_ptr<SceneHierarchyPanel> scene_hierarchy_panel;
        std::shared_ptr<ViewportPanel> viewport_panel;

        //VkDescriptorSet vk_post_processing_descriptor_set;
    };
}