//
// Created by lukas on 15.11.21.
//

#include "imguiLayer.h"

#include "device.h"
#include "../Core/Window.h"

// libs
#include "../../Vendor/imgui/imgui.h"
#include "../../Vendor/imgui/imgui_impl_glfw.h"
#include "../../Vendor/imgui/imgui_impl_vulkan.h"

// std
#include <stdexcept>

namespace Engine {

// ok this just initializes imgui using the provided integration files. So in our case we need to
// initialize the vulkan and glfw imgui implementations, since that's what our engine is built
// using.
    Imgui::Imgui(
            Window &window, Device &device, VkRenderPass renderPass, uint32_t imageCount) : m_Device{device} {
        // set up a descriptor pool stored on this instance, see header for more comments on this.
        VkDescriptorPoolSize pool_sizes[] = {
                {VK_DESCRIPTOR_TYPE_SAMPLER, 2000},
                {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2000},
                {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 2000},
                {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 2000},
                {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 2000},
                {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 2000},
                {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2000},
                {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2000},
                {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 2000},
                {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 2000},
                {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 2000}};
        VkDescriptorPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        pool_info.maxSets = 2000 * IM_ARRAYSIZE(pool_sizes);
        pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
        pool_info.pPoolSizes = pool_sizes;
        if (vkCreateDescriptorPool(device.device(), &pool_info, nullptr, &m_DescriptorPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to set up descriptor pool for imgui");
        }

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        (void)io;
        // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        //io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();

        auto& colors = ImGui::GetStyle().Colors;
        colors[ImGuiCol_WindowBg] = ImVec4{ 0.1f, 0.105f, 0.11f, 1.0f };

        // Headers
        colors[ImGuiCol_Header] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
        colors[ImGuiCol_HeaderHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
        colors[ImGuiCol_HeaderActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

        // Buttons
        colors[ImGuiCol_Button] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
        colors[ImGuiCol_ButtonHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
        colors[ImGuiCol_ButtonActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

        // Frame BG
        colors[ImGuiCol_FrameBg] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
        colors[ImGuiCol_FrameBgHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
        colors[ImGuiCol_FrameBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

        // Tabs
        colors[ImGuiCol_Tab] = ImVec4{ 0.20f, 0.2005f, 0.201f, 1.0f };
        colors[ImGuiCol_TabHovered] = ImVec4{ 0.38f, 0.3805f, 0.381f, 1.0f };
        colors[ImGuiCol_TabActive] = ImVec4{ 0.28f, 0.2805f, 0.281f, 1.0f };
        colors[ImGuiCol_TabUnfocused] = ImVec4{ 0.20f, 0.2005f, 0.201f, 1.0f };
        colors[ImGuiCol_TabUnfocusedActive] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };

        // Title
        colors[ImGuiCol_TitleBg] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
        colors[ImGuiCol_TitleBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

        //ImGui::StyleColorsClassic();

        // Setup Platform/Renderer backends
        // Initialize imgui for vulkan
        ImGui_ImplGlfw_InitForVulkan(window.getGLFWwindow(), true);
        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance = device.getInstance();
        init_info.PhysicalDevice = device.getPhysicalDevice();
        init_info.Device = device.device();
        init_info.QueueFamily = device.getGraphicsQueueFamily();
        init_info.Queue = device.graphicsQueue();

        // pipeline cache is a potential future optimization, ignoring for now
        init_info.PipelineCache = VK_NULL_HANDLE;
        init_info.DescriptorPool = m_DescriptorPool;
        // todo, I should probably get around to integrating a memory allocator library such as Vulkan
        // memory allocator (VMA) sooner than later. We don't want to have to update adding an allocator
        // in a ton of locations.
        init_info.Allocator = VK_NULL_HANDLE;
        init_info.MinImageCount = 2;
        init_info.ImageCount = imageCount;
        init_info.CheckVkResultFn = check_vk_result;
        ImGui_ImplVulkan_Init(&init_info, renderPass);

        // upload fonts, this is done by recording and submitting a one time use command buffer
        // which can be done easily bye using some existing helper functions on the lve device object
        auto commandBuffer = device.beginSingleTimeCommands();
        ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
        device.endSingleTimeCommands(commandBuffer);
        ImGui_ImplVulkan_DestroyFontUploadObjects();
    }

    Imgui::~Imgui() {
        vkDestroyDescriptorPool(m_Device.device(), m_DescriptorPool, nullptr);
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    void Imgui::newFrame() {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

// this tells imgui that we're done setting up the current frame,
// then gets the draw data from imgui and uses it to record to the provided
// command buffer the necessary draw commands
    void Imgui::render(VkCommandBuffer commandBuffer) {
        ImGui::Render();
        ImDrawData *drawdata = ImGui::GetDrawData();
        ImGui_ImplVulkan_RenderDrawData(drawdata, commandBuffer);
    }
}