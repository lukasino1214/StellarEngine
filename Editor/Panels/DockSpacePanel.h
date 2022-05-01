//
// Created by lukas on 24.04.22.
//

#ifndef ENGINE_DOCKSPACEPANEL_H
#define ENGINE_DOCKSPACEPANEL_H

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

namespace Engine {
    class DockSpacePanel {
    public:
        DockSpacePanel() = default;

        void OnImGuiRender();
    private:
        bool p_open = true;
        bool opt_fullscreen = true;
        bool opt_padding = false;
        ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    };
}

#endif //ENGINE_DOCKSPACEPANEL_H
