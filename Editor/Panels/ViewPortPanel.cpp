//
// Created by lukas on 24.04.22.
//

#include "ViewPortPanel.h"

#include <imgui.h>
#include <backends/imgui_impl_vulkan.h>
#include <ImGuizmo.h>
#include "../../Engine/Math/Math.h"
#include "../../Engine/Core/Input.h"

namespace Engine {

    ViewPortPanel::ViewPortPanel(std::shared_ptr<SceneHierarchyPanel> sceneHierarchyPanel,
                                 std::shared_ptr<Camera> camera, std::shared_ptr<Window> window, VkSampler sampler,
                                 VkImageView imageView) : m_SceneHierarchyPanel{sceneHierarchyPanel}, m_Camera{camera},
                                                          m_Window{window} {
        m_Image = ImGui_ImplVulkan_AddTexture(sampler, imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }

    void ViewPortPanel::OnImGuiRender(float frameTime) {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("Viewport");
        ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
        if (m_ViewportSize != *((glm::vec2 *) &viewportPanelSize)) {
            m_ShouldResize = true;
            m_Camera->SetProjection(viewportPanelSize.x, viewportPanelSize.y);
            m_ViewportSize = {viewportPanelSize.x, viewportPanelSize.y};
        }

        ImGui::Image(m_Image, viewportPanelSize);

        if (InputManager::m_Input->IsPressed(Key::U)) {
            if (!ImGuizmo::IsUsing())
                m_GizmoType = -1;
        }

        if (InputManager::m_Input->IsPressed(Key::I)) {
            if (!ImGuizmo::IsUsing())
                m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
        }

        if (InputManager::m_Input->IsPressed(Key::O)) {
            if (!ImGuizmo::IsUsing())
                m_GizmoType = ImGuizmo::OPERATION::ROTATE;
        }

        if (InputManager::m_Input->IsPressed(Key::P)) {
            if (!ImGuizmo::IsUsing())
                m_GizmoType = ImGuizmo::OPERATION::SCALE;
        }

        if (ImGui::IsWindowHovered())
            m_Camera->Move(m_Window->getGLFWwindow(), frameTime);

        Entity selectedEntity = m_SceneHierarchyPanel->GetSelectedEntity();
        if (selectedEntity && m_GizmoType != -1) {
            ImGuizmo::SetOrthographic(false);
            ImGuizmo::SetDrawlist();

            auto &tc = selectedEntity.GetComponent<TransformComponent>();

            float windowWidth = (float) ImGui::GetWindowWidth();
            float windowHeight = (float) ImGui::GetWindowHeight();
            ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, windowWidth, windowHeight);
            glm::mat4 cameraView = glm::inverse(m_Camera->getView());
            glm::mat4 cameraProj = m_Camera->getProjection();

            glm::mat4 mod_mat = tc.ModelMatrix;
            glm::mat4 delta = glm::mat4{1.0};
            ImGuizmo::Manipulate(glm::value_ptr(m_Camera->getView()), glm::value_ptr(cameraProj),
                                 (ImGuizmo::OPERATION) m_GizmoType, ImGuizmo::LOCAL, glm::value_ptr(mod_mat), glm::value_ptr(delta), nullptr, nullptr, nullptr);

            if (ImGuizmo::IsUsing()) {
                glm::vec3 translation, rotation, scale;
                Math::DecomposeTransform(mod_mat, translation, rotation, scale);

                tc.Translation += translation - tc.Translation;
                tc.Rotation += rotation - tc.Rotation;
                tc.Scale += scale - tc.Scale;
                tc.isDirty = true;
                //tc.ModelMatrix = tc.mat4();
            }
        }
        ImGui::End();
        ImGui::PopStyleVar();
    }

    void ViewPortPanel::UpdateImage(VkSampler sampler, VkImageView imageView) {
        m_Image = ImGui_ImplVulkan_AddTexture(sampler, imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        m_ShouldResize = false;
    }
}