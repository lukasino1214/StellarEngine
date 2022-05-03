#include "PointLightSystem.h"
#include "../Graphics/Core.h"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <array>
#include <cassert>
#include <stdexcept>

namespace Engine {

    struct PointLightPushConstantData {
        glm::vec4 position;
        glm::vec4 color;
    };

    PointLightSystem::PointLightSystem(VkRenderPass renderPass) {
        createPipelineLayout();
        createPipeline(renderPass);
    }

    PointLightSystem::~PointLightSystem() {
        vkDestroyPipelineLayout(Core::m_Device->device(), pipelineLayout, nullptr);
    }

    void PointLightSystem::createPipelineLayout() {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(PointLightPushConstantData);

        std::vector<VkDescriptorSetLayout> descriptorSetLayouts{Core::m_GlobalSetLayout->getDescriptorSetLayout()};

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
        if (vkCreatePipelineLayout(Core::m_Device->device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) !=
            VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }
    }

    void PointLightSystem::createPipeline(VkRenderPass renderPass) {
        assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

        PipelineConfigInfo pipelineConfig{};
        Pipeline::defaultPipelineConfigInfo(pipelineConfig);
        Pipeline::enebleAlphaBlending(pipelineConfig);
        pipelineConfig.attributeDescriptions.clear();
        pipelineConfig.bindingDescriptions.clear();
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = pipelineLayout;
        m_Pipeline = std::make_unique<Pipeline>("assets/shaders/point_light.vert",
                                                "assets/shaders/point_light.frag", pipelineConfig);
    }

    void PointLightSystem::renderGameObjects(FrameInfo &frameInfo, const Ref<Scene> &Scene) {
        m_Pipeline->bind(frameInfo.commandBuffer);

        vkCmdBindDescriptorSets(frameInfo.commandBuffer,VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &frameInfo.globalDescriptorSet, 0,nullptr);

        Scene->m_Registry.each([&](auto entityID) {
            Entity entity = {entityID, Scene.get()};
            if (!entity)
                return;

            if (entity.HasComponent<PointLightComponent>()) {
                PointLightPushConstantData push{};

                auto Transform = entity.GetComponent<TransformComponent>();
                push.position = glm::vec4(Transform.GetTranslation(), 1.0);
                push.color = glm::vec4(entity.GetComponent<PointLightComponent>().color, 1.0);


                vkCmdPushConstants(frameInfo.commandBuffer, pipelineLayout,
                                   VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                                   sizeof(PointLightPushConstantData), &push);

                vkCmdDraw(frameInfo.commandBuffer, 6, 1, 0, 0);
            }
        });
    }

}  // namespace lve
