add_library(IMGUI STATIC)

find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(IMGUI PRIVATE glfw)

target_sources(IMGUI
        PRIVATE
        imgui_demo.cpp
        imgui_draw.cpp
        imgui_tables.cpp
        imgui_widgets.cpp
        imgui.cpp

        PRIVATE
        backends/imgui_impl_vulkan.cpp
        backends/imgui_impl_glfw.cpp
        )

target_include_directories( IMGUI
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../glfw/include/
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/
        PUBLIC backends
        )