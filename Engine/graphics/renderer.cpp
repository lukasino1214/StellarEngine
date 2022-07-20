#include "renderer.h"
#include "core.h"

namespace Engine {

    Renderer::Renderer(std::shared_ptr<Window> _window, std::shared_ptr<Device> _device) : window{_window}, device{_device} {
        recreate_swapchain();
        create_command_buffers();
    }

    Renderer::~Renderer() { free_command_buffers(); }

    void Renderer::recreate_swapchain() {
        VkExtent2D extent = window->get_extent();
        while (extent.width == 0 || extent.height == 0) {
            extent = window->get_extent();
            glfwWaitEvents();
        }
        vkDeviceWaitIdle(device->vk_device);

        if (swapchain == nullptr) {
            swapchain = std::make_unique<SwapChain>(device, extent);
        } else {
            std::shared_ptr<SwapChain> old_swapchain = std::move(swapchain);
            swapchain = std::make_unique<SwapChain>(device, extent, old_swapchain);

            if (!old_swapchain->compare_swap_formats(*swapchain.get())) {
                throw std::runtime_error("Swap chain image(or depth) format has changed!");
            }
        }
    }

    void Renderer::create_command_buffers() {
        command_buffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);

        VkCommandBufferAllocateInfo vk_command_buffer_allocate_info = {
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                .pNext = nullptr,
                .commandPool = device->vk_command_pool,
                .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                .commandBufferCount = static_cast<uint32_t>(command_buffers.size())
        };

        if (vkAllocateCommandBuffers(device->vk_device, &vk_command_buffer_allocate_info, command_buffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffers!");
        }
    }

    void Renderer::free_command_buffers() {
        vkFreeCommandBuffers(device->vk_device, device->vk_command_pool, static_cast<uint32_t>(command_buffers.size()), command_buffers.data());
        command_buffers.clear();
    }

    VkCommandBuffer Renderer::begin_frame() {
        assert(!is_frame_started && "Can't call beginFrame while already in progress");

        VkResult result = swapchain->acquire_next_image(&current_image_index);
        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            recreate_swapchain();
            return nullptr;
        }

        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        is_frame_started = true;

        VkCommandBuffer command_buffer = get_current_command_buffer();
        VkCommandBufferBeginInfo vk_command_buffer_begin_info = {
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                .pNext = nullptr,
                .flags = 0,
                .pInheritanceInfo = nullptr
        };

        if (vkBeginCommandBuffer(command_buffer, &vk_command_buffer_begin_info) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }
        return command_buffer;
    }

    void Renderer::end_frame() {
        assert(is_frame_started && "Can't call endFrame while frame is not in progress");
        VkCommandBuffer command_buffer = get_current_command_buffer();
        if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }

        VkResult result = swapchain->submit_command_buffers(&command_buffer, &current_image_index);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
            window->was_window_resized()) {
            window->reset_window_resized_flag();
            recreate_swapchain();
        } else if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to present swap chain image!");
        }

        is_frame_started = false;
        current_frame_index = (current_frame_index + 1) % SwapChain::MAX_FRAMES_IN_FLIGHT;
    }

    void Renderer::begin_swapchain_renderpass(VkCommandBuffer command_buffer) {
        assert(is_frame_started && "Can't call beginSwapChainRenderPass if frame is not in progress");
        assert(command_buffer == get_current_command_buffer() && "Can't begin render pass on command buffer from a different frame");

        std::vector<VkClearValue> clear_values = {
                { .color = { 0.01f, 0.01f, 0.01f, 1.0f } },
                { .depthStencil = { 1.0f, 0 } }
        };

        VkRenderPassBeginInfo vk_renderpass_begin_info = {
                .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
                .pNext = nullptr,
                .renderPass = swapchain->vk_renderpass,
                .framebuffer = swapchain->get_framebuffer(current_image_index),
                .renderArea = {
                        .offset = { 0, 0 },
                        .extent = swapchain->get_swapchain_extent()
                },
                .clearValueCount = static_cast<uint32_t>(clear_values.size()),
                .pClearValues = clear_values.data()
        };

        vkCmdBeginRenderPass(command_buffer, &vk_renderpass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport = {
                .x = 0.0f,
                .y = 0.0f,
                .width = static_cast<float>(swapchain->get_swapchain_extent().width),
                .height = static_cast<float>(swapchain->get_swapchain_extent().height),
                .minDepth = 0.0f,
                .maxDepth = 1.0f
        };

        vkCmdSetViewport(command_buffer, 0, 1, &viewport);

        VkRect2D scissor = {
            .offset = { 0, 0 },
            .extent = swapchain->get_swapchain_extent()
        };

        vkCmdSetScissor(command_buffer, 0, 1, &scissor);
    }

    void Renderer::end_swapchain_renderpass(VkCommandBuffer command_buffer) {
        assert(is_frame_started && "Can't call endSwapChainRenderPass if frame is not in progress");
        assert(command_buffer == get_current_command_buffer() && "Can't end render pass on command buffer from a different frame");
        vkCmdEndRenderPass(command_buffer);
    }

}