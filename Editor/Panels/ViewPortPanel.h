//
// Created by lukas on 24.04.22.
//

#ifndef ENGINEEDITOR_VIEWPORTPANEL_H
#define ENGINEEDITOR_VIEWPORTPANEL_H

#include "SceneHierarchyPanel.h"
#include "../../Engine/Graphics/Camera.h"

namespace Engine {
    class ViewPortPanel {
    public:
        ViewPortPanel(std::shared_ptr<SceneHierarchyPanel> sceneHierarchyPanel, std::shared_ptr<Camera> camera,
                      std::shared_ptr<Window> window, VkSampler sampler, VkImageView imageView);

        void OnImGuiRender(float frameTime);

        void UpdateImage(VkSampler sampler, VkImageView imageView);

        bool ShouldResize() { return m_ShouldResize; }

        glm::vec2 GetViewPortSize() { return m_ViewportSize; }

    private:
        std::shared_ptr<SceneHierarchyPanel> m_SceneHierarchyPanel;
        std::shared_ptr<Camera> m_Camera;
        std::shared_ptr<Window> m_Window;

        int m_GizmoType = 0;
        bool m_ShouldResize = false;
        glm::vec2 m_ViewportSize{1280.0, 720.0};
        VkDescriptorSet m_Image;
    };
}


#endif //ENGINEEDITOR_VIEWPORTPANEL_H
