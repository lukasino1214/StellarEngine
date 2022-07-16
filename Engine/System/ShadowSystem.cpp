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
                .format = Format::D16_UNORM,
                .dimensions = { m_Width, m_Height, 1 },
                .usage = UsageFlags::DEPTH_STENCIL_ATTACHMENT | UsageFlags::SAMPLED,
                .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
                .isDepth = true
        });

        m_Sampler = new Sampler(m_Device, {
                .minFilter = Filter::LINEAR,
                .magFilter = Filter::LINEAR,
                .maxAnistropy = 1.0,
                .addressMode = AddressMode::CLAMP_TO_EDGE
        });

        std::cout << "Shadow" << std::endl;

        renderpass = new RenderPass(m_Device, {
            { .frameBufferAttachment = depth, .loadOp = LoadOp::CLEAR, .storeOp = StoreOp::STORE }
        }, { { .renderTargets = { 0 } } }, "shadow");


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
        pipelineConfig.renderPass = renderpass->GetRenderPass();
        pipelineConfig.pipelineLayout = pipelineLayout;
        m_Pipeline = std::make_unique<Pipeline>(m_Device, pipelineConfig, ShaderPaths {
            .vertPath = "assets/shaders/shadow.vert",
        });
    }

    void ShadowSystem::Render(FrameInfo &frameInfo, std::shared_ptr<Scene> Scene) {
        renderpass->Start(frameInfo.commandBuffer);

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

        renderpass->End(frameInfo.commandBuffer);
    }

    ShadowSystem::~ShadowSystem() {
        auto device = m_Device->device();

        delete depth;
        delete m_Sampler;

        delete renderpass;
        
        vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    }
}