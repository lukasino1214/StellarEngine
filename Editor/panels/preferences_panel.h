#pragma once

#include <imgui.h>

namespace Engine {
    struct ImGuiColors {
        ImVec4 Text;
        ImVec4 TextDisabled;
        ImVec4 WindowBg;
        ImVec4 ChildBg;
        ImVec4 PopupBg;
        ImVec4 Border;
        ImVec4 BorderShadow;
        ImVec4 FrameBg;
        ImVec4 FrameBgHovered;
        ImVec4 FrameBgActive;
        ImVec4 TitleBg;
        ImVec4 TitleBgActive;
        ImVec4 TitleBgCollapsed;
        ImVec4 MenuBarBg;
        ImVec4 ScrollbarBg;
        ImVec4 ScrollbarGrab;
        ImVec4 ScrollbarGrabHovered;
        ImVec4 ScrollbarGrabActive;
        ImVec4 CheckMark;
        ImVec4 SliderGrab;
        ImVec4 SliderGrabActive;
        ImVec4 Button;
        ImVec4 ButtonHovered;
        ImVec4 ButtonActive;
        ImVec4 Header;
        ImVec4 HeaderHovered;
        ImVec4 HeaderActive;
        ImVec4 Separator;
        ImVec4 SeparatorHovered;
        ImVec4 SeparatorActive;
        ImVec4 ResizeGrip;
        ImVec4 ResizeGripHovered;
        ImVec4 ResizeGripActive;
        ImVec4 Tab;
        ImVec4 TabHovered;
        ImVec4 TabActive;
        ImVec4 TabUnfocused;
        ImVec4 TabUnfocusedActive;
        ImVec4 DockingPreview;
        ImVec4 DockingEmptyBg;
        ImVec4 PlotLines;
        ImVec4 PlotLinesHovered;
        ImVec4 PlotHistogram;
        ImVec4 PlotHistogramHovered;
        ImVec4 TableHeaderBg;
        ImVec4 TableBorderStrong;
        ImVec4 TableBorderLight;
        ImVec4 TableRowBg;
        ImVec4 TableRowBgAlt;
        ImVec4 TextSelectedBg;
        ImVec4 DragDropTarget;
        ImVec4 NavHighlight;
        ImVec4 NavWindowingHighlight;
        ImVec4 NavWindowingDimBg;
        ImVec4 ModalWindowDimBg;
    };

    struct ImGuiStyle {
        float       Alpha;
        float       DisabledAlpha;
        ImVec2      WindowPadding;
        float       WindowRounding;
        float       WindowBorderSize;
        ImVec2      WindowMinSize;
        ImVec2      WindowTitleAlign;
        ImGuiDir    WindowMenuButtonPosition;
        float       ChildRounding;
        float       ChildBorderSize;
        float       PopupRounding;
        float       PopupBorderSize;
        ImVec2      FramePadding;
        float       FrameRounding;
        float       FrameBorderSize;
        ImVec2      ItemSpacing;
        ImVec2      ItemInnerSpacing;
        ImVec2      CellPadding;
        ImVec2      TouchExtraPadding;
        float       IndentSpacing;
        float       ColumnsMinSpacing;
        float       ScrollbarSize;
        float       ScrollbarRounding;
        float       GrabMinSize;
        float       GrabRounding;
        float       LogSliderDeadzone;
        float       TabRounding;
        float       TabBorderSize;
        float       TabMinWidthForCloseButton;
        ImGuiDir    ColorButtonPosition;
        ImVec2      ButtonTextAlign;
        ImVec2      SelectableTextAlign;
        ImVec2      DisplayWindowPadding;
        ImVec2      DisplaySafeAreaPadding;
        float       MouseCursorScale;
        bool        AntiAliasedLines;
        bool        AntiAliasedLinesUseTex;
        bool        AntiAliasedFill;
        float       CurveTessellationTol;
        float       CircleTessellationMaxError;
        ImGuiColors Color;
    };

    class PreferencesPanel {
    public:
        PreferencesPanel();

        void render();

    private:
        void set_style();
        void set_default_colors();
        void set_engine_default_colors();
        void read_config_file();
        void save_config_file();
        void apply_style();

        ImGuiStyle style {};
    };
}
