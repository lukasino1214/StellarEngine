//
// Created by lukas on 27.11.21.
//

#include "OffScreen.h"

#include <iostream>
#include <glm/glm.hpp>
#include <vulkan/vulkan_core.h>

namespace Engine {
    OffScreen::OffScreen(std::shared_ptr<Device> device, uint32_t width, uint32_t height) : m_Width{width}, m_Height{height}, m_Device{device} {
        CreateImages();
    }

    void OffScreen::CreateImages() {
        auto device = m_Device->device();

        // Find a suitable depth format
        VkFormat fbDepthFormat = m_Device->findSupportedFormat(
                {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
                VK_IMAGE_TILING_OPTIMAL,
                VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

        if(!first) {
            vkDeviceWaitIdle(m_Device->device());
            delete color;
            delete depth;
            delete renderpass;

            delete sampler;
        }

        color = new FrameBufferAttachment(m_Device, {
                .format = Format::B8G8R8A8_SRGB,
                .dimensions = { m_Width, m_Height, 1 },
                .usage = UsageFlags::COLOR_ATTACHMENT | UsageFlags::SAMPLED,
                .finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                .isDepth = false
        });

        depth = new FrameBufferAttachment(m_Device, {
                .format = (Format)fbDepthFormat,
                .dimensions = { m_Width, m_Height, 1 },
                .usage = UsageFlags::DEPTH_STENCIL_ATTACHMENT,
                .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
                .isDepth = true
        });

        sampler = new Sampler(m_Device, {
                .minFilter = Filter::LINEAR,
                .magFilter = Filter::LINEAR,
                .maxAnistropy = 1.0,
        });

        renderpass = new RenderPass(m_Device, {
            { .frameBufferAttachment = color, .loadOp = LoadOp::CLEAR, .storeOp = StoreOp::STORE },
            { .frameBufferAttachment = depth, .loadOp = LoadOp::CLEAR, .storeOp = StoreOp::STORE }
        }, { { .renderTargets = { 0, 1 }, .subpassInputs = {} } }, "offscreen");

        first = false;

        descriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        descriptor.imageView = color->view->GetImageView();
        descriptor.sampler = sampler->GetSampler();
    };

    void OffScreen::Start(FrameInfo frameInfo) {
        renderpass->Start(frameInfo.commandBuffer);
    }

    void OffScreen::End(FrameInfo frameInfo) {
        renderpass->End(frameInfo.commandBuffer);
    }

    OffScreen::~OffScreen() {
        delete color;
        delete depth;

        delete sampler;

        delete renderpass;
    }
}