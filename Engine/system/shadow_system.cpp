#include "shadow_system.h"

#include <utility>
#include "../graphics/core.h"

namespace Engine {
    struct PushConstantData {
        glm::mat4 model_matrix{1.0f};
        glm::mat4 normal_matrix{1.0f};
    };

    ShadowSystem::ShadowSystem(std::shared_ptr<Device> _device) : device{std::move(_device)} {

        depth = new FrameBufferAttachment(device, {
                .format = ImageFormat::D16_UNORM,
                .dimensions = { width, height, 1 },
                .usage = ImageUsageFlagBits::DEPTH_STENCIL_ATTACHMENT | ImageUsageFlagBits::SAMPLED,
                .final_layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
        });

        sampler = new Sampler(device, {
                .min_filter = Filter::LINEAR,
                .mag_filter = Filter::LINEAR,
                .max_anistropy = 1.0,
                .address_mode = SamplerAddressMode::CLAMP_TO_BORDER
        });

        renderpass = new RenderPass(device, {
            { .frameBufferAttachment = depth, .loadOp = LoadOp::CLEAR, .storeOp = StoreOp::STORE }
        }, { { .renderTargets = { 0 } } });

        framebuffer = new Framebuffer(device, renderpass->vk_renderpass, { depth });

        VkPushConstantRange vk_push_constant_range = {
            .stageFlags =VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            .offset = 0,
            .size = sizeof(PushConstantData)
        };

        std::vector<VkDescriptorSetLayout> descriptor_set_layouts = {Core::global_descriptor_set_layout->get_descriptor_set_layout(), Core::pbr_material_descriptor_set_layout->get_descriptor_set_layout()};

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
        pipeline_config.vk_renderpass = renderpass->vk_renderpass;
        pipeline_config.vk_pipeline_layout = vk_pipeline_layout;
        pipeline = std::make_unique<Pipeline>(device, pipeline_config, ShaderFilepaths {
            .vertex = "assets/shaders/shadow.vert",
        });
    }

    void ShadowSystem::render(FrameInfo &frame_info, std::shared_ptr<Scene> scene) {
        renderpass->start(framebuffer, frame_info.command_buffer);

        vkCmdSetDepthBias(frame_info.command_buffer, 1.25f, 0.0f, 1.75f);

        pipeline->bind(frame_info.command_buffer);
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

        renderpass->end(frame_info.command_buffer);
    }

    ShadowSystem::~ShadowSystem() {
        delete depth;
        delete sampler;

        delete renderpass;

        vkDestroyPipelineLayout(device->vk_device, vk_pipeline_layout, nullptr);
    }
}