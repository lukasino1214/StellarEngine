#pragma once

#include "device.h"
#include "../core/window.h"

// libs
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

#include "../pgepch.h"

namespace Engine {

    static void check_vk_result(VkResult err) {
        if (err == 0) return;
        fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
        if (err < 0) abort();
    }

    class ImGuiLayer {
    public:
        ImGuiLayer(std::shared_ptr<Device> _device, Window &window, VkRenderPass renderpass, uint32_t image_count);
        ~ImGuiLayer();

        void new_frame();
        void render(VkCommandBuffer command_buffer);

        VkDescriptorPool get_descriptor_pool() { return vk_descriptor_pool; }

    private:
        VkDescriptorPool vk_descriptor_pool;
        std::shared_ptr<Device> device;
    };
}