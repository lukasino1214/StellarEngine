#pragma once

#include <vulkan/vulkan.h>

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>

#include <string>

namespace Engine {
    class Window {
    public:
        Window(const int &_width, const int &_height, const std::string &_name);
        ~Window();

        Window(const Window &) = delete;
        Window &operator=(const Window &) = delete;

        bool should_close() { return glfwWindowShouldClose(window); }
        VkExtent2D get_extent() { return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)}; }
        bool was_window_resized() { return framebuffer_resized; }
        void reset_window_resized_flag() { framebuffer_resized = false; }
        GLFWwindow *get_GLFWwindow() const { return window; }
        void create_window_surface(VkInstance instance, VkSurfaceKHR *surface);

    private:
        static void framebuffer_resize_callback(GLFWwindow *window, int width, int height);

        int width;
        int height;
        bool framebuffer_resized = false;
        std::string name;
        GLFWwindow *window;
    };
}
