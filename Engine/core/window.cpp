#include "window.h"
#include "../graphics/core.h"

#include <stdexcept>
#include <memory>

namespace Engine {
    Window::Window(const int &_width, const int &_height, const std::string &_name) : width{_width}, height{_height}, name{_name} {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        window = glfwCreateWindow(width, height, name.c_str(), nullptr, nullptr);
        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, framebuffer_resize_callback);
    }

    Window::~Window() {
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    void Window::create_window_surface(VkInstance instance, VkSurfaceKHR *surface) {
        if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS) {
            throw std::runtime_error("failed to craete window surface");
        }
    }

    void Window::framebuffer_resize_callback(GLFWwindow *_window, int width, int height) {
        auto window = reinterpret_cast<Window *>(glfwGetWindowUserPointer(_window));
        window->framebuffer_resized = true;
        window->width = width;
        window->height = height;
    }

}