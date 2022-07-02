//
// Created by lukas on 23.04.22.
//

#include "ContentBrowserPanel.h"

#include <imgui.h>
#include <imgui_impl_vulkan.h>

namespace Engine {

    // Once we have projects, change this
    extern const std::filesystem::path g_AssetPath = "assets";

    ContentBrowserPanel::ContentBrowserPanel(std::shared_ptr<Device> device) : m_CurrentDirectory(g_AssetPath) {
        m_FileIcon = std::make_unique<Texture>(device, "assets/file.png");
        m_DirectoryIcon = std::make_unique<Texture>(device, "assets/directory.png");
    }

    void ContentBrowserPanel::FileTree(const std::filesystem::path &path) {
        for (auto &directoryEntry: std::filesystem::directory_iterator(path)) {
            if (directoryEntry.is_directory()) {
                if (ImGui::TreeNodeEx(directoryEntry.path().filename().c_str())) {
                    if (ImGui::IsItemHovered() && ImGui::GetMouseClickedCount(ImGuiMouseButton_Left) == 1) {
                        if (directoryEntry.is_directory())
                            m_CurrentDirectory = directoryEntry.path();
                    }
                    FileTree(directoryEntry.path());
                    ImGui::TreePop();
                }
            }
        }
    }

    void ContentBrowserPanel::OnImGuiRender() {
        static auto file_set = ImGui_ImplVulkan_AddTexture(m_FileIcon->GetSampler(), m_FileIcon->GetImageView(),
                                                           VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        static auto directory_set = ImGui_ImplVulkan_AddTexture(m_DirectoryIcon->GetSampler(),
                                                                m_DirectoryIcon->GetImageView(),
                                                                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        ImGui::Begin("Content Browser");

        static bool start = true;


        ImGui::Columns(2);
        if (start) {
            ImGui::SetColumnWidth(0, ImGui::GetWindowWidth() * 0.16);
            start = false;
        }

        ImGui::BeginChild("##folderTree");
        FileTree(g_AssetPath);
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
        static float thumbnailSize = 96.0f;
        float cellSize = thumbnailSize + padding;

        float panelWidth = ImGui::GetContentRegionAvail().x;
        int columnCount = (int) (panelWidth / cellSize);
        if (columnCount < 1)
            columnCount = 1;

        ImGui::BeginChild("##list");

        ImGui::Columns(columnCount, 0, false);

        for (auto &directoryEntry: std::filesystem::directory_iterator(m_CurrentDirectory)) {
            const auto &path = directoryEntry.path();
            auto relativePath = std::filesystem::relative(path, g_AssetPath);
            std::string filenameString = relativePath.filename().string();

            ImGui::PushID(filenameString.c_str());
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetColumnWidth() / 2) - (thumbnailSize / 2));
            if (directoryEntry.is_directory()) {
                ImGui::ImageButton(directory_set, {thumbnailSize, thumbnailSize});
            } else {
                ImGui::ImageButton(file_set, {thumbnailSize, thumbnailSize});
            }

            ImGui::PopStyleColor();
            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                if (directoryEntry.is_directory())
                    m_CurrentDirectory /= path.filename();

            }

            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetColumnWidth() / 2) -
                                 (ImGui::CalcTextSize(filenameString.c_str()).x / 2));
            ImGui::TextWrapped("%s", filenameString.c_str());

            ImGui::NextColumn();

            ImGui::PopID();
        }

        ImGui::EndChild();

        ImGui::Columns(1);

        /*ImGui::SliderFloat("Thumbnail Size", &thumbnailSize, 16, 512);
        ImGui::SliderFloat("Padding", &padding, 0, 32);*/

        // TODO: status bar
        ImGui::End();
    }

}