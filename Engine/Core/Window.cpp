//
// Created by lukas on 07.11.21.
//

#include "Window.h"
#include "../Graphics/Core.h"

#include <stdexcept>
#include <memory>

namespace Engine {
    Window::Window(const int& width, const int& height, const std::string& name) : m_Width{width}, m_Height{height}, m_Name{name} {
        initWindow();
        Core::m_Device = std::make_shared<Device>(this);
    }

    Window::~Window() {
        glfwDestroyWindow(m_Window);
        glfwTerminate();
    }

    void Window::initWindow() {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        m_Window = glfwCreateWindow(m_Width, m_Height, m_Name.c_str(), nullptr, nullptr);
        glfwSetWindowUserPointer(m_Window, this);
        glfwSetFramebufferSizeCallback(m_Window, framebufferResizeCallback);
    }

    void Window::createWindowSurface(VkInstance instance, VkSurfaceKHR *surface) {
        if (glfwCreateWindowSurface(instance, m_Window, nullptr, surface) != VK_SUCCESS) {
            throw std::runtime_error("failed to craete window surface");
        }
    }

    void Window::framebufferResizeCallback(GLFWwindow *window, int width, int height) {
        auto lveWindow = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
        lveWindow->framebufferResized = true;
        lveWindow->m_Width = width;
        lveWindow->m_Height = height;
    }
    
}