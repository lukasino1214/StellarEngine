#include "swapchain.h"

#include <array>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <utility>

namespace Engine {
    SwapChain::SwapChain(std::shared_ptr<Device> _device, VkExtent2D extent) : vk_window_extent{extent}, device{std::move(_device)} {
        init();
    }

    SwapChain::SwapChain(std::shared_ptr<Device> _device, VkExtent2D extent, std::shared_ptr<SwapChain> previous) : vk_window_extent{extent}, old_swapchain{std::move(previous)}, device{std::move(_device)} {
        init();
        old_swapchain = nullptr;
    }

    void SwapChain::init() {
        create_swapchain();
        create_image_views();
        create_renderpass();
        create_depth_resources();
        create_framebuffers();
        create_sync_objects();
    }

    SwapChain::~SwapChain() {
        for (auto vk_image_view : vk_swapchain_image_views) {
            vkDestroyImageView(device->vk_device, vk_image_view, nullptr);
        }
        vk_swapchain_image_views.clear();

        if (vk_swapchain != nullptr) {
            vkDestroySwapchainKHR(device->vk_device, vk_swapchain, nullptr);
            vk_swapchain = nullptr;
        }

        for (int i = 0; i < vk_depth_images.size(); i++) {
            vkDestroyImageView(device->vk_device, vk_depth_image_views[i], nullptr);
            /*vkDestroyImage(device->vk_device, vk_depth_images[i], nullptr);
            vkFreeMemory(device->vk_device, vk_depth_image_device_memories[i], nullptr);*/
            vmaDestroyImage(device->vma_allocator, vk_depth_images[i], vma_depth_image_allocation[i]);
        }

        for (auto vk_framebuffer : vk_swapchain_framebuffers) {
            vkDestroyFramebuffer(device->vk_device, vk_framebuffer, nullptr);
        }

        vkDestroyRenderPass(device->vk_device, vk_renderpass, nullptr);

        // cleanup synchronization objects
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroySemaphore(device->vk_device, vk_render_finished_semaphores[i], nullptr);
            vkDestroySemaphore(device->vk_device, vk_image_available_semaphores[i], nullptr);
            vkDestroyFence(device->vk_device, vk_in_flight_fences[i], nullptr);
        }
    }

    VkResult SwapChain::acquire_next_image(uint32_t *image_index) {
        vkWaitForFences(device->vk_device, 1, &vk_in_flight_fences[current_frame], VK_TRUE, std::numeric_limits<uint64_t>::max());

        VkResult result = vkAcquireNextImageKHR(device->vk_device, vk_swapchain, std::numeric_limits<uint64_t>::max(), vk_image_available_semaphores[current_frame], VK_NULL_HANDLE, image_index);
        return result;
    }

    VkResult SwapChain::submit_command_buffers(const VkCommandBuffer *buffers, const uint32_t *image_index) {
        if (vk_images_in_flight[*image_index] != VK_NULL_HANDLE) {
            vkWaitForFences(device->vk_device, 1, &vk_images_in_flight[*image_index], VK_TRUE, UINT64_MAX);
        }
        vk_images_in_flight[*image_index] = vk_images_in_flight[current_frame];

        VkSemaphore wait_semaphores[] = {vk_image_available_semaphores[current_frame]};
        VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        VkSemaphore signal_semaphores[] = {vk_render_finished_semaphores[current_frame]};

        VkSubmitInfo vk_submit_info = {
                .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                .pNext = nullptr,
                .waitSemaphoreCount = 1,
                .pWaitSemaphores = wait_semaphores,
                .pWaitDstStageMask = wait_stages,
                .commandBufferCount = 1,
                .pCommandBuffers = buffers,
                .signalSemaphoreCount = 1,
                .pSignalSemaphores = signal_semaphores
        };

        vkResetFences(device->vk_device, 1, &vk_in_flight_fences[current_frame]);
        if (vkQueueSubmit(device->vk_graphics_queue, 1, &vk_submit_info, vk_in_flight_fences[current_frame]) != VK_SUCCESS) {
            throw std::runtime_error("failed to submit draw command buffer!");
        }

        VkSwapchainKHR vk_swapchains[] = {vk_swapchain};

        VkPresentInfoKHR vk_present_info_khr = {
                .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
                .pNext = nullptr,
                .waitSemaphoreCount = 1,
                .pWaitSemaphores = signal_semaphores,
                .swapchainCount = 1,
                .pSwapchains = vk_swapchains,
                .pImageIndices = image_index,
                .pResults = nullptr
        };

        current_frame = (current_frame + 1) % MAX_FRAMES_IN_FLIGHT;

        auto result = vkQueuePresentKHR(device->vk_present_queue, &vk_present_info_khr);
        return result;
    }

    void SwapChain::create_swapchain() {
        SwapChainSupportDetails swapchain_support = device->get_swapchain_support();

        VkSurfaceFormatKHR surface_format = choose_swap_surface_format(swapchain_support.formats);
        VkPresentModeKHR present_mode = choose_swap_present_mode(swapchain_support.present_modes);
        VkExtent2D extent = choose_swap_extent(swapchain_support.capabilities);

        uint32_t image_count = swapchain_support.capabilities.minImageCount + 1;
        if (swapchain_support.capabilities.maxImageCount > 0 && image_count > swapchain_support.capabilities.maxImageCount) {
            image_count = swapchain_support.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR vk_swapchain_create_info_khr = {};
        vk_swapchain_create_info_khr.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        vk_swapchain_create_info_khr.surface = device->vk_surface_khr;

        vk_swapchain_create_info_khr.minImageCount = image_count;
        vk_swapchain_create_info_khr.imageFormat = surface_format.format;
        vk_swapchain_create_info_khr.imageColorSpace = surface_format.colorSpace;
        vk_swapchain_create_info_khr.imageExtent = extent;
        vk_swapchain_create_info_khr.imageArrayLayers = 1;
        vk_swapchain_create_info_khr.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        QueueFamilyIndices indices = device->find_physical_queue_families();
        uint32_t queue_family_indices[] = {indices.graphics_family, indices.present_family};

        if (indices.graphics_family != indices.present_family) {
            vk_swapchain_create_info_khr.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            vk_swapchain_create_info_khr.queueFamilyIndexCount = 2;
            vk_swapchain_create_info_khr.pQueueFamilyIndices = queue_family_indices;
        } else {
            vk_swapchain_create_info_khr.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            vk_swapchain_create_info_khr.queueFamilyIndexCount = 0;
            vk_swapchain_create_info_khr.pQueueFamilyIndices = nullptr;
        }

        vk_swapchain_create_info_khr.preTransform = swapchain_support.capabilities.currentTransform;
        vk_swapchain_create_info_khr.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

        vk_swapchain_create_info_khr.presentMode = present_mode;
        vk_swapchain_create_info_khr.clipped = VK_TRUE;

        vk_swapchain_create_info_khr.oldSwapchain = old_swapchain == nullptr ? VK_NULL_HANDLE : old_swapchain->vk_swapchain;

        if (vkCreateSwapchainKHR(device->vk_device, &vk_swapchain_create_info_khr, nullptr, &vk_swapchain) != VK_SUCCESS) {
            throw std::runtime_error("failed to create swap chain!");
        }

        vkGetSwapchainImagesKHR(device->vk_device, vk_swapchain, &image_count, nullptr);
        vk_swapchain_images.resize(image_count);
        vkGetSwapchainImagesKHR(device->vk_device, vk_swapchain, &image_count, vk_swapchain_images.data());

        vk_swapchain_image_format = surface_format.format;
        vk_swapchain_extent = extent;
    }

    void SwapChain::create_image_views() {
        vk_swapchain_image_views.resize(vk_swapchain_images.size());
        for (size_t i = 0; i < vk_swapchain_image_views.size(); i++) {
            VkImageViewCreateInfo vk_image_view_create_info = {
                    .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                    .pNext = nullptr,
                    .flags = 0,
                    .image = vk_swapchain_images[i],
                    .viewType = VK_IMAGE_VIEW_TYPE_2D,
                    .format = vk_swapchain_image_format,
                    .components = {},
                    .subresourceRange = {
                            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                            .baseMipLevel = 0,
                            .levelCount = 1,
                            .baseArrayLayer = 0,
                            .layerCount = 1
                    }
            };

            if (vkCreateImageView(device->vk_device, &vk_image_view_create_info, nullptr, &vk_swapchain_image_views[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create texture image view!");
            }
        }
    }

    void SwapChain::create_renderpass() {
        VkAttachmentDescription vk_depth_attachment_description = {
                .flags = 0,
                .format = find_depth_format(),
                .samples = VK_SAMPLE_COUNT_1_BIT,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
        };

        VkAttachmentReference vk_depth_attachment_reference = {
                .attachment = 1,
                .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
        };

        VkAttachmentDescription vk_color_attachment_description = {
                .flags = 0,
                .format = get_swapchain_image_format(),
                .samples = VK_SAMPLE_COUNT_1_BIT,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
        };

        VkAttachmentReference vk_color_attachment_reference = {
                .attachment = 0,
                .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        };

        VkSubpassDescription vk_subpass_description = {
                .flags = 0,
                .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
                .inputAttachmentCount = 0,
                .pInputAttachments = nullptr,
                .colorAttachmentCount = 1,
                .pColorAttachments = &vk_color_attachment_reference,
                .pResolveAttachments = nullptr,
                .pDepthStencilAttachment = &vk_depth_attachment_reference,
                .preserveAttachmentCount = 0,
                .pPreserveAttachments = nullptr
        };

        VkSubpassDependency vk_subpass_dependency = {
                .srcSubpass = VK_SUBPASS_EXTERNAL,
                .dstSubpass = 0,
                .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
                .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
                .srcAccessMask = 0,
                .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                .dependencyFlags = {}
        };

        std::array<VkAttachmentDescription, 2> attachments = { vk_color_attachment_description, vk_depth_attachment_description };
        VkRenderPassCreateInfo vk_renderpass_create_info = {
                .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .attachmentCount = static_cast<uint32_t>(attachments.size()),
                .pAttachments = attachments.data(),
                .subpassCount = 1,
                .pSubpasses = &vk_subpass_description,
                .dependencyCount = 1,
                .pDependencies = &vk_subpass_dependency
        };

        if (vkCreateRenderPass(device->vk_device, &vk_renderpass_create_info, nullptr, &vk_renderpass) != VK_SUCCESS) {
            throw std::runtime_error("failed to create render pass!");
        }
    }

    void SwapChain::create_framebuffers() {
        vk_swapchain_framebuffers.resize(get_image_count());
        for (size_t i = 0; i < get_image_count(); i++) {
            std::array<VkImageView, 2> attachments = {vk_swapchain_image_views[i], vk_depth_image_views[i]};

            VkExtent2D swapchain_extent = get_swapchain_extent();
            VkFramebufferCreateInfo vk_framebuffer_create_info = {
                    .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                    .pNext = nullptr,
                    .flags = 0,
                    .renderPass = vk_renderpass,
                    .attachmentCount = static_cast<uint32_t>(attachments.size()),
                    .pAttachments = attachments.data(),
                    .width = swapchain_extent.width,
                    .height = swapchain_extent.height,
                    .layers = 1
            };

            if (vkCreateFramebuffer(device->vk_device, &vk_framebuffer_create_info, nullptr, &vk_swapchain_framebuffers[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create framebuffer!");
            }
        }
    }

    void SwapChain::create_depth_resources() {
        VkFormat vk_depth_format = find_depth_format();
        vk_swapchain_depth_format = vk_depth_format;
        VkExtent2D swapchain_extent = get_swapchain_extent();

        vk_depth_images.resize(get_image_count());
        vma_depth_image_allocation.resize(get_image_count());
        vk_depth_image_views.resize(get_image_count());

        for (int i = 0; i < vk_depth_images.size(); i++) {
            VkImageCreateInfo vk_image_create_info = {
                    .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                    .pNext = nullptr,
                    .flags = 0,
                    .imageType = VK_IMAGE_TYPE_2D,
                    .format = vk_depth_format,
                    .extent = {
                            .width = swapchain_extent.width,
                            .height = swapchain_extent.height,
                            .depth = 1
                    },
                    .mipLevels = 1,
                    .arrayLayers = 1,
                    .samples = VK_SAMPLE_COUNT_1_BIT,
                    .tiling = VK_IMAGE_TILING_OPTIMAL,
                    .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                    .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
                    .queueFamilyIndexCount = 0,
                    .pQueueFamilyIndices = nullptr,
                    .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
            };

            device->create_image_with_info(vk_image_create_info, MemoryFlagBits::DEDICATED_MEMORY, vk_depth_images[i], vma_depth_image_allocation[i]);

            VkImageViewCreateInfo vk_image_view_create_info = {
                    .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                    .pNext = nullptr,
                    .flags = 0,
                    .image = vk_depth_images[i],
                    .viewType = VK_IMAGE_VIEW_TYPE_2D,
                    .format = vk_depth_format,
                    .components = {},
                    .subresourceRange = {
                            .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
                            .baseMipLevel = 0,
                            .levelCount = 1,
                            .baseArrayLayer = 0,
                            .layerCount = 1
                    }
            };

            if (vkCreateImageView(device->vk_device, &vk_image_view_create_info, nullptr, &vk_depth_image_views[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create texture image view!");
            }
        }
    }

    void SwapChain::create_sync_objects() {
        vk_image_available_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
        vk_render_finished_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
        vk_in_flight_fences.resize(MAX_FRAMES_IN_FLIGHT);
        vk_images_in_flight.resize(get_image_count(), VK_NULL_HANDLE);

        VkSemaphoreCreateInfo vk_semaphore_create_info = {
                .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0
        };

        VkFenceCreateInfo vk_fence_create_info = {
                .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
                .pNext = nullptr,
                .flags = VK_FENCE_CREATE_SIGNALED_BIT,
        };

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            if (vkCreateSemaphore(device->vk_device, &vk_semaphore_create_info, nullptr, &vk_image_available_semaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(device->vk_device, &vk_semaphore_create_info, nullptr, &vk_render_finished_semaphores[i]) != VK_SUCCESS ||
                vkCreateFence(device->vk_device, &vk_fence_create_info, nullptr, &vk_in_flight_fences[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create synchronization objects for a frame!");
            }
        }
    }

    VkSurfaceFormatKHR SwapChain::choose_swap_surface_format(const std::vector<VkSurfaceFormatKHR> &available_formats) {
        for (const auto &available_format: available_formats) {
            if (available_format.format == VK_FORMAT_B8G8R8A8_UNORM) {
                return available_format;
            }
        }

        return available_formats[0];
    }

    VkPresentModeKHR SwapChain::choose_swap_present_mode(const std::vector<VkPresentModeKHR> &available_present_modes) {
        for (const auto &available_present_mode: available_present_modes) {
            if (available_present_mode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return available_present_mode;
            }
        }

        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D SwapChain::choose_swap_extent(const VkSurfaceCapabilitiesKHR &capabilities) {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        } else {
            VkExtent2D vk_actual_extent = vk_window_extent;
            vk_actual_extent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, vk_actual_extent.width));
            vk_actual_extent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, vk_actual_extent.height));

            return vk_actual_extent;
        }
    }

    VkFormat SwapChain::find_depth_format() {
        return device->find_supported_format({VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT}, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    }

}