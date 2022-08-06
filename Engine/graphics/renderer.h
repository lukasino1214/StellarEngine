#pragma once

#include "device.h"
#include "swapchain.h"
#include "../core/window.h"

// std
#include "../pgepch.h"

namespace Engine {
    class Renderer {
    public:
        Renderer(std::shared_ptr<Window> _window, std::shared_ptr<Device> _device);
        ~Renderer();

        Renderer(const Renderer &) = delete;
        Renderer &operator=(const Renderer &) = delete;

        [[nodiscard]] VkRenderPass get_swapchain_renderpass() const { return swapchain->vk_renderpass; }
        [[nodiscard]] u32 get_image_count() const { return swapchain->get_image_count(); }

        [[maybe_unused]] [[nodiscard]] f32 get_aspect_ratio() const { return swapchain->get_extent_aspect_ratio(); }

        [[maybe_unused]] [[nodiscard]] bool is_frame_in_progress() const { return is_frame_started; }

        [[maybe_unused]] VkImageView get_image_view(u32 index) { return swapchain->get_image_view(index); }

        [[nodiscard]] VkCommandBuffer get_current_command_buffer() const {
            assert(is_frame_started && "Cannot get command buffer when frame not in progress");
            return command_buffers[current_frame_index];
        }

        [[nodiscard]] u32 get_frame_index() const {
            assert(is_frame_started && "Cannot get frame index when frame not in progress");
            return current_frame_index;
        }

        VkCommandBuffer begin_frame();
        void end_frame();

        void begin_swapchain_renderpass(VkCommandBuffer command_buffer);
        void end_swapchain_renderpass(VkCommandBuffer command_buffer) const;

    private:
        void create_command_buffers();
        void free_command_buffers();
        void recreate_swapchain();

        std::shared_ptr<Window> window;
        std::shared_ptr<Device> device;
        std::unique_ptr<SwapChain> swapchain;
        std::vector<VkCommandBuffer> command_buffers;

        u32 current_image_index = 0;
        u32 current_frame_index = 0;
        bool is_frame_started = false;
    };
}