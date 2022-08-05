#include "preferences_panel.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>

#include <yaml-cpp/yaml.h>

#include <filesystem>
#include <fstream>
#include <iostream>

namespace YAML {
    template<>
    struct convert<ImVec2> {
        static Node encode(const ImVec2 &rhs) {
            Node node;
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            node.SetStyle(EmitterStyle::Flow);
            return node;
        }

        static bool decode(const Node &node, ImVec2 &rhs) {
            if (!node.IsSequence() || node.size() != 2)
                return false;

            rhs.x = node[0].as<float>();
            rhs.y = node[1].as<float>();
            return true;
        }
    };

    template<>
    struct convert<ImVec4> {
        static Node encode(const ImVec4 &rhs) {
            Node node;
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            node.push_back(rhs.z);
            node.push_back(rhs.w);
            node.SetStyle(EmitterStyle::Flow);
            return node;
        }

        static bool decode(const Node &node, ImVec4 &rhs) {
            if (!node.IsSequence() || node.size() != 4)
                return false;

            rhs.x = node[0].as<float>();
            rhs.y = node[1].as<float>();
            rhs.z = node[2].as<float>();
            rhs.w = node[3].as<float>();
            return true;
        }
    };

}

namespace Engine {
    YAML::Emitter &operator<<(YAML::Emitter &out, const ImVec2 &v) {
        out << YAML::Flow;
        out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
        return out;
    }

    YAML::Emitter &operator<<(YAML::Emitter &out, const ImVec4 &v) {
        out << YAML::Flow;
        out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
        return out;
    }

    PreferencesPanel::PreferencesPanel() {
        if(std::filesystem::exists("editor_colors.config")) {
            read_config_file();
        } else {
            set_style();
            set_default_colors();
            set_engine_default_colors();
        }

        apply_style();
    }

    static inline void color_picker(const char* text, ImVec4 &color) {
        if(ImGui::TreeNode(text)) {
            ImGui::ColorPicker4(text, reinterpret_cast<float*>(&color));
            ImGui::TreePop();
        }
    }

    static inline void drag_float(const char* text, float &value) {
        if(ImGui::TreeNode(text)) {
            ImGui::DragFloat(text, &value);
            ImGui::TreePop();
        }
    }

    static inline void drag_float2(const char* text, ImVec2 &value) {
        if(ImGui::TreeNode(text)) {
            ImGui::DragFloat2(text, reinterpret_cast<float*>(&value));
            ImGui::TreePop();
        }
    }

    static inline void check_mark(const char* text, bool &value) {
        if(ImGui::TreeNode(text)) {
            ImGui::Checkbox(text, &value);
            ImGui::TreePop();
        }
    }

    static inline void slider_int(const char* text, int &value, int min = 0, int max = 3) {
        if(ImGui::TreeNode(text)) {
            ImGui::SliderInt(text, &value, min, max);
            ImGui::TreePop();
        }
    }

