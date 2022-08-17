#include "point_light_system.h"
#include "../graphics/core.h"

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

    struct PushConstantData {
        glm::vec4 position;
        glm::vec4 color;
    };

    PointLightSystem::PointLightSystem(std::shared_ptr<Device> _device, VkRenderPass renderpass) : device{_device} {
        VkPushConstantRange vk_push_constant_range = {
                .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                .offset = 0,
                .size = sizeof(PushConstantData)
        };

        std::vector<VkDescriptorSetLayout> descriptor_set_layouts = {Core::global_descriptor_set_layout->get_descriptor_set_layout()};

        VkPipelineLayoutCreateInfo vk_pipeline_layout_create_info = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .setLayoutCount = static_cast<uint32_t>(descriptor_set_layouts.size()),
                .pSetLayouts = descriptor_set_layouts.data(),
                .pushConstantRangeCount = 1,
                .pPushConstantRanges = &vk_push_constant_range
        };

        if (vkCreatePipelineLayout(device->vk_device, &vk_pipeline_layout_create_info, nullptr, &vk_pipeline_layout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }

        PipelineConfigInfo pipeline_config = {};
        Pipeline::default_pipeline_config_info(pipeline_config);
        Pipeline::eneble_alpha_blending(pipeline_config);
        pipeline_config.attribute_descriptions.clear();
        pipeline_config.binding_descriptions.clear();
        pipeline_config.vk_renderpass = renderpass;
        pipeline_config.vk_pipeline_layout = vk_pipeline_layout;
        pipeline = std::make_unique<Pipeline>(device, pipeline_config, ShaderFilepaths {
                .vertex = "assets/shaders/point_light.vert",
                .fragment = "assets/shaders/point_light.frag"
        });
    }

    PointLightSystem::~PointLightSystem() {
        vkDestroyPipelineLayout(device->vk_device, vk_pipeline_layout, nullptr);
    }

    void PointLightSystem::render(FrameInfo &frame_info, const std::shared_ptr<Scene> &scene) {
        pipeline->bind(frame_info.command_buffer);

        vkCmdBindDescriptorSets(frame_info.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_pipeline_layout, 0, 1, &frame_info.vk_global_descriptor_set, 0, nullptr);

        scene->registry.each([&](auto entityID) {
            Entity entity = {entityID, scene.get()};
            if (!entity)
                return;

            if (entity.has_component<PointLightComponent>()) {
                auto transform_component = entity.get_component<TransformComponent>();

                PushConstantData push = {
                        .position = glm::vec4(transform_component.translation, 1.0),
                        .color = glm::vec4(entity.get_component<PointLightComponent>().color, 1.0)
                };

                vkCmdPushConstants(frame_info.command_buffer, vk_pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstantData), &push);
                vkCmdDraw(frame_info.command_buffer, 6, 1, 0, 0);
            }
        });
    }
}