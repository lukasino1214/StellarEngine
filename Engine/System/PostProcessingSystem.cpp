//
// Created by lukas on 27.11.21.
//

#include "PostProcessingSystem.h"

#include <iostream>
#include <glm/glm.hpp>
#include <vulkan/vulkan_core.h>

namespace Engine {

    struct SimplePushConstantData {
        glm::mat4 modelMatrix{1.0f};
        glm::mat4 normalMatrix{1.0f};
    };


    PostProcessingSystem::PostProcessingSystem(std::shared_ptr<Device> device, uint32_t width, uint32_t height)
            : m_Width{width}, m_Height{height}, m_Device{device} {
        CreateImages();
        createPipelineLayout();
        createPipeline(renderPass);
    }

    void PostProcessingSystem::CreateImages() {
        auto device = m_Device->device();

        // Find a suitable depth format
        VkFormat fbDepthFormat = m_Device->findSupportedFormat({VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT}, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

        if(!first) {
            vkDeviceWaitIdle(m_Device->device());
            delete color;
            delete depth;

            delete sampler;

            vkDestroyRenderPass(m_Device->device(), renderPass, nullptr);
            vkDestroyFramebuffer(m_Device->device(), frameBuffer, nullptr);
        }

        color = new FrameBufferAttachment(m_Device, {
                .format = Format::B8G8R8A8_SRGB,
                .dimensions = { m_Width, m_Height, 1 },
                .usage = UsageFlags::COLOR_ATTACHMENT | UsageFlags::SAMPLED,
        });

        depth = new FrameBufferAttachment(m_Device, {
                .format = (Format)fbDepthFormat,
                .dimensions = { m_Width, m_Height, 1 },
                .usage = UsageFlags::DEPTH_STENCIL_ATTACHMENT,
        });

        sampler = new Sampler(m_Device, {
                .minFilter = Filter::LINEAR,
                .magFilter = Filter::LINEAR,
                .maxAnistropy = 1.0,
        });

        first = false;

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

        if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
            std::cout << "Failed to create render pass" << std::endl;
        }

        VkImageView attachments[2];
        attachments[0] = color->view->GetImageView();
        attachments[1] = depth->view->GetImageView();

        VkFramebufferCreateInfo fbufCreateInfo = {};
        fbufCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fbufCreateInfo.renderPass = renderPass;
        fbufCreateInfo.attachmentCount = 2;
        fbufCreateInfo.pAttachments = attachments;
        fbufCreateInfo.width = m_Width;
        fbufCreateInfo.height = m_Height;
        fbufCreateInfo.layers = 1;

        if (vkCreateFramebuffer(device, &fbufCreateInfo, nullptr, &frameBuffer) != VK_SUCCESS) {
            std::cout << "Failed to create framebuffer" << std::endl;
        }

        // Fill a descriptor for later use in a descriptor set
        descriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        descriptor.imageView = color->view->GetImageView();
        descriptor.sampler = sampler->GetSampler();
    };

    void PostProcessingSystem::createPipelineLayout() {
        /*VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(PointLightPushConstantData);*/

        std::vector<VkDescriptorSetLayout> descriptorSetLayouts{Core::m_PostProcessingLayout->getDescriptorSetLayout(),
                                                                Core::m_GlobalSetLayout->getDescriptorSetLayout()};

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        pipelineLayoutInfo.pPushConstantRanges = nullptr;
        if (vkCreatePipelineLayout(m_Device->device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) !=
            VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }
    }

    void PostProcessingSystem::createPipeline(VkRenderPass render_pass) {
        assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

        PipelineConfigInfo pipelineConfig{};
        Pipeline::defaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.attributeDescriptions.clear();
        pipelineConfig.bindingDescriptions.clear();
        pipelineConfig.renderPass = render_pass;
        pipelineConfig.pipelineLayout = pipelineLayout;
        /*m_Pipeline = std::make_unique<Pipeline>(m_Device, pipelineConfig, "assets/shaders/post_processing.vert",
                                                "assets/shaders/post_processing.frag");*/

        m_Pipeline = std::make_unique<Pipeline>(m_Device, pipelineConfig, ShaderPaths {
            .vertPath = "assets/shaders/post_processing.vert",
            .fragPath = "assets/shaders/post_processing.frag"
        });
    }

    void PostProcessingSystem::Render(FrameInfo &frameInfo, VkDescriptorSet &set) {
        Start(frameInfo);
        m_Pipeline->bind(frameInfo.commandBuffer);
        std::vector<VkDescriptorSet> sets = {set, frameInfo.globalDescriptorSet};
        vkCmdBindDescriptorSets(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 2,
                                sets.data(), 0, nullptr);
        vkCmdDraw(frameInfo.commandBuffer, 6, 1, 0, 0);
        End(frameInfo);
    }

    void PostProcessingSystem::RenderWithoutRenderpass(FrameInfo &frameInfo, VkDescriptorSet &set) {
        m_Pipeline->bind(frameInfo.commandBuffer);
        std::vector<VkDescriptorSet> sets = {set, frameInfo.ShadowSet};
        vkCmdBindDescriptorSets(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 2,
                                sets.data(), 0, nullptr);
        vkCmdDraw(frameInfo.commandBuffer, 6, 1, 0, 0);
    }

    void PostProcessingSystem::Start(FrameInfo &frameInfo) {
        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {0.01f, 0.01f, 0.01f, 1.0f};
        clearValues[1].depthStencil = {1.0f, 0};
        VkDeviceSize offsets[1] = {0};

        VkRenderPassBeginInfo renderPassBeginInfo{};
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.renderPass = renderPass;
        renderPassBeginInfo.framebuffer = frameBuffer;
        renderPassBeginInfo.renderArea.extent.width = m_Width;
        renderPassBeginInfo.renderArea.extent.height = m_Height;
        renderPassBeginInfo.clearValueCount = 2;
        renderPassBeginInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(frameInfo.commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport = {};
        viewport.width = (float) m_Width;
        viewport.height = (float) m_Height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(frameInfo.commandBuffer, 0, 1, &viewport);

        VkRect2D scissor = {};
        scissor.extent.width = m_Width;
        scissor.extent.height = m_Height;
        scissor.offset.x = 0;
        scissor.offset.y = 0;
        vkCmdSetScissor(frameInfo.commandBuffer, 0, 1, &scissor);
    }

    void PostProcessingSystem::End(FrameInfo &frameInfo) {
        vkCmdEndRenderPass(frameInfo.commandBuffer);
    }

    PostProcessingSystem::~PostProcessingSystem() {
        auto device = m_Device->device();
        
        delete color;
        delete depth;

        delete sampler;

        vkDestroyRenderPass(device, renderPass, nullptr);
        vkDestroyFramebuffer(device, frameBuffer, nullptr);

        vkDestroyPipelineLayout(m_Device->device(), pipelineLayout, nullptr);
    }
}