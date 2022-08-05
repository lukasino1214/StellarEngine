#include "viewport_panel.h"

#include <imgui.h>
#include <backends/imgui_impl_vulkan.h>
#include <ImGuizmo.h>

#include <utility>
#include "../../Engine/math/math.h"
#include "../../Engine/core/input_manager.h"

namespace Engine {
    ViewportPanel::ViewportPanel(std::shared_ptr<SceneHierarchyPanel> _scene_hierarchy_panel, std::shared_ptr<Camera> _camera, std::shared_ptr<Window> _window, VkSampler sampler, VkImageView imageView) : scene_hierarchy_panel{std::move(_scene_hierarchy_panel)}, camera{std::move(_camera)}, window{std::move(_window)} {
        image = ImGui_ImplVulkan_AddTexture(sampler, imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }

    void ViewportPanel::render(float frameTime) {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("Viewport");
        ImVec2 viewport_panel_size = ImGui::GetContentRegionAvail();
        if (viewport_size != *(reinterpret_cast<glm::ivec2*>(&viewport_panel_size))) {
        //if (viewport_size != *((glm::vec2 *) &viewport_panel_size)) {
            should_resize = true;
            viewport_size = {viewport_panel_size.x, viewport_panel_size.y};
            camera->SetProjection(viewport_size.x, viewport_size.y);
        }

        ImGui::Image(image, viewport_panel_size);

        if (InputManager::is_pressed(Key::U)) {
            if (!ImGuizmo::IsUsing())
                gizmo_type = -1;
        }

        if (InputManager::is_pressed(Key::I)) {
            if (!ImGuizmo::IsUsing())
                gizmo_type = ImGuizmo::OPERATION::TRANSLATE;
        }

        if (InputManager::is_pressed(Key::O)) {
            if (!ImGuizmo::IsUsing())
                gizmo_type = ImGuizmo::OPERATION::ROTATE;
        }

        if (InputManager::is_pressed(Key::P)) {
            if (!ImGuizmo::IsUsing())
                gizmo_type = ImGuizmo::OPERATION::SCALE;
        }

        if(ImGui::IsWindowHovered()) {
            camera->Move(window->get_GLFWwindow(), frameTime);
        }

        Entity selected_entity = scene_hierarchy_panel->get_selected_entity();
        if (selected_entity && gizmo_type != -1) {
            ImGuizmo::SetOrthographic(false);
            ImGuizmo::SetDrawlist();

            auto &tc = selected_entity.get_component<TransformComponent>();

            ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, ImGui::GetWindowWidth(), ImGui::GetWindowHeight());

            glm::mat4 mod_mat = tc.calculate_matrix();
            glm::mat4 delta = glm::mat4{1.0};
            ImGuizmo::Manipulate(glm::value_ptr(camera->getView()), glm::value_ptr(camera->GetProjection()), static_cast<ImGuizmo::OPERATION>(gizmo_type), ImGuizmo::LOCAL, glm::value_ptr(mod_mat), glm::value_ptr(delta), nullptr, nullptr, nullptr);

            if (ImGuizmo::IsUsing()) {
                glm::vec3 translation, rotation, scale;
                Math::decompose_transform(mod_mat, translation, rotation, scale);

                tc.translation += translation - tc.translation;
                tc.rotation += rotation - tc.rotation;
                tc.scale += scale - tc.scale;
                tc.is_dirty = true;
            }
        }
        ImGui::End();
        ImGui::PopStyleVar();
    }

    void ViewportPanel::update_image(VkSampler sampler, VkImageView imageView) {
        image = ImGui_ImplVulkan_AddTexture(sampler, imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        should_resize = false;
    }
}