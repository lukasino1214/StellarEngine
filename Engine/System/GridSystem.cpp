//
// Created by lukas on 25.11.21.
//

#include "GridSystem.h"
#include "../Graphics/core.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <array>
#include <cassert>
#include <stdexcept>

namespace Engine {

    GridSystem::GridSystem(VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout) {
        createPipelineLayout(globalSetLayout);
        createPipeline(renderPass);
    }

    GridSystem::~GridSystem() {
        vkDestroyPipelineLayout(Core::m_Device->device(), pipelineLayout, nullptr);
    }

    void GridSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout) {
        std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalSetLayout};

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        pipelineLayoutInfo.pPushConstantRanges = nullptr;
        if (vkCreatePipelineLayout(Core::m_Device->device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) !=
            VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }
    }

    void GridSystem::createPipeline(VkRenderPass renderPass) {
        assert(pipelineLayout != VK_NULL_HANDLE && "Cannot create pipeline before pipeline layout");

        PipelineConfigInfo configInfo{};
        Pipeline::defaultPipelineConfigInfo(configInfo);
        configInfo.renderPass = renderPass;
        configInfo.pipelineLayout = pipelineLayout;
        configInfo.bindingDescriptions.clear();
        configInfo.attributeDescriptions.clear();
        configInfo.colorBlendAttachment.blendEnable = VK_TRUE;
        configInfo.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        configInfo.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;

        m_Pipeline = std::make_unique<Pipeline>(
                "assets/shaders/grid.vert.spv",
                "assets/shaders/grid.frag.spv",
                configInfo, false);
    }

    void GridSystem::render(FrameInfo& frameInfo) {
        m_Pipeline->bind(frameInfo.commandBuffer);

        vkCmdBindDescriptorSets(
                frameInfo.commandBuffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                pipelineLayout,
                0,
                1,
                &frameInfo.globalDescriptorSet,
                0,
                nullptr);


            vkCmdDraw(frameInfo.commandBuffer, 6, 1, 0, 0);
    }
}