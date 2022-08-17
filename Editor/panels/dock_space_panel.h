#pragma once

#include <imgui.h>

namespace Engine {
    class DockSpacePanel {
    public:
        DockSpacePanel() = default;

        void render();

        bool is_preferences_panel_enabled = false;
    private:
        bool p_open = true;
        bool opt_fullscreen = true;
        bool opt_padding = false;
        ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    };
}