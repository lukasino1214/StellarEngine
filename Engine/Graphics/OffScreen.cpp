//
// Created by lukas on 27.11.21.
//

#include "OffScreen.h"

#include <array>
#include <iostream>
#include <glm/glm.hpp>
#include "pipeline.h"
#include <memory>

namespace Engine {

    struct SimplePushConstantData {
        glm::mat4 modelMatrix{1.0f};
        glm::mat4 normalMatrix{1.0f};
    };


    OffScreen::OffScreen(Device &device) : m_Device{device} {
        pass.width = 1280;
        pass.height = 720;

        CreateImages();
        after = true;
    }

    void OffScreen::CreateImages() {
        if(after) {
            /*vkDestroyImageView(m_Device.device(), pass.color.view, nullptr);
            vkDestroyImage(m_Device.device(), pass.color.image, nullptr);
            vkFreeMemory(m_Device.device(), pass.color.mem, nullptr);

            // Depth attachment
            vkDestroyImageView(m_Device.device(), pass.depth.view, nullptr);
            vkDestroyImage(m_Device.device(), pass.depth.image, nullptr);
            vkFreeMemory(m_Device.device(), pass.depth.mem, nullptr);

            vkDestroyRenderPass(m_Device.device(), pass.renderPass, nullptr);
            vkDestroySampler(m_Device.device(), pass.sampler, nullptr);
            vkDestroyFramebuffer(m_Device.device(), pass.frameBuffer, nullptr);*/
        }

        // Find a suitable depth format
        VkFormat fbDepthFormat = m_Device.findSupportedFormat(
                {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
                VK_IMAGE_TILING_OPTIMAL,
                VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

        // Color attachment
        VkImageCreateInfo image = {};
        image.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        image.imageType = VK_IMAGE_TYPE_2D;
        image.format = VK_FORMAT_B8G8R8A8_SRGB;
        image.extent.width = pass.width;
        image.extent.height = pass.height;
        image.extent.depth = 1;
        image.mipLevels = 1;
        image.arrayLayers = 1;
        image.samples = VK_SAMPLE_COUNT_1_BIT;
        image.tiling = VK_IMAGE_TILING_OPTIMAL;
        // We will sample directly from the color attachment
        image.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

        VkMemoryAllocateInfo memAlloc = {};
        memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        VkMemoryRequirements memReqs;

        if (vkCreateImage(m_Device.device(), &image, nullptr, &pass.color.image) != VK_SUCCESS) {
            std::cout << "Failed to create image" << std::endl;
        }
        vkGetImageMemoryRequirements(m_Device.device(), pass.color.image, &memReqs);
        memAlloc.allocationSize = memReqs.size;
        memAlloc.memoryTypeIndex = m_Device.findMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        //memAlloc.memoryTypeIndex = getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        if (vkAllocateMemory(m_Device.device(), &memAlloc, nullptr, &pass.color.mem) != VK_SUCCESS) {
            std::cout << "Failed to allocate memory" << std::endl;
        }
        if (vkBindImageMemory(m_Device.device(), pass.color.image, pass.color.mem, 0) != VK_SUCCESS) {
            std::cout << "Failed to bind memory" << std::endl;
        }

        VkImageViewCreateInfo colorImageView = {};
        colorImageView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        colorImageView.viewType = VK_IMAGE_VIEW_TYPE_2D;
        colorImageView.format = VK_FORMAT_B8G8R8A8_SRGB;
        colorImageView.subresourceRange = {};
        colorImageView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        colorImageView.subresourceRange.baseMipLevel = 0;
        colorImageView.subresourceRange.levelCount = 1;
        colorImageView.subresourceRange.baseArrayLayer = 0;
        colorImageView.subresourceRange.layerCount = 1;
        colorImageView.image = pass.color.image;
        if (vkCreateImageView(m_Device.device(), &colorImageView, nullptr, &pass.color.view) != VK_SUCCESS) {
            std::cout << "Failed to create color image view" << std::endl;
        }

        // Create sampler to sample from the attachment in the fragment shader
        VkSamplerCreateInfo samplerInfo = {};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeV = samplerInfo.addressModeU;
        samplerInfo.addressModeW = samplerInfo.addressModeU;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.maxAnisotropy = 1.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 1.0f;
        samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
        if (vkCreateSampler(m_Device.device(), &samplerInfo, nullptr, &pass.sampler) != VK_SUCCESS) {
            std::cout << "Failed to create sampler" << std::endl;
        }

        // Depth stencil attachment
        image.format = fbDepthFormat;
        image.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

        if (vkCreateImage(m_Device.device(), &image, nullptr, &pass.depth.image) != VK_SUCCESS) {
            std::cout << "Failed to create image" << std::endl;
        }
        vkGetImageMemoryRequirements(m_Device.device(), pass.depth.image, &memReqs);
        memAlloc.allocationSize = memReqs.size;
        memAlloc.memoryTypeIndex = m_Device.findMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        if (vkAllocateMemory(m_Device.device(), &memAlloc, nullptr, &pass.depth.mem) != VK_SUCCESS) {
            std::cout << "Failed to allocate memory" << std::endl;
        }
        if (vkBindImageMemory(m_Device.device(), pass.depth.image, pass.depth.mem, 0) != VK_SUCCESS) {
            std::cout << "Failed to bind memory" << std::endl;
        }

        VkImageViewCreateInfo depthStencilView = {};
        depthStencilView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        depthStencilView.viewType = VK_IMAGE_VIEW_TYPE_2D;
        depthStencilView.format = fbDepthFormat;
        depthStencilView.flags = 0;
        depthStencilView.subresourceRange = {};
        // if is something wrong with depth uncomment this
        //depthStencilView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        depthStencilView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        depthStencilView.subresourceRange.baseMipLevel = 0;
        depthStencilView.subresourceRange.levelCount = 1;
        depthStencilView.subresourceRange.baseArrayLayer = 0;
        depthStencilView.subresourceRange.layerCount = 1;
        depthStencilView.image = pass.depth.image;
        if (vkCreateImageView(m_Device.device(), &depthStencilView, nullptr, &pass.depth.view) != VK_SUCCESS) {
            std::cout << "Failed to create depth stencil view" << std::endl;
        }

        // Create a separate render pass for the offscreen rendering as it may differ from the one used for scene rendering

        std::array<VkAttachmentDescription, 2> attchmentDescriptions = {};
        // Color attachment
        attchmentDescriptions[0].format = VK_FORMAT_B8G8R8A8_SRGB;
        attchmentDescriptions[0].samples = VK_SAMPLE_COUNT_1_BIT;
        attchmentDescriptions[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attchmentDescriptions[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attchmentDescriptions[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attchmentDescriptions[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attchmentDescriptions[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attchmentDescriptions[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        // Depth attachment
        attchmentDescriptions[1].format = fbDepthFormat;
        attchmentDescriptions[1].samples = VK_SAMPLE_COUNT_1_BIT;
        attchmentDescriptions[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attchmentDescriptions[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attchmentDescriptions[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attchmentDescriptions[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attchmentDescriptions[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attchmentDescriptions[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference colorReference = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
        VkAttachmentReference depthReference = {1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

        VkSubpassDescription subpassDescription = {};
        subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDescription.colorAttachmentCount = 1;
        subpassDescription.pColorAttachments = &colorReference;
        subpassDescription.pDepthStencilAttachment = &depthReference;

        // Use subpass dependencies for layout transitions
        std::array<VkSubpassDependency, 2> dependencies;

        dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[0].dstSubpass = 0;
        dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        dependencies[1].srcSubpass = 0;
        dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        // Create the actual renderpass
        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attchmentDescriptions.size());
        renderPassInfo.pAttachments = attchmentDescriptions.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpassDescription;
        renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
        renderPassInfo.pDependencies = dependencies.data();

        if (vkCreateRenderPass(m_Device.device(), &renderPassInfo, nullptr, &pass.renderPass) != VK_SUCCESS) {
            std::cout << "Failed to create render pass" << std::endl;
        }

        VkImageView attachments[2];
        attachments[0] = pass.color.view;
        attachments[1] = pass.depth.view;

        VkFramebufferCreateInfo fbufCreateInfo = {};
        fbufCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fbufCreateInfo.renderPass = pass.renderPass;
        fbufCreateInfo.attachmentCount = 2;
        fbufCreateInfo.pAttachments = attachments;
        fbufCreateInfo.width = pass.width;
        fbufCreateInfo.height = pass.height;
        fbufCreateInfo.layers = 1;

        if (vkCreateFramebuffer(m_Device.device(), &fbufCreateInfo, nullptr, &pass.frameBuffer) != VK_SUCCESS) {
            std::cout << "Failed to create framebuffer" << std::endl;
        }

        // Fill a descriptor for later use in a descriptor set
        pass.descriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        pass.descriptor.imageView = pass.color.view;
        pass.descriptor.sampler = pass.sampler;
    };

    void OffScreen::Start(FrameInfo frameInfo) {
        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {0.01f, 0.01f, 0.01f, 1.0f};
        clearValues[1].depthStencil = {1.0f, 0};
        VkDeviceSize offsets[1] = {0};

        VkRenderPassBeginInfo renderPassBeginInfo{};
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.renderPass = pass.renderPass;
        renderPassBeginInfo.framebuffer = pass.frameBuffer;
        renderPassBeginInfo.renderArea.extent.width = pass.width;
        renderPassBeginInfo.renderArea.extent.height = pass.height;
        renderPassBeginInfo.clearValueCount = 2;
        renderPassBeginInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(frameInfo.commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport = {};
        viewport.width = (float) pass.width;
        viewport.height = (float) pass.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(frameInfo.commandBuffer, 0, 1, &viewport);

        VkRect2D scissor = {};
        scissor.extent.width = pass.width;
        scissor.extent.height = pass.height;
        scissor.offset.x = 0;
        scissor.offset.y = 0;
        vkCmdSetScissor(frameInfo.commandBuffer, 0, 1, &scissor);
    }

    void OffScreen::End(FrameInfo frameInfo) {
        vkCmdEndRenderPass(frameInfo.commandBuffer);
    }
}