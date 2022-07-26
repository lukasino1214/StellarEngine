#include "renderpass.h"
#include "framebuffer_attachment.h"
#include <cstddef>
#include <cstdint>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace Engine {
    RenderPass::RenderPass(std::shared_ptr<Device> device, std::vector<Attachment> attachments, std::vector<SubpassInfo> subpassInfos) : m_Device{device} {
        auto VK_Device = m_Device->vk_device;

        std::vector<VkAttachmentDescription> attachmentDescriptions;
        attachmentDescriptions.resize(attachments.size());

        for(u32 i = 0; i < attachments.size(); i++) {
            attachmentDescriptions[i].format = (VkFormat)attachments[i].frameBufferAttachment->image->get_format();
            attachmentDescriptions[i].samples = VK_SAMPLE_COUNT_1_BIT;
            attachmentDescriptions[i].loadOp = (VkAttachmentLoadOp)attachments[0].loadOp;
            attachmentDescriptions[i].storeOp = (VkAttachmentStoreOp)attachments[0].storeOp; 
            attachmentDescriptions[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; // I don't care maybe change this in future
            attachmentDescriptions[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; // this too
            attachmentDescriptions[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            if(attachments[i].frameBufferAttachment->is_depth) {
                attachmentDescriptions[i].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
            } else {
                attachmentDescriptions[i].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; // TODO
            }
        }

        // Subpasses
        std::vector<SubpassDescription> subpassDescriptions{};
        for(uint32_t i = 0; i < subpassInfos.size(); i++) {
            SubpassDescription subpassDescriptionMy {};
            subpassDescriptionMy.subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpassDescriptionMy.subpassDescription.colorAttachmentCount = 0;
            subpassDescriptionMy.subpassDescription.inputAttachmentCount = 0;

            bool hasDepth = false;
            bool hasColor = false;
            bool hasInput = false;

            for(auto& attachment : attachments) {
                if(attachment.frameBufferAttachment->is_depth) {
                    hasDepth = true;
                }
            }

            for(uint32_t j = 0; j < subpassInfos[i].renderTargets.size(); j++) {
                uint32_t renderTarget = subpassInfos[i].renderTargets[j];
                if(attachments[renderTarget].frameBufferAttachment->is_depth) {
                    subpassDescriptionMy.depthReference.attachment = renderTarget;
                    subpassDescriptionMy.depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

                    hasDepth = true;
                } else {
                    VkAttachmentReference colorReference{};
                    colorReference.attachment = renderTarget;
                    colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                    subpassDescriptionMy.colorReferences.push_back(colorReference);

                    hasColor = true;
                }
            }

            for(uint32_t j = 0; j < subpassInfos[i].subpassInputs.size(); j++) {
                uint32_t renderTarget = subpassInfos[i].subpassInputs[j];
                if(attachments[renderTarget].frameBufferAttachment->is_depth) {
                    subpassDescriptionMy.depthReference.attachment = renderTarget;
                    subpassDescriptionMy.depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

                    hasDepth = true;
                } else {
                    VkAttachmentReference inputReference{};
                    inputReference.attachment = renderTarget;
                    inputReference.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                    subpassDescriptionMy.inputReferences.push_back(inputReference);

                    hasInput = true;
                }
            }

            if(hasDepth) {
                subpassDescriptionMy.subpassDescription.pDepthStencilAttachment = &subpassDescriptionMy.depthReference;
            }

            if(hasColor) {
                subpassDescriptionMy.subpassDescription.pColorAttachments = subpassDescriptionMy.colorReferences.data();
                subpassDescriptionMy.subpassDescription.colorAttachmentCount = subpassDescriptionMy.colorReferences.size();

            }

            if(hasInput) {
                subpassDescriptionMy.subpassDescription.pInputAttachments = subpassDescriptionMy.inputReferences.data();
                subpassDescriptionMy.subpassDescription.inputAttachmentCount = subpassDescriptionMy.inputReferences.size();
            }

            subpassDescriptions.push_back(std::move(subpassDescriptionMy));
        }

        // Dependencies
        std::vector<VkSubpassDependency> dependencies;
        if(subpassInfos.size() != 1) {
            dependencies.resize(subpassInfos.size() + 1);
            VkSubpassDependency& firstDependency = dependencies[0];
            firstDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
            firstDependency.dstSubpass = 0;
            firstDependency.srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
            firstDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            firstDependency.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
            firstDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            firstDependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

            for (size_t i = 1; i < (dependencies.size() - 1); i++) {
                dependencies[i].srcSubpass = i-1;
                dependencies[i].dstSubpass = i;
                dependencies[i].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                dependencies[i].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                dependencies[i].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                dependencies[i].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                dependencies[i].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
            }

            VkSubpassDependency& lastDependency = *(dependencies.end() - 1);
            lastDependency.srcSubpass = subpassInfos.size() - 1;
            lastDependency.dstSubpass = VK_SUBPASS_EXTERNAL;
            lastDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            lastDependency.dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
            lastDependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            lastDependency.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
            lastDependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
        } else {
            dependencies.resize(2);
            dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
            dependencies[0].dstSubpass = 0;
            dependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            dependencies[0].srcAccessMask = 0;
            dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

            dependencies[1].srcSubpass = 0;
            dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
            dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
            dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
            dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
        }

        std::vector<VkSubpassDescription> subpassDescriptionsVK{};
        for(u32 i  = 0; i < subpassDescriptions.size(); i++) {
            subpassDescriptionsVK.push_back(subpassDescriptions[i].subpassDescription);
        }

        VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentDescriptions.size());
		renderPassInfo.pAttachments = attachmentDescriptions.data();
		renderPassInfo.subpassCount = static_cast<uint32_t>(subpassDescriptionsVK.size());
		renderPassInfo.pSubpasses = subpassDescriptionsVK.data();
		renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
		renderPassInfo.pDependencies = dependencies.data();

		if (vkCreateRenderPass(VK_Device, &renderPassInfo, nullptr, &vk_renderpass) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create renderpass");
        }

        clearValues.resize(attachments.size());
        for(uint32_t i = 0; i < attachments.size(); i++) {
            if(attachments[i].frameBufferAttachment->is_depth) {
                clearValues[i].depthStencil = {1.0f, 0};
            } else {
                clearValues[i].color = {0.01f, 0.01f, 0.01f, 1.0f};
            }
        }
    }

    RenderPass::~RenderPass() {
        auto VK_Device = m_Device->vk_device;

        vkDestroyRenderPass(VK_Device, vk_renderpass, nullptr);
    }

    void RenderPass::next_subpass(VkCommandBuffer command_buffer) {
        vkCmdNextSubpass(command_buffer, VK_SUBPASS_CONTENTS_INLINE);
    }

    void RenderPass::start(Framebuffer* framebuffer, VkCommandBuffer command_buffer) {
        VkRenderPassBeginInfo renderPassBeginInfo{};
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.renderPass = vk_renderpass;
        renderPassBeginInfo.framebuffer = framebuffer->vk_framebuffer;
        renderPassBeginInfo.renderArea.extent.width = framebuffer->width;
        renderPassBeginInfo.renderArea.extent.height = framebuffer->height;
        renderPassBeginInfo.clearValueCount = clearValues.size();
        renderPassBeginInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(command_buffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport = {};
        viewport.width = static_cast<float>(framebuffer->width);
        viewport.height = static_cast<float>(framebuffer->height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(command_buffer, 0, 1, &viewport);

        VkRect2D scissor = {};
        scissor.extent.width = framebuffer->width;
        scissor.extent.height = framebuffer->height;
        scissor.offset.x = 0;
        scissor.offset.y = 0;
        vkCmdSetScissor(command_buffer, 0, 1, &scissor);
    }

    void RenderPass::end(VkCommandBuffer command_buffer) {
        vkCmdEndRenderPass(command_buffer);
    }
}