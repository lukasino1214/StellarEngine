#include "rendering_system.h"
#include "../graphics/core.h"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace Engine {
    struct PushConstantData {
        glm::mat4 model_matrix{1.0f};
        glm::mat4 normal_matrix{1.0f};
    };

    RenderSystem::RenderSystem(std::shared_ptr<Device> _device, VkRenderPass renderpass) : device{_device} {
        VkPushConstantRange vk_push_constant_range = {
                .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                .offset = 0,
                .size = sizeof(PushConstantData)
        };

        std::vector<VkDescriptorSetLayout> descriptor_set_layouts = {Core::global_descriptor_set_layout->get_descriptor_set_layout(), Core::pbr_material_descriptor_set_layout->get_descriptor_set_layout(), Core::shadow_descriptor_set_layout->get_descriptor_set_layout()};

        VkPipelineLayoutCreateInfo vk_pipeline_layout_create_info = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .setLayoutCount = static_cast<uint32_t>(descriptor_set_layouts.size()),
                .pSetLayouts = descriptor_set_layouts.data(),
                .pushConstantRangeCount = 1,
                .pPushConstantRanges = &vk_push_constant_range,
        };

        if (vkCreatePipelineLayout(device->vk_device, &vk_pipeline_layout_create_info, nullptr, &vk_pipeline_layout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }

        PipelineConfigInfo pipeline_config = {};
        Pipeline::default_pipeline_config_info(pipeline_config);
        pipeline_config.vk_renderpass = renderpass;
        pipeline_config.vk_pipeline_layout = vk_pipeline_layout;

        pipeline = std::make_unique<Pipeline>(device, pipeline_config, ShaderFilepaths {
                .vertex = "assets/shaders/simple_shader.vert",
                .fragment = "assets/shaders/simple_shader.frag"
        });
    }

    RenderSystem::~RenderSystem() {
        vkDestroyPipelineLayout(device->vk_device, vk_pipeline_layout, nullptr);
    }

    void RenderSystem::render(FrameInfo &frame_info, const std::shared_ptr<Scene> &scene) {
        pipeline->bind(frame_info.command_buffer);

        vkCmdBindDescriptorSets(frame_info.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_pipeline_layout, 2, 1, &frame_info.vk_shadow_descriptor_set, 0, nullptr);

        scene->registry.each([&](auto entityID) {
            Entity entity = {entityID, scene.get()};
            if (!entity)
                return;

            if (entity.has_component<ModelComponent>()) {
                auto transform_component = entity.get_component<TransformComponent>();

                PushConstantData push = {
                        .model_matrix = transform_component.calculate_matrix(),
                        .normal_matrix = transform_component.calculate_normal_matrix()
                };

                vkCmdPushConstants(frame_info.command_buffer, vk_pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstantData), &push);

                auto model = entity.get_component<ModelComponent>().model;
                model->bind(frame_info.command_buffer);
                model->draw(frame_info, vk_pipeline_layout);
            }
        });
    }
}  // namespace lve
