#pragma once

#include "device.h"

#include "../pgepch.h"

namespace Engine {
    class SwapChain {
    public:
        static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

        SwapChain(std::shared_ptr<Device> _device, VkExtent2D windowExtent);
        SwapChain(std::shared_ptr<Device> _device, VkExtent2D extent, std::shared_ptr<SwapChain> previous);
        ~SwapChain();

        SwapChain(const SwapChain &) = delete;
        SwapChain &operator=(const SwapChain &) = delete;

        VkFramebuffer get_framebuffer(u32 index) { return vk_swapchain_framebuffers[index]; }
        VkImageView get_image_view(u32 index) { return vk_swapchain_image_views[index]; }
        u32 get_image_count() { return static_cast<u32>(vk_swapchain_images.size()); }
        VkFormat get_swapchain_image_format() { return vk_swapchain_image_format; }
        VkExtent2D get_swapchain_extent() { return vk_swapchain_extent; }
        [[nodiscard]] u32 width() const { return vk_swapchain_extent.width; }
        [[nodiscard]] u32 height() const { return vk_swapchain_extent.height; }

        [[nodiscard]] float get_extent_aspect_ratio() const {
            return static_cast<float>(vk_swapchain_extent.width) / static_cast<float>(vk_swapchain_extent.height);
        }
        VkFormat find_depth_format();

        VkResult acquire_next_image(uint32_t *image_index);
        VkResult submit_command_buffers(const VkCommandBuffer *buffers, const uint32_t *image_index);

        [[nodiscard]] bool compare_swap_formats(const SwapChain &swapchain) const {
            return swapchain.vk_swapchain_depth_format == vk_swapchain_depth_format && swapchain.vk_swapchain_image_format == vk_swapchain_image_format;
        }

        VkRenderPass vk_renderpass = {};
    private:
        void init();
        void create_swapchain();
        void create_image_views();
        void create_depth_resources();
        void create_renderpass();
        void create_framebuffers();
        void create_sync_objects();

        static VkSurfaceFormatKHR choose_swap_surface_format(const std::vector<VkSurfaceFormatKHR> &available_formats);
        static VkPresentModeKHR choose_swap_present_mode(const std::vector<VkPresentModeKHR> &available_present_modes);
        VkExtent2D choose_swap_extent(const VkSurfaceCapabilitiesKHR &capabilities);

        VkFormat vk_swapchain_image_format = {};
        VkFormat vk_swapchain_depth_format = {};
        VkExtent2D vk_swapchain_extent = {};

        std::vector<VkFramebuffer> vk_swapchain_framebuffers = {};

        std::vector<VkImage> vk_depth_images = {};
        std::vector<VkDeviceMemory> vk_depth_image_device_memories = {};
        std::vector<VkImageView> vk_depth_image_views = {};
        std::vector<VkImage> vk_swapchain_images = {};
        std::vector<VkImageView> vk_swapchain_image_views = {};

        VkExtent2D vk_window_extent;

        VkSwapchainKHR vk_swapchain = {};
        std::shared_ptr<SwapChain> old_swapchain;

        std::vector<VkSemaphore> vk_image_available_semaphores = {};
        std::vector<VkSemaphore> vk_render_finished_semaphores = {};
        std::vector<VkFence> vk_in_flight_fences = {};
        std::vector<VkFence> vk_images_in_flight = {};
        size_t current_frame = 0;
        std::shared_ptr<Device> device;
    };

}