//
// Created by lukas on 07.11.21.
//

#ifndef ENGINE_WINDOW_H
#define ENGINE_WINDOW_H

#include <vulkan/vulkan.h>

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>

#include <string>

namespace Engine {
    class Window {
    public:
        Window(const int &width, const int &height, const std::string &name);

        ~Window();

        Window(const Window &) = delete;

        Window &operator=(const Window &) = delete;

        bool shouldClose() { return glfwWindowShouldClose(m_Window); }

        VkExtent2D getExtent() { return {static_cast<uint32_t>(m_Width), static_cast<uint32_t>(m_Height)}; }

        bool wasWindowResized() { return framebufferResized; }

        void resetWindowResizedFlag() { framebufferResized = false; }

        GLFWwindow *getGLFWwindow() const { return m_Window; }

        void createWindowSurface(VkInstance instance, VkSurfaceKHR *surface);

    private:
        static void framebufferResizeCallback(GLFWwindow *window, int width, int height);

        void initWindow();

        int m_Width;
        int m_Height;
        bool framebufferResized = false;
        std::string m_Name;
        GLFWwindow *m_Window;
    };
}


#endif //ENGINE_WINDOW_H
