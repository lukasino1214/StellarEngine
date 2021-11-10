#include "../Vendor/imgui/imgui.h"
#include "../Vendor/imgui/imgui_impl_glfw.h"
#include "../Vendor/imgui/imgui_impl_opengl3.h"
#include "../Engine/Engine.h"

#include "Panels/SceneHierarchyPanel.h"

//#include <stdio.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

using namespace Engine;

/*static constexpr int width = 1280;
static constexpr int height = 720;

int main() {
    Window m_Window(width, height, "Stella Engine");
    Device device{m_Window};
    VkPipelineLayout pipelineLayout;
    Pipeline m_Pipeline{device, "assets/shaders/simple_shader.vert.spv", "assets/shaders/simple_shader.frag.spv", Pipeline::defaultPipelineConfigInfo(width, height)};

    while(!m_Window.shouldClose()) {
        glfwPollEvents();
    }

    return 0;
}*/

#include "app.h"

#include <cstdlib>
#include <iostream>
#include <stdexcept>

int main() {
    FirstApp app{};

    try {
        app.run();
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}