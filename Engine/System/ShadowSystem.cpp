//
// Created by lukas on 07.05.22.
//

#include "ShadowSystem.h"
#include "../pgepch.h"
#include "../Graphics/Core.h"
#include "../Graphics/Pipeline.h"
#include <shaderc/shaderc.hpp>

namespace Engine {
    struct SimplePushConstantData {
        glm::mat4 modelMatrix{1.0f};
        glm::mat4 normalMatrix{1.0f};
    };

    ShadowSystem::ShadowSystem(std::shared_ptr<Device> p_device) : m_Device{p_device} {
        depth = new FrameBufferAttachment(m_Device, {
                .format = Format::D16,
                .dimensions = { m_Width, m_Height, 1 },
                .usage = UsageFlags::DEPTH_STENCIL_ATTACHMENT | UsageFlags::SAMPLED,
        });

        m_Sampler = new Sampler(m_Device, {
                .minFilter = Filter::LINEAR,
                .magFilter = Filter::LINEAR,
                .maxAnistropy = 1.0,
                .addressMode = AddressMode::CLAMP_TO_EDGE
        });

        VkAttachmentDescription attachmentDescription{};
        attachmentDescription.format = VK_FORMAT_D16_UNORM;
        attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
        attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;                            // Clear depth at beginning of the render pass
        attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;                        // We will read from depth, so it's important to store the depth attachment results
        attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;                    // We don't care about initial layout of the attachment
        attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;// Attachment will be transitioned to shader read at render pass end

        VkAttachmentReference depthReference = {};
        depthReference.attachment = 0;
        depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;            // Attachment will be used as depth/stencil during render pass

        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 0;                                                    // No color attachments
        subpass.pDepthStencilAttachment = &depthReference;                                    // Reference to our depth attachment

        // Use subpass dependencies for layout transitions
        std::array<VkSubpassDependency, 2> dependencies;

        dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[0].dstSubpass = 0;
        dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        dependencies[1].srcSubpass = 0;
        dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[1].srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dependencies[1].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        VkRenderPassCreateInfo renderPassCreateInfo = {};
        renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassCreateInfo.attachmentCount = 1;
        renderPassCreateInfo.pAttachments = &attachmentDescription;
        renderPassCreateInfo.subpassCount = 1;
        renderPassCreateInfo.pSubpasses = &subpass;
        renderPassCreateInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
        renderPassCreateInfo.pDependencies = dependencies.data();

        vkCreateRenderPass(m_Device->device(), &renderPassCreateInfo, nullptr, &m_RenderPass);


        VkFramebufferCreateInfo fbufCreateInfo = {};
        fbufCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fbufCreateInfo.renderPass = m_RenderPass;
        fbufCreateInfo.attachmentCount = 1;
        //fbufCreateInfo.pAttachments = &depth.view;
        fbufCreateInfo.pAttachments = &depth->view->GetImageView();
        fbufCreateInfo.width = m_Width;
        fbufCreateInfo.height = m_Height;
        fbufCreateInfo.layers = 1;

        vkCreateFramebuffer(m_Device->device(), &fbufCreateInfo, nullptr, &m_FrameBuffer);


        auto device = m_Device->device();

        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(SimplePushConstantData);

        std::vector<VkDescriptorSetLayout> descriptorSetLayouts{Core::m_GlobalSetLayout->getDescriptorSetLayout(),
                                                                Core::m_EntitySetLayout->getDescriptorSetLayout()};

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
        if (vkCreatePipelineLayout(m_Device->device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) !=
            VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }

        PipelineConfigInfo pipelineConfig{};
        Pipeline::defaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = m_RenderPass;
        pipelineConfig.pipelineLayout = pipelineLayout;
        m_Pipeline = std::make_unique<Pipeline>(m_Device, pipelineConfig, ShaderPaths {
            .vertPath = "assets/shaders/shadow.vert",
        });
    }

    void ShadowSystem::Render(FrameInfo &frameInfo, std::shared_ptr<Scene> Scene) {
        std::array<VkClearValue, 1> clearValues{};
        clearValues[0].depthStencil = {1.0f, 0};
        VkDeviceSize offsets[1] = {0};

        VkRenderPassBeginInfo renderPassBeginInfo{};
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.renderPass = m_RenderPass;
        renderPassBeginInfo.framebuffer = m_FrameBuffer;
        renderPassBeginInfo.renderArea.extent.width = m_Width;
        renderPassBeginInfo.renderArea.extent.height = m_Height;
        renderPassBeginInfo.clearValueCount = 1;
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

        vkCmdSetDepthBias(frameInfo.commandBuffer, 1.25f, 0.0f, 1.75f);

        m_Pipeline->bind(frameInfo.commandBuffer);
        Scene->m_Registry.each([&](auto entityID) {
            Entity entity = {entityID, Scene.get()};
            if (!entity)
                return;

            if (entity.HasComponent<ModelComponent>()) {
                SimplePushConstantData push{};

                auto Transform = entity.GetComponent<TransformComponent>();
                push.modelMatrix = Transform.ModelMatrix;
                push.normalMatrix = Transform.normalMatrix();

                vkCmdPushConstants(frameInfo.commandBuffer, pipelineLayout,
                                   VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                                   sizeof(SimplePushConstantData), &push);

                auto Model = entity.GetComponent<ModelComponent>().GetModel();
                Model->bind(frameInfo.commandBuffer);
                Model->draw(frameInfo, pipelineLayout);
            }
        });

        vkCmdEndRenderPass(frameInfo.commandBuffer);
    }

    ShadowSystem::~ShadowSystem() {
        auto device = m_Device->device();
        vkDestroyRenderPass(device, m_RenderPass, nullptr);
        delete depth;
        delete m_Sampler;
        vkDestroyFramebuffer(device, m_FrameBuffer, nullptr);
        vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    }
}