#include "RenderPass.h"
#include "FrameBufferAttachment.h"
#include <cstddef>
#include <cstdint>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace Engine {
    RenderPass::RenderPass(std::shared_ptr<Engine::Device> device, std::vector<Attachment> attachments, std::vector<SubpassInfo> subpassInfos, std::string debugName) : m_Device{device}, m_Attachments{attachments} {
        auto VK_Device = m_Device->device();

        std::vector<VkAttachmentDescription> attachmentDescriptions;
        attachmentDescriptions.resize(m_Attachments.size());

        for(u32 i = 0; i < m_Attachments.size(); i++) {
            attachmentDescriptions[i].format = (VkFormat)attachments[i].frameBufferAttachment->image->GetFormat();
            attachmentDescriptions[i].samples = VK_SAMPLE_COUNT_1_BIT;
            attachmentDescriptions[i].loadOp = (VkAttachmentLoadOp)attachments[0].loadOp;
            attachmentDescriptions[i].storeOp = (VkAttachmentStoreOp)attachments[0].storeOp; 
            attachmentDescriptions[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; // I don't care maybe change this in future
            attachmentDescriptions[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; // this too
            attachmentDescriptions[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            if(m_Attachments[i].frameBufferAttachment->isDepth) {
                attachmentDescriptions[i].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
            } else {
                attachmentDescriptions[i].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; // TODO
            }
        }

        std::vector<VkAttachmentReference> test;
        test.resize(1);

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
                if(attachment.frameBufferAttachment->isDepth) {
                    hasDepth = true;
                }
            }

            for(uint32_t j = 0; j < subpassInfos[i].renderTargets.size(); j++) {
                uint32_t renderTarget = subpassInfos[i].renderTargets[j];
                if(attachments[renderTarget].frameBufferAttachment->isDepth) {
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
                if(attachments[renderTarget].frameBufferAttachment->isDepth) {
                    subpassDescriptionMy.depthReference.attachment = renderTarget;
                    subpassDescriptionMy.depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

                    hasDepth = true;
                } else {
                    VkAttachmentReference inputReference{};
                    inputReference.attachment = renderTarget;
                    inputReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
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
        for(auto& subpass : subpassDescriptions) {
            subpassDescriptionsVK.push_back(subpass.subpassDescription);
        }


        VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentDescriptions.size());
		renderPassInfo.pAttachments = attachmentDescriptions.data();
		renderPassInfo.subpassCount = static_cast<uint32_t>(subpassDescriptionsVK.size());
		renderPassInfo.pSubpasses = subpassDescriptionsVK.data();
		renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
		renderPassInfo.pDependencies = dependencies.data();

		if (vkCreateRenderPass(VK_Device, &renderPassInfo, nullptr, &VK_RenderPass) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create renderpass");
        }

        std::vector<VkImageView> imageViews;
        imageViews.resize(m_Attachments.size());

        for(u32 i = 0; i < m_Attachments.size(); i++) {
            imageViews[i] = m_Attachments[i].frameBufferAttachment->view->GetImageView();
        }

        VkFramebufferCreateInfo fbufCreateInfo = {};
        fbufCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fbufCreateInfo.renderPass = VK_RenderPass;
        fbufCreateInfo.attachmentCount = imageViews.size();
        fbufCreateInfo.pAttachments = imageViews.data();
        fbufCreateInfo.width = m_Attachments[0].frameBufferAttachment->dimensions.x;
        fbufCreateInfo.height = m_Attachments[0].frameBufferAttachment->dimensions.y;
        fbufCreateInfo.layers = m_Attachments[0].frameBufferAttachment->dimensions.z;

        if (vkCreateFramebuffer(m_Device->device(), &fbufCreateInfo, nullptr, &VK_FrameBuffer) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create framebuffer");
        }

        clearValues.resize(m_Attachments.size());
        for(uint32_t i = 0; i < m_Attachments.size(); i++) {
            if(m_Attachments[i].frameBufferAttachment->isDepth) {
                clearValues[i].depthStencil = {1.0f, 0};
            } else {
                clearValues[i].color = {0.01f, 0.01f, 0.01f, 1.0f};
            }
        }
    }

    RenderPass::~RenderPass() {
        auto VK_Device = m_Device->device();

        vkDestroyRenderPass(VK_Device, VK_RenderPass, nullptr);
        vkDestroyFramebuffer(VK_Device, VK_FrameBuffer, nullptr);
    }

    void RenderPass::NextSubpass(VkCommandBuffer commandBuffer) {
        vkCmdNextSubpass(commandBuffer, VK_SUBPASS_CONTENTS_INLINE);
    }

    void RenderPass::Start(VkCommandBuffer commandBuffer) {
        VkRenderPassBeginInfo renderPassBeginInfo{};
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.renderPass = VK_RenderPass;
        renderPassBeginInfo.framebuffer = VK_FrameBuffer;
        renderPassBeginInfo.renderArea.extent.width = m_Attachments[0].frameBufferAttachment->dimensions.x;
        renderPassBeginInfo.renderArea.extent.height = m_Attachments[0].frameBufferAttachment->dimensions.y;
        renderPassBeginInfo.clearValueCount = clearValues.size();
        renderPassBeginInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport = {};
        viewport.width = (float) m_Attachments[0].frameBufferAttachment->dimensions.x;
        viewport.height = (float) m_Attachments[0].frameBufferAttachment->dimensions.y;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor = {};
        scissor.extent.width = m_Attachments[0].frameBufferAttachment->dimensions.x;
        scissor.extent.height = m_Attachments[0].frameBufferAttachment->dimensions.y;
        scissor.offset.x = 0;
        scissor.offset.y = 0;
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    }

    void RenderPass::End(VkCommandBuffer commandBuffer) {
        vkCmdEndRenderPass(commandBuffer);
    }
}