#include "imgui_layer.h"

#include "device.h"

// libs
#include "core.h"

namespace Engine {

    ImGuiLayer::ImGuiLayer(std::shared_ptr<Device> _device, Window &window, VkRenderPass renderpass, uint32_t image_count) : device{_device} {
        VkDescriptorPoolSize pool_sizes[] = {
                {VK_DESCRIPTOR_TYPE_SAMPLER,                2000},
                {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2000},
                {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,          2000},
                {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,          2000},
                {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,   2000},
                {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,   2000},
                {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         2000},
                {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,         2000},
                {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 2000},
                {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 2000},
                {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,       2000}
        };

        VkDescriptorPoolCreateInfo vk_descriptor_pool_create_info = {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
                .pNext = nullptr,
                .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
                .maxSets = 2000 * IM_ARRAYSIZE(pool_sizes),
                .poolSizeCount = IM_ARRAYSIZE(pool_sizes),
                .pPoolSizes = pool_sizes
        };

        if (vkCreateDescriptorPool(device->vk_device, &vk_descriptor_pool_create_info, nullptr, &vk_descriptor_pool) != VK_SUCCESS) {
            throw std::runtime_error("failed to set up descriptor pool for imgui");
        }

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        (void) io;
        // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        //io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

        //io.Fonts->AddFontFromFileTTF("assets/UbuntuMono-Regular.ttf", 14);
        io.Fonts->AddFontFromFileTTF("assets/Roboto-Regular.ttf", 15);

        ImGui::StyleColorsDark();

        auto &colors = ImGui::GetStyle().Colors;

        /*
         * constexpr auto colorFromBytes = [](const uint8_t r, const uint8_t g, const uint8_t b) { return ImVec4(static_cast<float>(r) / 255.0f, static_cast<float>(g) / 255.0f, static_cast<float>(b) / 255.0f, 1.0f); };
        */

        colors[ImGuiCol_WindowBg] = ImVec4{0.1f, 0.105f, 0.11f, 1.0f};

        // Headers
        colors[ImGuiCol_Header] = ImVec4{0.2f, 0.205f, 0.21f, 1.0f};
        colors[ImGuiCol_HeaderHovered] = ImVec4{0.3f, 0.305f, 0.31f, 1.0f};
        colors[ImGuiCol_HeaderActive] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};

        // Buttons
        colors[ImGuiCol_Button] = ImVec4{0.2f, 0.205f, 0.21f, 1.0f};
        colors[ImGuiCol_ButtonHovered] = ImVec4{0.3f, 0.305f, 0.31f, 1.0f};
        colors[ImGuiCol_ButtonActive] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};

        // Frame BG
        colors[ImGuiCol_FrameBg] = ImVec4{0.2f, 0.205f, 0.21f, 1.0f};
        colors[ImGuiCol_FrameBgHovered] = ImVec4{0.3f, 0.305f, 0.31f, 1.0f};
        colors[ImGuiCol_FrameBgActive] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};

        // Tabs
        colors[ImGuiCol_Tab] = ImVec4{0.20f, 0.2005f, 0.201f, 1.0f};
        colors[ImGuiCol_TabHovered] = ImVec4{0.38f, 0.3805f, 0.381f, 1.0f};
        colors[ImGuiCol_TabActive] = ImVec4{0.28f, 0.2805f, 0.281f, 1.0f};
        colors[ImGuiCol_TabUnfocused] = ImVec4{0.20f, 0.2005f, 0.201f, 1.0f};
        colors[ImGuiCol_TabUnfocusedActive] = ImVec4{0.2f, 0.205f, 0.21f, 1.0f};

        // Title
        colors[ImGuiCol_TitleBg] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};
        colors[ImGuiCol_TitleBgActive] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};

        ImGui_ImplGlfw_InitForVulkan(window.get_GLFWwindow(), true);
        ImGui_ImplVulkan_InitInfo init_info = {
                .Instance = device->vk_instance,
                .PhysicalDevice = device->vk_physical_device,
                .Device = device->vk_device,
                .QueueFamily = device->get_graphics_queue_family(),
                .Queue = device->vk_graphics_queue,
                .PipelineCache = {},
                .DescriptorPool = vk_descriptor_pool,
                .Subpass = 0,
                .MinImageCount = 2,
                .ImageCount = image_count,
                .MSAASamples = {},
                .Allocator = nullptr,
                .CheckVkResultFn = check_vk_result
        };

        ImGui_ImplVulkan_Init(&init_info, renderpass);

        auto command_buffer = device->begin_single_time_command_buffer();
        ImGui_ImplVulkan_CreateFontsTexture(command_buffer);
        device->end_single_time_command_buffer(command_buffer);
        ImGui_ImplVulkan_DestroyFontUploadObjects();
    }

    ImGuiLayer::~ImGuiLayer() {
        vkDestroyDescriptorPool(device->vk_device, vk_descriptor_pool, nullptr);
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    void ImGuiLayer::new_frame() {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void ImGuiLayer::render(VkCommandBuffer command_buffer) {
        ImGui::Render();
        ImDrawData *drawdata = ImGui::GetDrawData();
        ImGui_ImplVulkan_RenderDrawData(drawdata, command_buffer);
    }
}