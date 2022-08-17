#pragma once

#include "scene_hierarchy_panel.h"
#include "../../Engine/graphics/camera.h"

namespace Engine {
    class ViewportPanel {
    public:
        ViewportPanel(std::shared_ptr<SceneHierarchyPanel> _scene_hierarchy_panel, std::shared_ptr<Camera> _camera,
                      std::shared_ptr<Window> _window, VkSampler sampler, VkImageView imageView);

        void render(float frame_time);

        void update_image(VkSampler sampler, VkImageView image_view);

        bool resized() const { return should_resize; }
        glm::ivec2 get_viewport_size() { return viewport_size; }

    private:
        // TODO: This garbage
        std::shared_ptr<SceneHierarchyPanel> scene_hierarchy_panel;
        std::shared_ptr<Camera> camera;
        std::shared_ptr<Window> window;

        int gizmo_type = 0;
        bool should_resize = false;
        glm::ivec2 viewport_size = { 1280, 720 };
        VkDescriptorSet image;
    };
}