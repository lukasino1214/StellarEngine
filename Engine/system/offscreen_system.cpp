#include "offscreen_system.h"

#include <iostream>
#include <glm/glm.hpp>
#include <vulkan/vulkan_core.h>

namespace Engine {
    OffScreenSystem::OffScreenSystem(std::shared_ptr<Device> _device, uint32_t _width, uint32_t _height) : width{_width}, height{_height}, device{_device} {
        create_images();

        renderpass = new RenderPass(device, {
                { .frameBufferAttachment = color, .loadOp = LoadOp::CLEAR, .storeOp = StoreOp::STORE },
                { .frameBufferAttachment = depth, .loadOp = LoadOp::CLEAR, .storeOp = StoreOp::STORE }
        }, { { .renderTargets = { 0, 1 }, .subpassInputs = {} } });

        create_framebuffer();
    }

    void OffScreenSystem::create_framebuffer() {
        if(!first) {
            delete framebuffer;
        }

        framebuffer = new Framebuffer(device, renderpass->vk_renderpass, { color, depth });

        first = false;
    }

    void OffScreenSystem::create_images() {
        if(!first) {
            vkDeviceWaitIdle(device->vk_device);
            delete color;
            delete depth;

            delete sampler;
        }

        VkFormat fb_depth_format = device->find_supported_format({VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT}, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

        color = new FrameBufferAttachment(device, {
                .format = ImageFormat::B8G8R8A8_UNORM,
                .dimensions = { width, height, 1 },
                .usage = ImageUsageFlagBits::COLOR_ATTACHMENT | ImageUsageFlagBits::SAMPLED,
        });

        depth = new FrameBufferAttachment(device, {
                .format = (ImageFormat)fb_depth_format,
                .dimensions = { width, height, 1 },
                .usage = ImageUsageFlagBits::DEPTH_STENCIL_ATTACHMENT,
        });

        sampler = new Sampler(device, {
                .min_filter = Filter::LINEAR,
                .mag_filter = Filter::LINEAR,
                .max_anistropy = 1.0,
        });

        VkDescriptorImageInfo image_info = {};
        image_info.sampler = sampler->vk_sampler;
        image_info.imageView = color->image_view->vk_image_view;
        image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        DescriptorWriter(*Core::postprocessing_descriptor_set_layout, *Core::global_descriptor_pool)
                .write_image(0, &image_info)
                .build(device, vk_present_descriptor_set);
    };

    void OffScreenSystem::start(FrameInfo frame_info) {
        renderpass->start(framebuffer, frame_info.command_buffer);
    }

    void OffScreenSystem::end(FrameInfo frame_info) {
        renderpass->end(frame_info.command_buffer);
    }

    OffScreenSystem::~OffScreenSystem() {
        delete color;
        delete depth;

        delete sampler;
        delete framebuffer;

        delete renderpass;
    }
}