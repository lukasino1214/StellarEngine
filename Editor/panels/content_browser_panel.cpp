#include "content_browser_panel.h"

#define VK_NO_PROTOTYPES
#include <imgui.h>
#include <imgui_impl_vulkan.h>

namespace Engine {
    extern const std::filesystem::path asset_path = "assets";

    ContentBrowserPanel::ContentBrowserPanel(std::shared_ptr<Device> device) : current_directory(asset_path) {
        file_icon = std::make_unique<Texture>(device, "assets/file.png");
        directory_icon = std::make_unique<Texture>(device, "assets/directory.png");
    }

    void ContentBrowserPanel::file_tree(const std::filesystem::path &path) {
        for (auto &directory_entry: std::filesystem::directory_iterator(path)) {
            if (directory_entry.is_directory()) {
                if (ImGui::TreeNodeEx(directory_entry.path().filename().c_str())) {
                    if (ImGui::IsItemHovered() && ImGui::GetMouseClickedCount(ImGuiMouseButton_Left) == 1) {
                        if (directory_entry.is_directory())
                            current_directory = directory_entry.path();
                    }
                    file_tree(directory_entry.path());
                    ImGui::TreePop();
                }
            }
        }
    }

    void ContentBrowserPanel::render() {
        static auto file_set = ImGui_ImplVulkan_AddTexture(file_icon->get_sampler(), file_icon->get_image_view(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        static auto directory_set = ImGui_ImplVulkan_AddTexture(directory_icon->get_sampler(), directory_icon->get_image_view(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        ImGui::Begin("Content Browser");

        static bool start = true;

        ImGui::Columns(2);
        if (start) {
            ImGui::SetColumnWidth(0, ImGui::GetWindowWidth() * 0.16f);
            start = false;
        }

        ImGui::BeginChild("##folderTree");
        file_tree(asset_path);
        ImGui::EndChild();

        ImGui::NextColumn();

        /*if (m_CurrentDirectory != std::filesystem::path(g_AssetPath))
        {
            if (ImGui::Button("<-"))
            {
                m_CurrentDirectory = m_CurrentDirectory.parent_path();
            }
        }*/

        static float padding = 16.0f;
        static float thumbnail_size = 96.0f;
        float cell_size = thumbnail_size + padding;

        float panelWidth = ImGui::GetContentRegionAvail().x;
        i32 column_count = static_cast<i32>(panelWidth / cell_size);
        if (column_count < 1)
            column_count = 1;

        ImGui::BeginChild("##list");

        ImGui::Columns(column_count, 0, false);

        for (auto &directory_entry: std::filesystem::directory_iterator(current_directory)) {
            const auto &path = directory_entry.path();
            auto relative_path = std::filesystem::relative(path, asset_path);
            std::string filename_string = relative_path.filename().string();

            ImGui::PushID(filename_string.c_str());
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetColumnWidth() / 2) - (thumbnail_size / 2));
            if (directory_entry.is_directory()) {
                ImGui::ImageButton(directory_set, {thumbnail_size, thumbnail_size});
            } else {
                ImGui::ImageButton(file_set, {thumbnail_size, thumbnail_size});
            }

            ImGui::PopStyleColor();
            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                if (directory_entry.is_directory())
                    current_directory /= path.filename();

            }

            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetColumnWidth() / 2) - (ImGui::CalcTextSize(filename_string.c_str()).x / 2));
            ImGui::TextWrapped("%s", filename_string.c_str());

            ImGui::NextColumn();

            ImGui::PopID();
        }

        ImGui::EndChild();
        ImGui::Columns(1);
        ImGui::End();
    }

}