    void PreferencesPanel::render() {
        ImGui::Begin("Preference Panel");

        drag_float("Alpha", style.Alpha);
        drag_float("Disabled Alpha", style.DisabledAlpha);
        drag_float2("Window Padding", style.WindowPadding);
        drag_float("Window Rounding", style.WindowRounding);
        drag_float("Window Border Size", style.WindowBorderSize);
        drag_float2("Window Minimal Size", style.WindowMinSize);
        drag_float2("Window Title Alignment", style.WindowTitleAlign);
        slider_int("Window Menu Button Position", style.WindowMenuButtonPosition);
        drag_float("Child Rounding", style.ChildRounding);
        drag_float("Child Border Size", style.ChildBorderSize);
        drag_float("Popup Rounding", style.PopupRounding);
        drag_float("Popup Border Size", style.PopupBorderSize);
        drag_float2("Frame Padding", style.FramePadding);
        drag_float("Frame Rounding", style.FrameRounding);
        drag_float("Frame Border Size", style.FrameBorderSize);
        drag_float2("Item Spacing", style.ItemSpacing);
        drag_float2("Item Inner Spacing", style.ItemInnerSpacing);
        drag_float2("Cell Padding", style.CellPadding);
        drag_float2("Touch Extra Padding", style.TouchExtraPadding);
        drag_float("Indent Spacing", style.IndentSpacing);
        drag_float("Columns Minimal Spacing", style.ColumnsMinSpacing);
        drag_float("Scrollbar Size", style.ScrollbarSize);
        drag_float("Scrollbar Rounding", style.ScrollbarRounding);
        drag_float("Grab Minimal Size", style.GrabMinSize);
        drag_float("Grab Rounding", style.GrabRounding);
        drag_float("Log Slider Deadzone", style.LogSliderDeadzone);
        drag_float("Tab Rounding", style.TabRounding);
        drag_float("Tab Border Size", style.TabBorderSize);
        drag_float("Tab Minimal Width For Close Button", style.TabMinWidthForCloseButton);
        slider_int("Color Button Position", style.ColorButtonPosition);
        drag_float2("Button Text Align", style.ButtonTextAlign);
        drag_float2("Selectable Text Align", style.SelectableTextAlign);
        drag_float2("Display Window Padding", style.DisplayWindowPadding);
        drag_float2("Display Safe Area Padding", style.DisplaySafeAreaPadding);
        drag_float("Log Slider Deadzone", style.LogSliderDeadzone);
        check_mark("AntiAliased Lines", style.AntiAliasedLines);
        check_mark("AntiAliased LinesUseTex", style.AntiAliasedLinesUseTex);
        check_mark("AntiAliased Fill", style.AntiAliasedFill);
        drag_float("Tab Rounding", style.TabRounding);
        drag_float("Tab Border Size", style.TabBorderSize);

        auto& colors = style.Color;

        color_picker("Text Color", colors.Text);
        color_picker("Text Disabled Color", colors.TextDisabled);
        color_picker("Window Background Color", colors.WindowBg);
        color_picker("Children Background Color", colors.ChildBg);
        color_picker("Popup Background Color", colors.PopupBg);
        color_picker("Border Color", colors.Border);
        color_picker("Border Shadow Color", colors.BorderShadow);
        color_picker("Frame Background Color", colors.FrameBg);
        color_picker("Frame Background Hovered Color", colors.FrameBgHovered);
        color_picker("Frame Background Active Color", colors.FrameBgActive);
        color_picker("Title Background Color", colors.TitleBg);
        color_picker("Title Background Active Color", colors.TitleBgActive);
        color_picker("Title Background Collapsed Color", colors.TitleBgCollapsed);
        color_picker("Menu Bar Background Color", colors.MenuBarBg);
        color_picker("Scrollbar Background Color", colors.ScrollbarBg);
        color_picker("Scrollbar Grab Color", colors.ScrollbarGrab);
        color_picker("Scrollbar Grab Hovered Color", colors.ScrollbarGrabHovered);
        color_picker("Scrollbar Grab Active Color", colors.ScrollbarGrabActive);
        color_picker("Check Mark Color", colors.CheckMark);
        color_picker("Slider Grab Color", colors.SliderGrab);
        color_picker("Slider Grab Active Color", colors.SliderGrabActive);
        color_picker("Button Color", colors.Button);
        color_picker("Button Hovered Color", colors.ButtonHovered);
        color_picker("Button Active Color", colors.ButtonActive);
        color_picker("Header Color", colors.Header);
        color_picker("Header Hovered Color", colors.HeaderHovered);
        color_picker("Header Active Color", colors.HeaderActive);
        color_picker("Separator Color", colors.Separator);
        color_picker("Separator Hovered Color", colors.SeparatorHovered);
        color_picker("Separator Active Color", colors.SeparatorActive);
        color_picker("Resize Grip Color", colors.ResizeGrip);
        color_picker("Resize Grip Hovered Color", colors.ResizeGripHovered);
        color_picker("Resize Grip Active Color", colors.ResizeGripActive);
        color_picker("Tab Color", colors.Tab);
        color_picker("Tab Hovered Color", colors.TabHovered);
        color_picker("Tab Active Color", colors.TabActive);
        color_picker("Tab Unfocused Color", colors.TabUnfocused);
        color_picker("Tab Unfocused Active Color", colors.TabUnfocusedActive);
        color_picker("Docking Preview Color", colors.DockingPreview);
        color_picker("Docking Empty Background Color", colors.DockingEmptyBg);
        color_picker("Plot Lines Color", colors.PlotLines);
        color_picker("Plot Lines Hovered Color", colors.PlotLinesHovered);
        color_picker("Plot Histogram Color", colors.PlotHistogram);
        color_picker("Plot Histogram Hovered Color", colors.PlotHistogramHovered);
        color_picker("Table Header Background Color", colors.TableHeaderBg);
        color_picker("Table Border Strong Color", colors.TableBorderStrong);
        color_picker("Table Border Light Color", colors.TableBorderLight);
        color_picker("Table Row Background Color", colors.TableRowBg);
        color_picker("Table Row Background Alternative Color", colors.TableRowBgAlt);
        color_picker("Text Selected Background Color", colors.TextSelectedBg);
        color_picker("Drag Drop Target Color", colors.DragDropTarget);
        color_picker("Nav Highlight Color", colors.NavHighlight);
        color_picker("Nav Windowing Highlight Color", colors.NavWindowingHighlight);
        color_picker("Nav Windowing Dim Bg Color", colors.NavWindowingDimBg);
        color_picker("Modal Window Dim Bg Color", colors.ModalWindowDimBg);

        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x + ImGui::GetScrollX() - (ImGui::CalcTextSize("Apply").x + style.FramePadding.x * 2.0f) - 4.0f);
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ImGui::GetContentRegionAvail().y + ImGui::GetScrollY() - (ImGui::CalcTextSize("Apply").y + style.FramePadding.y * 2.0f) - 4.0f);
        if(ImGui::Button("Apply")) {
            apply_style();
        }

        ImGui::End();
    }

    void PreferencesPanel::set_style() {
        style.Alpha                      = 1.0f;
        style.DisabledAlpha              = 0.60f;
        style.WindowPadding              = ImVec2(8,8);
        style.WindowRounding             = 0.0f;
        style.WindowBorderSize           = 1.0f;
        style.WindowMinSize              = ImVec2(32,32);
        style.WindowTitleAlign           = ImVec2(0.0f,0.5f);
        style.WindowMenuButtonPosition   = ImGuiDir_Left;
        style.ChildRounding              = 0.0f;
        style.ChildBorderSize            = 1.0f;
        style.PopupRounding              = 0.0f;
        style.PopupBorderSize            = 1.0f;
        style.FramePadding               = ImVec2(4,3);
        style.FrameRounding              = 0.0f;
        style.FrameBorderSize            = 0.0f;
        style.ItemSpacing                = ImVec2(8,4);
        style.ItemInnerSpacing           = ImVec2(4,4);
        style.CellPadding                = ImVec2(4,2);
        style.TouchExtraPadding          = ImVec2(0,0);
        style.IndentSpacing              = 21.0f;
        style.ColumnsMinSpacing          = 6.0f;
        style.ScrollbarSize              = 14.0f;
        style.ScrollbarRounding          = 9.0f;
        style.GrabMinSize                = 12.0f;
        style.GrabRounding               = 0.0f;
        style.LogSliderDeadzone          = 4.0f;
        style.TabRounding                = 4.0f;
        style.TabBorderSize              = 0.0f;
        style.TabMinWidthForCloseButton  = 0.0f;
        style.ColorButtonPosition        = ImGuiDir_Right;
        style.ButtonTextAlign            = ImVec2(0.5f,0.5f);
        style.SelectableTextAlign        = ImVec2(0.0f,0.0f);
        style.DisplayWindowPadding       = ImVec2(19,19);
        style.DisplaySafeAreaPadding     = ImVec2(3,3);
        style.MouseCursorScale           = 1.0f;
        style.AntiAliasedLines           = true;
        style.AntiAliasedLinesUseTex     = true;
        style.AntiAliasedFill            = true;
        style.CurveTessellationTol       = 1.25f;
        style.CircleTessellationMaxError = 0.30f;
    }

    void PreferencesPanel::set_default_colors() {
        auto& colors = style.Color;

        colors.Text                  = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
        colors.TextDisabled          = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
        colors.WindowBg              = ImVec4(0.06f, 0.06f, 0.06f, 0.94f);
        colors.ChildBg               = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors.PopupBg               = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
        colors.Border                = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
        colors.BorderShadow          = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors.FrameBg               = ImVec4(0.16f, 0.29f, 0.48f, 0.54f);
        colors.FrameBgHovered        = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
        colors.FrameBgActive         = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
        colors.TitleBg               = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
        colors.TitleBgActive         = ImVec4(0.16f, 0.29f, 0.48f, 1.00f);
        colors.TitleBgCollapsed      = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
        colors.MenuBarBg             = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
        colors.ScrollbarBg           = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
        colors.ScrollbarGrab         = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
        colors.ScrollbarGrabHovered  = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
        colors.ScrollbarGrabActive   = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
        colors.CheckMark             = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        colors.SliderGrab            = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
        colors.SliderGrabActive      = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        colors.Button                = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
        colors.ButtonHovered         = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        colors.ButtonActive          = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
        colors.Header                = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
        colors.HeaderHovered         = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
        colors.HeaderActive          = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        colors.Separator             = colors.Border;
        colors.SeparatorHovered      = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
        colors.SeparatorActive       = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
        colors.ResizeGrip            = ImVec4(0.26f, 0.59f, 0.98f, 0.20f);
        colors.ResizeGripHovered     = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
        colors.ResizeGripActive      = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
        colors.Tab                   = ImLerp(colors.Header, colors.TitleBgActive, 0.80f);
        colors.TabHovered            = colors.HeaderHovered;
        colors.TabActive             = ImLerp(colors.HeaderActive, colors.TitleBgActive, 0.60f);
        colors.TabUnfocused          = ImLerp(colors.Tab, colors.TitleBg, 0.80f);
        colors.TabUnfocusedActive    = ImLerp(colors.TabActive, colors.TitleBg, 0.40f);
        colors.DockingPreview        = colors.HeaderActive * ImVec4(1.0f, 1.0f, 1.0f, 0.7f);
        colors.DockingEmptyBg        = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
        colors.PlotLines             = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
        colors.PlotLinesHovered      = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
        colors.PlotHistogram         = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
        colors.PlotHistogramHovered  = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
        colors.TableHeaderBg         = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
        colors.TableBorderStrong     = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
        colors.TableBorderLight      = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
        colors.TableRowBg            = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors.TableRowBgAlt         = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
        colors.TextSelectedBg        = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
        colors.DragDropTarget        = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
        colors.NavHighlight          = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        colors.NavWindowingHighlight = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
        colors.NavWindowingDimBg     = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
        colors.ModalWindowDimBg      = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
    }

    void PreferencesPanel::set_engine_default_colors() {
        auto& colors = style.Color;

        // Headers
        colors.Header             = ImVec4{0.2f, 0.205f, 0.21f, 1.0f};
        colors.HeaderHovered      = ImVec4{0.3f, 0.305f, 0.31f, 1.0f};
        colors.HeaderActive       = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};

        // Buttons
        colors.Button             = ImVec4{0.2f, 0.205f, 0.21f, 1.0f};
        colors.ButtonHovered      = ImVec4{0.3f, 0.305f, 0.31f, 1.0f};
        colors.ButtonActive       = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};

        // Frame BG
        colors.FrameBg            = ImVec4{0.2f, 0.205f, 0.21f, 1.0f};
        colors.FrameBgHovered     = ImVec4{0.3f, 0.305f, 0.31f, 1.0f};
        colors.FrameBgActive      = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};

        // Tabs
        colors.Tab                = ImVec4{0.20f, 0.2005f, 0.201f, 1.0f};
        colors.TabHovered         = ImVec4{0.38f, 0.3805f, 0.381f, 1.0f};
        colors.TabActive          = ImVec4{0.28f, 0.2805f, 0.281f, 1.0f};
        colors.TabUnfocused       = ImVec4{0.20f, 0.2005f, 0.201f, 1.0f};
        colors.TabUnfocusedActive = ImVec4{0.2f, 0.205f, 0.21f, 1.0f};

        // Title
        colors.TitleBg            = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};
        colors.TitleBgActive      = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};
        colors.TitleBgCollapsed   = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};
    }

    void PreferencesPanel::read_config_file() {
        YAML::Node data;
        try {
            data = YAML::LoadFile("editor_colors.config");
        }
        catch (YAML::ParserException e) {
            throw std::runtime_error(e.msg);
        }

        style.Alpha                      = data["Alpha"].as<float>();
        style.DisabledAlpha              = data["DisabledAlpha"].as<float>();
        style.WindowPadding              = data["WindowPadding"].as<ImVec2>();
        style.WindowRounding             = data["WindowRounding"].as<float>();
        style.WindowBorderSize           = data["WindowBorderSize"].as<float>();
        style.WindowMinSize              = data["WindowMinSize"].as<ImVec2>();
        style.WindowTitleAlign           = data["WindowTitleAlign"].as<ImVec2>();
        style.WindowMenuButtonPosition   = data["WindowMenuButtonPosition"].as<ImGuiDir>();
        style.ChildRounding              = data["ChildRounding"].as<float>();
        style.ChildBorderSize            = data["ChildBorderSize"].as<float>();
        style.PopupRounding              = data["PopupRounding"].as<float>();
        style.PopupBorderSize            = data["PopupBorderSize"].as<float>();
        style.FramePadding               = data["FramePadding"].as<ImVec2>();
        style.FrameRounding              = data["FrameRounding"].as<float>();
        style.FrameBorderSize            = data["FrameBorderSize"].as<float>();
        style.ItemSpacing                = data["ItemSpacing"].as<ImVec2>();
        style.ItemInnerSpacing           = data["ItemInnerSpacing"].as<ImVec2>();
        style.CellPadding                = data["CellPadding"].as<ImVec2>();
        style.TouchExtraPadding          = data["TouchExtraPadding"].as<ImVec2>();
        style.IndentSpacing              = data["IndentSpacing"].as<float>();
        style.ColumnsMinSpacing          = data["ColumnsMinSpacing"].as<float>();
        style.ScrollbarSize              = data["ScrollbarSize"].as<float>();
        style.ScrollbarRounding          = data["ScrollbarRounding"].as<float>();
        style.GrabMinSize                = data["GrabMinSize"].as<float>();
        style.GrabRounding               = data["GrabRounding"].as<float>();
        style.LogSliderDeadzone          = data["LogSliderDeadzone"].as<float>();
        style.TabRounding                = data["TabRounding"].as<float>();
        style.TabBorderSize              = data["TabBorderSize"].as<float>();
        style.TabMinWidthForCloseButton  = data["TabMinWidthForCloseButton"].as<float>();
        style.ColorButtonPosition        = data["ColorButtonPosition"].as<ImGuiDir>();
        style.ButtonTextAlign            = data["ButtonTextAlign"].as<ImVec2>();
        style.SelectableTextAlign        = data["SelectableTextAlign"].as<ImVec2>();
        style.DisplayWindowPadding       = data["DisplayWindowPadding"].as<ImVec2>();
        style.DisplaySafeAreaPadding     = data["DisplaySafeAreaPadding"].as<ImVec2>();
        style.MouseCursorScale           = data["MouseCursorScale"].as<float>();
        style.AntiAliasedLines           = data["AntiAliasedLines"].as<bool>();
        style.AntiAliasedLinesUseTex     = data["AntiAliasedLinesUseTex"].as<bool>();
        style.AntiAliasedFill            = data["AntiAliasedFill"].as<bool>();
        style.CurveTessellationTol       = data["CurveTessellationTol"].as<float>();
        style.CircleTessellationMaxError = data["CircleTessellationMaxError"].as<float>();

        auto& colors = style.Color;

        colors.Text                  = data["Text"].as<ImVec4>();
        colors.TextDisabled          = data["TextDisabled"].as<ImVec4>();
        colors.WindowBg              = data["WindowBg"].as<ImVec4>();
        colors.ChildBg               = data["ChildBg"].as<ImVec4>();
        colors.PopupBg               = data["PopupBg"].as<ImVec4>();
        colors.Border                = data["Border"].as<ImVec4>();
        colors.BorderShadow          = data["BorderShadow"].as<ImVec4>();
        colors.FrameBg               = data["FrameBg"].as<ImVec4>();
        colors.FrameBgHovered        = data["FrameBgHovered"].as<ImVec4>();
        colors.FrameBgActive         = data["FrameBgActive"].as<ImVec4>();
        colors.TitleBg               = data["TitleBg"].as<ImVec4>();
        colors.TitleBgActive         = data["TitleBgActive"].as<ImVec4>();
        colors.TitleBgCollapsed      = data["TitleBgCollapsed"].as<ImVec4>();
        colors.MenuBarBg             = data["MenuBarBg"].as<ImVec4>();
        colors.ScrollbarBg           = data["ScrollbarBg"].as<ImVec4>();
        colors.ScrollbarGrab         = data["ScrollbarGrab"].as<ImVec4>();
        colors.ScrollbarGrabHovered  = data["ScrollbarGrabHovered"].as<ImVec4>();
        colors.ScrollbarGrabActive   = data["ScrollbarGrabActive"].as<ImVec4>();
        colors.CheckMark             = data["CheckMark"].as<ImVec4>();
        colors.SliderGrab            = data["SliderGrab"].as<ImVec4>();
        colors.SliderGrabActive      = data["SliderGrabActive"].as<ImVec4>();
        colors.Button                = data["Button"].as<ImVec4>();
        colors.ButtonHovered         = data["ButtonHovered"].as<ImVec4>();
        colors.ButtonActive          = data["ButtonActive"].as<ImVec4>();
        colors.Header                = data["Header"].as<ImVec4>();
        colors.HeaderHovered         = data["HeaderHovered"].as<ImVec4>();
        colors.HeaderActive          = data["HeaderActive"].as<ImVec4>();
        colors.Separator             = data["Separator"].as<ImVec4>();
        colors.SeparatorHovered      = data["SeparatorHovered"].as<ImVec4>();
        colors.SeparatorActive       = data["SeparatorActive"].as<ImVec4>();
        colors.ResizeGrip            = data["ResizeGrip"].as<ImVec4>();
        colors.ResizeGripHovered     = data["ResizeGripHovered"].as<ImVec4>();
        colors.ResizeGripActive      = data["ResizeGripActive"].as<ImVec4>();
        colors.Tab                   = data["Tab"].as<ImVec4>();
        colors.TabHovered            = data["TabHovered"].as<ImVec4>();
        colors.TabActive             = data["TabActive"].as<ImVec4>();
        colors.TabUnfocused          = data["TabUnfocused"].as<ImVec4>();
        colors.TabUnfocusedActive    = data["TabUnfocusedActive"].as<ImVec4>();
        colors.DockingPreview        = data["DockingPreview"].as<ImVec4>();
        colors.DockingEmptyBg        = data["DockingEmptyBg"].as<ImVec4>();
        colors.PlotLines             = data["PlotLines"].as<ImVec4>();
        colors.PlotLinesHovered      = data["PlotLinesHovered"].as<ImVec4>();
        colors.PlotHistogram         = data["PlotHistogram"].as<ImVec4>();
        colors.PlotHistogramHovered  = data["PlotHistogramHovered"].as<ImVec4>();
        colors.TableHeaderBg         = data["TableHeaderBg"].as<ImVec4>();
        colors.TableBorderStrong     = data["TableBorderStrong"].as<ImVec4>();
        colors.TableBorderLight      = data["TableBorderLight"].as<ImVec4>();
        colors.TableRowBg            = data["TableRowBg"].as<ImVec4>();
        colors.TableRowBgAlt         = data["TableRowBgAlt"].as<ImVec4>();
        colors.TextSelectedBg        = data["TextSelectedBg"].as<ImVec4>();
        colors.DragDropTarget        = data["DragDropTarget"].as<ImVec4>();
        colors.NavHighlight          = data["NavHighlight"].as<ImVec4>();
        colors.NavWindowingHighlight = data["NavWindowingHighlight"].as<ImVec4>();
        colors.NavWindowingDimBg     = data["NavWindowingDimBg"].as<ImVec4>();
        colors.ModalWindowDimBg      = data["ModalWindowDimBg"].as<ImVec4>();


    }

    void PreferencesPanel::save_config_file() {
        YAML::Emitter out;
        out << YAML::BeginMap;

        out << YAML::Key << "Alpha"                      << YAML::Value << style.Alpha;
        out << YAML::Key << "DisabledAlpha"              << YAML::Value << style.DisabledAlpha;
        out << YAML::Key << "WindowPadding"              << YAML::Value << style.WindowPadding;
        out << YAML::Key << "WindowRounding"             << YAML::Value << style.WindowRounding;
        out << YAML::Key << "WindowBorderSize"           << YAML::Value << style.WindowBorderSize;
        out << YAML::Key << "WindowMinSize"              << YAML::Value << style.WindowMinSize;
        out << YAML::Key << "WindowTitleAlign"           << YAML::Value << style.WindowTitleAlign;
        out << YAML::Key << "WindowMenuButtonPosition"   << YAML::Value << style.WindowMenuButtonPosition;
        out << YAML::Key << "ChildRounding"              << YAML::Value << style.ChildRounding;
        out << YAML::Key << "ChildBorderSize"            << YAML::Value << style.ChildBorderSize;
        out << YAML::Key << "PopupRounding"              << YAML::Value << style.PopupRounding;
        out << YAML::Key << "PopupBorderSize"            << YAML::Value << style.PopupBorderSize;
        out << YAML::Key << "FramePadding"               << YAML::Value << style.FramePadding;
        out << YAML::Key << "FrameRounding"              << YAML::Value << style.FrameRounding;
        out << YAML::Key << "FrameBorderSize"            << YAML::Value << style.FrameBorderSize;
        out << YAML::Key << "ItemSpacing"                << YAML::Value << style.ItemSpacing;
        out << YAML::Key << "ItemInnerSpacing"           << YAML::Value << style.ItemInnerSpacing;
        out << YAML::Key << "CellPadding"                << YAML::Value << style.CellPadding;
        out << YAML::Key << "TouchExtraPadding"          << YAML::Value << style.TouchExtraPadding;
        out << YAML::Key << "IndentSpacing"              << YAML::Value << style.IndentSpacing;
        out << YAML::Key << "ColumnsMinSpacing"          << YAML::Value << style.ColumnsMinSpacing;
        out << YAML::Key << "ScrollbarSize"              << YAML::Value << style.ScrollbarSize;
        out << YAML::Key << "ScrollbarRounding"          << YAML::Value << style.ScrollbarRounding;
        out << YAML::Key << "GrabMinSize"                << YAML::Value << style.GrabMinSize;
        out << YAML::Key << "GrabRounding"               << YAML::Value << style.GrabRounding;
        out << YAML::Key << "LogSliderDeadzone"          << YAML::Value << style.LogSliderDeadzone;
        out << YAML::Key << "TabRounding"                << YAML::Value << style.TabRounding;
        out << YAML::Key << "TabBorderSize"              << YAML::Value << style.TabBorderSize;
        out << YAML::Key << "TabMinWidthForCloseButton"  << YAML::Value << style.TabMinWidthForCloseButton;
        out << YAML::Key << "ColorButtonPosition"        << YAML::Value << style.ColorButtonPosition;
        out << YAML::Key << "ButtonTextAlign"            << YAML::Value << style.ButtonTextAlign;
        out << YAML::Key << "SelectableTextAlign"        << YAML::Value << style.SelectableTextAlign;
        out << YAML::Key << "DisplayWindowPadding"       << YAML::Value << style.DisplayWindowPadding;
        out << YAML::Key << "DisplaySafeAreaPadding"     << YAML::Value << style.DisplaySafeAreaPadding;
        out << YAML::Key << "MouseCursorScale"           << YAML::Value << style.MouseCursorScale;
        out << YAML::Key << "AntiAliasedLines"           << YAML::Value << style.AntiAliasedLines;
        out << YAML::Key << "AntiAliasedLinesUseTex"     << YAML::Value << style.AntiAliasedLinesUseTex;
        out << YAML::Key << "AntiAliasedFill"            << YAML::Value << style.AntiAliasedFill;
        out << YAML::Key << "CurveTessellationTol"       << YAML::Value << style.CurveTessellationTol;
        out << YAML::Key << "CircleTessellationMaxError" << YAML::Value << style.CircleTessellationMaxError;

        auto& colors = style.Color;

        out << YAML::Key << "Text"                  << YAML::Value << colors.Text;
        out << YAML::Key << "TextDisabled"          << YAML::Value << colors.TextDisabled;
        out << YAML::Key << "WindowBg"              << YAML::Value << colors.WindowBg;
        out << YAML::Key << "ChildBg"               << YAML::Value << colors.ChildBg;
        out << YAML::Key << "PopupBg"               << YAML::Value << colors.PopupBg;
        out << YAML::Key << "Border"                << YAML::Value << colors.Border;
        out << YAML::Key << "BorderShadow"          << YAML::Value << colors.BorderShadow;
        out << YAML::Key << "FrameBg"               << YAML::Value << colors.FrameBg;
        out << YAML::Key << "FrameBgHovered"        << YAML::Value << colors.FrameBgHovered;
        out << YAML::Key << "FrameBgActive"         << YAML::Value << colors.FrameBgActive;
        out << YAML::Key << "TitleBg"               << YAML::Value << colors.TitleBg;
        out << YAML::Key << "TitleBgActive"         << YAML::Value << colors.TitleBgActive;
        out << YAML::Key << "TitleBgCollapsed"      << YAML::Value << colors.TitleBgCollapsed;
        out << YAML::Key << "MenuBarBg"             << YAML::Value << colors.MenuBarBg;
        out << YAML::Key << "ScrollbarBg"           << YAML::Value << colors.ScrollbarBg;
        out << YAML::Key << "ScrollbarGrab"         << YAML::Value << colors.ScrollbarGrab;
        out << YAML::Key << "ScrollbarGrabHovered"  << YAML::Value << colors.ScrollbarGrabHovered;
        out << YAML::Key << "ScrollbarGrabActive"   << YAML::Value << colors.ScrollbarGrabActive;
        out << YAML::Key << "CheckMark"             << YAML::Value << colors.CheckMark;
        out << YAML::Key << "SliderGrab"            << YAML::Value << colors.SliderGrab;
        out << YAML::Key << "SliderGrabActive"      << YAML::Value << colors.SliderGrabActive;
        out << YAML::Key << "Button"                << YAML::Value << colors.Button;
        out << YAML::Key << "ButtonHovered"         << YAML::Value << colors.ButtonHovered;
        out << YAML::Key << "ButtonActive"          << YAML::Value << colors.ButtonActive;
        out << YAML::Key << "Header"                << YAML::Value << colors.Header;
        out << YAML::Key << "HeaderHovered"         << YAML::Value << colors.HeaderHovered;
        out << YAML::Key << "HeaderActive"          << YAML::Value << colors.HeaderActive;
        out << YAML::Key << "Separator"             << YAML::Value << colors.Separator;
        out << YAML::Key << "SeparatorHovered"      << YAML::Value << colors.SeparatorHovered;
        out << YAML::Key << "SeparatorActive"       << YAML::Value << colors.SeparatorActive;
        out << YAML::Key << "ResizeGrip"            << YAML::Value << colors.ResizeGrip;
        out << YAML::Key << "ResizeGripHovered"     << YAML::Value << colors.ResizeGripHovered;
        out << YAML::Key << "ResizeGripActive"      << YAML::Value << colors.ResizeGripActive;
        out << YAML::Key << "Tab"                   << YAML::Value << colors.Tab;
        out << YAML::Key << "TabHovered"            << YAML::Value << colors.TabHovered;
        out << YAML::Key << "TabActive"             << YAML::Value << colors.TabActive;
        out << YAML::Key << "TabUnfocused"          << YAML::Value << colors.TabUnfocused;
        out << YAML::Key << "TabUnfocusedActive"    << YAML::Value << colors.TabUnfocusedActive;
        out << YAML::Key << "DockingPreview"        << YAML::Value << colors.DockingPreview;
        out << YAML::Key << "DockingEmptyBg"        << YAML::Value << colors.DockingEmptyBg;
        out << YAML::Key << "PlotLines"             << YAML::Value << colors.PlotLines;
        out << YAML::Key << "PlotLinesHovered"      << YAML::Value << colors.PlotLinesHovered;
        out << YAML::Key << "PlotHistogram"         << YAML::Value << colors.PlotHistogram;
        out << YAML::Key << "PlotHistogramHovered"  << YAML::Value << colors.PlotHistogramHovered;
        out << YAML::Key << "TableHeaderBg"         << YAML::Value << colors.TableHeaderBg;
        out << YAML::Key << "TableBorderStrong"     << YAML::Value << colors.TableBorderStrong;
        out << YAML::Key << "TableBorderLight"      << YAML::Value << colors.TableBorderLight;
        out << YAML::Key << "TableRowBg"            << YAML::Value << colors.TableRowBg;
        out << YAML::Key << "TableRowBgAlt"         << YAML::Value << colors.TableRowBgAlt;
        out << YAML::Key << "TextSelectedBg"        << YAML::Value << colors.TextSelectedBg;
        out << YAML::Key << "DragDropTarget"        << YAML::Value << colors.DragDropTarget;
        out << YAML::Key << "NavHighlight"          << YAML::Value << colors.NavHighlight;
        out << YAML::Key << "NavWindowingHighlight" << YAML::Value << colors.NavWindowingHighlight;
        out << YAML::Key << "NavWindowingDimBg"     << YAML::Value << colors.NavWindowingDimBg;
        out << YAML::Key << "ModalWindowDimBg"      << YAML::Value << colors.ModalWindowDimBg;

        out << YAML::EndMap;

        std::ofstream fout("editor_colors.config");
        fout << out.c_str();
    }

    void PreferencesPanel::apply_style() {
        auto& imgui_style = ImGui::GetStyle();

        imgui_style.Alpha                      = style.Alpha;
        imgui_style.DisabledAlpha              = style.DisabledAlpha;
        imgui_style.WindowPadding              = style.WindowPadding;
        imgui_style.WindowRounding             = style.WindowRounding;
        imgui_style.WindowBorderSize           = style.WindowBorderSize;
        imgui_style.WindowMinSize              = style.WindowMinSize;
        imgui_style.WindowTitleAlign           = style.WindowTitleAlign;
        imgui_style.WindowMenuButtonPosition   = style.WindowMenuButtonPosition;
        imgui_style.ChildRounding              = style.ChildRounding;
        imgui_style.ChildBorderSize            = style.ChildBorderSize;
        imgui_style.PopupRounding              = style.PopupRounding;
        imgui_style.PopupBorderSize            = style.PopupBorderSize;
        imgui_style.FramePadding               = style.FramePadding;
        imgui_style.FrameRounding              = style.FrameRounding;
        imgui_style.FrameBorderSize            = style.FrameBorderSize;
        imgui_style.ItemSpacing                = style.ItemSpacing;
        imgui_style.ItemInnerSpacing           = style.ItemInnerSpacing;
        imgui_style.CellPadding                = style.CellPadding;
        imgui_style.TouchExtraPadding          = style.TouchExtraPadding;
        imgui_style.IndentSpacing              = style.IndentSpacing;
        imgui_style.ColumnsMinSpacing          = style.ColumnsMinSpacing;
        imgui_style.ScrollbarSize              = style.ScrollbarSize;
        imgui_style.ScrollbarRounding          = style.ScrollbarRounding;
        imgui_style.GrabMinSize                = style.GrabMinSize;
        imgui_style.GrabRounding               = style.GrabRounding;
        imgui_style.LogSliderDeadzone          = style.LogSliderDeadzone;
        imgui_style.TabRounding                = style.TabRounding;
        imgui_style.TabBorderSize              = style.TabBorderSize;
        imgui_style.TabMinWidthForCloseButton  = style.TabMinWidthForCloseButton;
        imgui_style.ColorButtonPosition        = style.ColorButtonPosition;
        imgui_style.ButtonTextAlign            = style.ButtonTextAlign;
        imgui_style.SelectableTextAlign        = style.SelectableTextAlign;
        imgui_style.DisplayWindowPadding       = style.DisplayWindowPadding;
        imgui_style.DisplaySafeAreaPadding     = style.DisplaySafeAreaPadding;
        imgui_style.MouseCursorScale           = style.MouseCursorScale;
        imgui_style.AntiAliasedLines           = style.AntiAliasedLines;
        imgui_style.AntiAliasedLinesUseTex     = style.AntiAliasedLinesUseTex;
        imgui_style.AntiAliasedFill            = style.AntiAliasedFill;
        imgui_style.CurveTessellationTol       = style.CurveTessellationTol;
        imgui_style.CircleTessellationMaxError = style.CircleTessellationMaxError;


        auto& imgui_colors = ImGui::GetStyle().Colors;
        auto& colors = style.Color;

        imgui_colors[ImGuiCol_Text]                  = colors.Text;
        imgui_colors[ImGuiCol_TextDisabled]          = colors.TextDisabled;
        imgui_colors[ImGuiCol_WindowBg]              = colors.WindowBg;
        imgui_colors[ImGuiCol_ChildBg]               = colors.ChildBg;
        imgui_colors[ImGuiCol_PopupBg]               = colors.PopupBg;
        imgui_colors[ImGuiCol_Border]                = colors.Border;
        imgui_colors[ImGuiCol_BorderShadow]          = colors.BorderShadow;
        imgui_colors[ImGuiCol_FrameBg]               = colors.FrameBg;
        imgui_colors[ImGuiCol_FrameBgHovered]        = colors.FrameBgHovered;
        imgui_colors[ImGuiCol_FrameBgActive]         = colors.FrameBgActive;
        imgui_colors[ImGuiCol_TitleBg]               = colors.TitleBg;
        imgui_colors[ImGuiCol_TitleBgActive]         = colors.TitleBgActive;
        imgui_colors[ImGuiCol_TitleBgCollapsed]      = colors.TitleBgCollapsed;
        imgui_colors[ImGuiCol_MenuBarBg]             = colors.MenuBarBg;
        imgui_colors[ImGuiCol_ScrollbarBg]           = colors.ScrollbarBg;
        imgui_colors[ImGuiCol_ScrollbarGrab]         = colors.ScrollbarGrab;
        imgui_colors[ImGuiCol_ScrollbarGrabHovered]  = colors.ScrollbarGrabHovered;
        imgui_colors[ImGuiCol_ScrollbarGrabActive]   = colors.ScrollbarGrabActive;
        imgui_colors[ImGuiCol_CheckMark]             = colors.CheckMark;
        imgui_colors[ImGuiCol_SliderGrab]            = colors.SliderGrab;
        imgui_colors[ImGuiCol_SliderGrabActive]      = colors.SliderGrabActive;
        imgui_colors[ImGuiCol_Button]                = colors.Button;
        imgui_colors[ImGuiCol_ButtonHovered]         = colors.ButtonHovered;
        imgui_colors[ImGuiCol_ButtonActive]          = colors.ButtonActive;
        imgui_colors[ImGuiCol_Header]                = colors.Header;
        imgui_colors[ImGuiCol_HeaderHovered]         = colors.HeaderHovered;
        imgui_colors[ImGuiCol_HeaderActive]          = colors.HeaderActive;
        imgui_colors[ImGuiCol_Separator]             = colors.Separator;
        imgui_colors[ImGuiCol_SeparatorHovered]      = colors.SeparatorHovered;
        imgui_colors[ImGuiCol_SeparatorActive]       = colors.SeparatorActive;
        imgui_colors[ImGuiCol_ResizeGrip]            = colors.ResizeGrip;
        imgui_colors[ImGuiCol_ResizeGripHovered]     = colors.ResizeGripHovered;
        imgui_colors[ImGuiCol_ResizeGripActive]      = colors.ResizeGripActive;
        imgui_colors[ImGuiCol_Tab]                   = colors.Tab;
        imgui_colors[ImGuiCol_TabHovered]            = colors.TabHovered;
        imgui_colors[ImGuiCol_TabActive]             = colors.TabActive;
        imgui_colors[ImGuiCol_TabUnfocused]          = colors.TabUnfocused;
        imgui_colors[ImGuiCol_TabUnfocusedActive]    = colors.TabUnfocusedActive;
        imgui_colors[ImGuiCol_DockingPreview]        = colors.DockingPreview;
        imgui_colors[ImGuiCol_DockingEmptyBg]        = colors.DockingEmptyBg;
        imgui_colors[ImGuiCol_PlotLines]             = colors.PlotLines;
        imgui_colors[ImGuiCol_PlotLinesHovered]      = colors.PlotLinesHovered;
        imgui_colors[ImGuiCol_PlotHistogram]         = colors.PlotHistogram;
        imgui_colors[ImGuiCol_PlotHistogramHovered]  = colors.PlotHistogramHovered;
        imgui_colors[ImGuiCol_TableHeaderBg]         = colors.TableHeaderBg;
        imgui_colors[ImGuiCol_TableBorderStrong]     = colors.TableBorderStrong;
        imgui_colors[ImGuiCol_TableBorderLight]      = colors.TableBorderLight;
        imgui_colors[ImGuiCol_TableRowBg]            = colors.TableRowBg;
        imgui_colors[ImGuiCol_TableRowBgAlt]         = colors.TableRowBgAlt;
        imgui_colors[ImGuiCol_TextSelectedBg]        = colors.TextSelectedBg;
        imgui_colors[ImGuiCol_DragDropTarget]        = colors.DragDropTarget;
        imgui_colors[ImGuiCol_NavHighlight]          = colors.NavHighlight;
        imgui_colors[ImGuiCol_NavWindowingHighlight] = colors.NavWindowingHighlight;
        imgui_colors[ImGuiCol_NavWindowingDimBg]     = colors.NavWindowingDimBg;
        imgui_colors[ImGuiCol_ModalWindowDimBg]      = colors.ModalWindowDimBg;
    }
}