#include "grid_system.h"
#include "../graphics/core.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace Engine {
    GridSystem::GridSystem(std::shared_ptr<Device> _device, VkRenderPass renderpass) : device{_device} {
        std::vector<VkDescriptorSetLayout> descriptor_set_layouts = {Core::global_descriptor_set_layout->get_descriptor_set_layout()};

        VkPipelineLayoutCreateInfo vk_pipeline_create_info = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .setLayoutCount = static_cast<uint32_t>(descriptor_set_layouts.size()),
                .pSetLayouts = descriptor_set_layouts.data(),
                .pushConstantRangeCount = 0,
                .pPushConstantRanges = nullptr,
        };

        if (vkCreatePipelineLayout(device->vk_device, &vk_pipeline_create_info, nullptr, &vk_pipeline_layout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }

        PipelineConfigInfo pipeline_config = {};
        Pipeline::default_pipeline_config_info(pipeline_config);
        pipeline_config.attribute_descriptions.clear();
        pipeline_config.binding_descriptions.clear();
        pipeline_config.vk_renderpass = renderpass;
        pipeline_config.vk_pipeline_layout = vk_pipeline_layout;
        pipeline_config.color_blend_attachment.blendEnable = VK_TRUE;
        pipeline_config.color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        pipeline_config.color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;

        pipeline = std::make_unique<Pipeline>(device, pipeline_config, ShaderFilepaths {
                .vertex = "assets/shaders/grid.vert",
                .fragment = "assets/shaders/grid.frag"
        });
    }

    GridSystem::~GridSystem() {
        vkDestroyPipelineLayout(device->vk_device, vk_pipeline_layout, nullptr);
    }

    void GridSystem::render(FrameInfo &frame_info) {
        pipeline->bind(frame_info.command_buffer);

        vkCmdBindDescriptorSets(frame_info.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_pipeline_layout, 0, 1, &frame_info.vk_global_descriptor_set, 0, nullptr);
        vkCmdDraw(frame_info.command_buffer, 6, 1, 0, 0);
    }
}