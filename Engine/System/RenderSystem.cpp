#include "RenderSystem.h"
#include "../Graphics/Core.h"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace Engine {

    struct SimplePushConstantData {
        glm::mat4 modelMatrix{1.0f};
        glm::mat4 normalMatrix{1.0f};
    };

    RenderSystem::RenderSystem(std::shared_ptr<Device> device, VkRenderPass renderPass) : m_Device{device} {
        createPipelineLayout();
        createPipeline(renderPass);
    }

    RenderSystem::~RenderSystem() {
        vkDestroyPipelineLayout(m_Device->device(), pipelineLayout, nullptr);
    }

    void RenderSystem::createPipelineLayout() {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(SimplePushConstantData);

        std::vector<VkDescriptorSetLayout> descriptorSetLayouts{Core::m_GlobalSetLayout->getDescriptorSetLayout(),
                                                                Core::m_EntitySetLayout->getDescriptorSetLayout(),
                                                                Core::m_ShadowLayout->getDescriptorSetLayout()};

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
    }

    void RenderSystem::createPipeline(VkRenderPass renderPass) {
        assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

        PipelineConfigInfo pipelineConfig{};
        Pipeline::defaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = pipelineLayout;
        /*m_Pipeline = std::make_unique<Pipeline>(m_Device, pipelineConfig,  "assets/shaders/simple_shader.vert",
                                                "assets/shaders/simple_shader.frag");*/

        m_Pipeline = std::make_unique<Pipeline>(m_Device, pipelineConfig, ShaderPaths {
            .vertPath = "assets/shaders/simple_shader.vert",
            .fragPath = "assets/shaders/simple_shader.frag"
        });
    }

    void RenderSystem::renderGameObjects(FrameInfo &frameInfo, const Ref<Scene> &Scene) {
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


                //TODO: THIS SHIT
                auto Model = entity.GetComponent<ModelComponent>().GetModel();
                Model->bind(frameInfo.commandBuffer);
                vkCmdBindDescriptorSets(frameInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 2, 1,
                                        &frameInfo.ShadowSet, 0, nullptr);
                Model->draw(frameInfo, pipelineLayout);
            }
        });

    }

}  // namespace lve
