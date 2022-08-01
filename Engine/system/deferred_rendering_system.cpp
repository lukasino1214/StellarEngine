#include "deferred_rendering_system.h"
#include "../graphics/core.h"
#include "../data/entity.h"

namespace Engine {
    struct PushConstantData {
        glm::mat4 model_matrix{1.0f};
        glm::mat4 normal_matrix{1.0f};
    };

    DeferredRenderingSystem::DeferredRenderingSystem(std::shared_ptr<Device> _device, u32 _width, u32 _height) : device{_device}, width{_width}, height{_height} {
        sampler = new Sampler(device, {
                .min_filter = Filter::LINEAR,
                .mag_filter = Filter::LINEAR,
                .max_anistropy = 1.0,
        });

        create_images();

        renderpass = new RenderPass(device, {
                { .frameBufferAttachment = image, .loadOp = LoadOp::CLEAR, .storeOp = StoreOp::STORE },              // 0
                { .frameBufferAttachment = depth, .loadOp = LoadOp::CLEAR, .storeOp = StoreOp::DONT_CARE },              // 1
                { .frameBufferAttachment = albedo, .loadOp = LoadOp::CLEAR, .storeOp = StoreOp::STORE },             // 2
                { .frameBufferAttachment = position, .loadOp = LoadOp::CLEAR, .storeOp = StoreOp::STORE },           // 3
                { .frameBufferAttachment = normal, .loadOp = LoadOp::CLEAR, .storeOp = StoreOp::STORE },             // 4
                { .frameBufferAttachment = metallic_roughness, .loadOp = LoadOp::CLEAR, .storeOp = StoreOp::STORE }, // 5
                { .frameBufferAttachment = emissive, .loadOp = LoadOp::CLEAR, .storeOp = StoreOp::STORE },           // 6
            },{
                { .renderTargets = { 1, 2, 3, 4, 5, 6 }, .subpassInputs = {} }, // deferred
                { .renderTargets = { 0 }, .subpassInputs = { 1, 2, 3, 4, 5 } }, // lighting
                { .renderTargets = { 0,6 }, .subpassInputs = { 1 } }, // forward pass
        });

        create_framebuffer();

        // deferred rendering setup
        {
            VkPushConstantRange vk_push_constant_range = {
                    .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                    .offset = 0,
                    .size = sizeof(PushConstantData)
            };

            std::vector<VkDescriptorSetLayout> descriptor_set_layouts = { Core::global_descriptor_set_layout->get_descriptor_set_layout(), Core::pbr_material_descriptor_set_layout->get_descriptor_set_layout() };

            VkPipelineLayoutCreateInfo vk_pipeline_layout_create_info = {
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                    .pNext = nullptr,
                    .flags = 0,
                    .setLayoutCount = static_cast<uint32_t>(descriptor_set_layouts.size()),
                    .pSetLayouts = descriptor_set_layouts.data(),
                    .pushConstantRangeCount = 1,
                    .pPushConstantRanges = &vk_push_constant_range,
            };

            if (vkCreatePipelineLayout(device->vk_device, &vk_pipeline_layout_create_info, nullptr, &vk_deferred_pipeline_layout) != VK_SUCCESS) {
                throw std::runtime_error("failed to create pipeline layout!");
            }

            std::vector<VkPipelineColorBlendAttachmentState> vk_color_blend_attachments {5};

            vk_color_blend_attachments[0] = {
                    .blendEnable = VK_FALSE,
                    .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
                    .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
                    .colorBlendOp = VK_BLEND_OP_ADD,
                    .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
                    .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
                    .alphaBlendOp = VK_BLEND_OP_ADD,
                    .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
            };

            vk_color_blend_attachments[1] = {
                    .blendEnable = VK_FALSE,
                    .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
                    .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
                    .colorBlendOp = VK_BLEND_OP_ADD,
                    .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
                    .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
                    .alphaBlendOp = VK_BLEND_OP_ADD,
                    .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
            };

            vk_color_blend_attachments[2] = {
                    .blendEnable = VK_FALSE,
                    .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
                    .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
                    .colorBlendOp = VK_BLEND_OP_ADD,
                    .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
                    .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
                    .alphaBlendOp = VK_BLEND_OP_ADD,
                    .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
            };

            vk_color_blend_attachments[3] = {
                    .blendEnable = VK_FALSE,
                    .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
                    .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
                    .colorBlendOp = VK_BLEND_OP_ADD,
                    .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
                    .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
                    .alphaBlendOp = VK_BLEND_OP_ADD,
                    .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
            };

            vk_color_blend_attachments[4] = {
                    .blendEnable = VK_FALSE,
                    .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
                    .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
                    .colorBlendOp = VK_BLEND_OP_ADD,
                    .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
                    .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
                    .alphaBlendOp = VK_BLEND_OP_ADD,
                    .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
            };

            VkPipelineColorBlendStateCreateInfo color_blend_info = {
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
                    .pNext = nullptr,
                    .flags = 0,
                    .logicOpEnable = VK_FALSE,
                    .logicOp = VK_LOGIC_OP_COPY,
                    .attachmentCount = static_cast<uint32_t>(vk_color_blend_attachments.size()),
                    .pAttachments = vk_color_blend_attachments.data(),
                    .blendConstants = { 0.0f, 0.0f, 0.0f, 0.0f }
            };

            PipelineConfigInfo pipeline_config = {};
            Pipeline::default_pipeline_config_info(pipeline_config);
            pipeline_config.vk_renderpass = renderpass->vk_renderpass;
            pipeline_config.vk_pipeline_layout = vk_deferred_pipeline_layout;
            pipeline_config.color_blend_info = color_blend_info;
            pipeline_config.subpass = 0;

            deferred_pipeline = std::make_unique<Pipeline>(device, pipeline_config, ShaderFilepaths {
                    .vertex = "assets/shaders/deferred_shader.vert", // TODO: change this
                    .fragment = "assets/shaders/deferred_shader.frag"
            });
        }


        // composition setup
        {
            composition_descriptor_set_layout = DescriptorSetLayout::Builder(device)
                    .add_binding(0, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT)
                    .add_binding(1, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT)
                    .add_binding(2, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT)
                    .add_binding(3, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT)
                    .build_unique();

            std::vector<VkDescriptorSetLayout> descriptor_set_layouts = { Core::global_descriptor_set_layout->get_descriptor_set_layout(), composition_descriptor_set_layout->get_descriptor_set_layout() };

            VkPipelineLayoutCreateInfo vk_pipeline_layout_create_info = {
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                    .pNext = nullptr,
                    .flags = 0,
                    .setLayoutCount = static_cast<uint32_t>(descriptor_set_layouts.size()),
                    .pSetLayouts = descriptor_set_layouts.data(),
                    .pushConstantRangeCount = 0,
                    .pPushConstantRanges = nullptr,
            };

            if (vkCreatePipelineLayout(device->vk_device, &vk_pipeline_layout_create_info, nullptr, &vk_composition_pipeline_layout) != VK_SUCCESS) {
                throw std::runtime_error("failed to create pipeline layout!");
            }

            PipelineConfigInfo pipeline_config = {};
            Pipeline::default_pipeline_config_info(pipeline_config);
            pipeline_config.vk_renderpass = renderpass->vk_renderpass;
            pipeline_config.vk_pipeline_layout = vk_composition_pipeline_layout;
            pipeline_config.subpass = 1;
            pipeline_config.attribute_descriptions.clear();
            pipeline_config.binding_descriptions.clear();

            composition_pipeline = std::make_unique<Pipeline>(device, pipeline_config, ShaderFilepaths {
                    .vertex = "assets/shaders/composition_shader.vert", // TODO: change this
                    .fragment = "assets/shaders/composition_shader.frag"
            });
        }

        {
            VkPushConstantRange vk_push_constant_range = {
                    .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                    .offset = 0,
                    .size = sizeof(PushConstantData)
            };

            std::vector<VkDescriptorSetLayout> descriptor_set_layouts = { Core::global_descriptor_set_layout->get_descriptor_set_layout(), Core::pbr_material_descriptor_set_layout->get_descriptor_set_layout() };

            VkPipelineLayoutCreateInfo vk_pipeline_layout_create_info = {
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                    .pNext = nullptr,
                    .flags = 0,
                    .setLayoutCount = static_cast<uint32_t>(descriptor_set_layouts.size()),
                    .pSetLayouts = descriptor_set_layouts.data(),
                    .pushConstantRangeCount = 1,
                    .pPushConstantRanges = &vk_push_constant_range,
            };

            if (vkCreatePipelineLayout(device->vk_device, &vk_pipeline_layout_create_info, nullptr, &vk_forward_pass_pipeline_layout) != VK_SUCCESS) {
                throw std::runtime_error("failed to create pipeline layout!");
            }

            std::vector<VkPipelineColorBlendAttachmentState> vk_color_blend_attachments {2};

            vk_color_blend_attachments[0] = {
                    .blendEnable = VK_FALSE,
                    .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
                    .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
                    .colorBlendOp = VK_BLEND_OP_ADD,
                    .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
                    .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
                    .alphaBlendOp = VK_BLEND_OP_ADD,
                    .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
            };

            vk_color_blend_attachments[1] = {
                    .blendEnable = VK_FALSE,
                    .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
                    .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
                    .colorBlendOp = VK_BLEND_OP_ADD,
                    .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
                    .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
                    .alphaBlendOp = VK_BLEND_OP_ADD,
                    .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
            };

            VkPipelineColorBlendStateCreateInfo color_blend_info = {
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
                    .pNext = nullptr,
                    .flags = 0,
                    .logicOpEnable = VK_FALSE,
                    .logicOp = VK_LOGIC_OP_COPY,
                    .attachmentCount = static_cast<uint32_t>(vk_color_blend_attachments.size()),
                    .pAttachments = vk_color_blend_attachments.data(),
                    .blendConstants = { 0.0f, 0.0f, 0.0f, 0.0f }
            };

            PipelineConfigInfo pipeline_config = {};
            Pipeline::default_pipeline_config_info(pipeline_config);
            Pipeline::eneble_alpha_blending(pipeline_config);
            pipeline_config.vk_renderpass = renderpass->vk_renderpass;
            pipeline_config.vk_pipeline_layout = vk_forward_pass_pipeline_layout;
            pipeline_config.color_blend_info = color_blend_info;
            pipeline_config.subpass = 2;

            forward_pass_pipeline = std::make_unique<Pipeline>(device, pipeline_config, ShaderFilepaths {
                    .vertex = "assets/shaders/forward_shader.vert", // TODO: change this
                    .fragment = "assets/shaders/forward_shader.frag"
            });
        }

        write_composition_descriptor();

        VkDescriptorImageInfo image_info = {};
        image_info.sampler = sampler->vk_sampler;
        image_info.imageView = image->image_view->vk_image_view;
        image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        DescriptorWriter(*Core::postprocessing_descriptor_set_layout, *Core::global_descriptor_pool)
                .write_image(0, &image_info)
                .build(device, vk_present_descriptor_set);
    }

    void DeferredRenderingSystem::write_composition_descriptor() {
        VkDescriptorImageInfo vk_albedo_descriptor_image_info = {
                .sampler = {},
                .imageView = albedo->image_view->vk_image_view,
                .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        };

        VkDescriptorImageInfo vk_position_descriptor_image_info = {
                .sampler = {},
                .imageView = position->image_view->vk_image_view,
                .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        };

        VkDescriptorImageInfo vk_normal_descriptor_image_info = {
                .sampler = {},
                .imageView = normal->image_view->vk_image_view,
                .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        };

        VkDescriptorImageInfo vk_metallic_roughness_descriptor_image_info = {
                .sampler = {},
                .imageView = metallic_roughness->image_view->vk_image_view,
                .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        };

        DescriptorWriter(*composition_descriptor_set_layout, *Core::global_descriptor_pool)
                .write_image(0, &vk_albedo_descriptor_image_info)
                .write_image(1, &vk_position_descriptor_image_info)
                .write_image(2, &vk_normal_descriptor_image_info)
                .write_image(3, &vk_metallic_roughness_descriptor_image_info)
                .build(device, vk_composition_descriptor_set);
    }

    void DeferredRenderingSystem::resize(u32 _width, u32 _height) {
         width = _width;
         height = _height;
         create_images();
         create_framebuffer();

        write_composition_descriptor();

        VkDescriptorImageInfo image_info = {};
        image_info.sampler = sampler->vk_sampler;
        image_info.imageView = image->image_view->vk_image_view;
        image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        DescriptorWriter(*Core::postprocessing_descriptor_set_layout, *Core::global_descriptor_pool)
                .write_image(0, &image_info)
                .build(device, vk_present_descriptor_set);
    }

    DeferredRenderingSystem::~DeferredRenderingSystem() {
        delete image;
        delete depth;
        delete albedo;
        delete position;
        delete normal;
        delete metallic_roughness;
        delete renderpass;
        delete framebuffer;
        delete sampler;
        delete emissive;

        vkDestroyPipelineLayout(device->vk_device, vk_deferred_pipeline_layout, nullptr);
        vkDestroyPipelineLayout(device->vk_device, vk_composition_pipeline_layout, nullptr);
        vkDestroyPipelineLayout(device->vk_device, vk_forward_pass_pipeline_layout, nullptr);
    }

    void DeferredRenderingSystem::start(FrameInfo &frame_info, const std::shared_ptr<Scene> &scene) {
        renderpass->start(framebuffer, frame_info.command_buffer);

        // deferred pass
        deferred_pipeline->bind(frame_info.command_buffer);
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

                vkCmdPushConstants(frame_info.command_buffer, vk_deferred_pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstantData), &push);

                auto model = entity.get_component<ModelComponent>().model;
                model->bind(frame_info.command_buffer);
                model->draw(frame_info, vk_deferred_pipeline_layout);
            }
        });

        // composition
        std::vector<VkDescriptorSet> vk_composition_descriptor_sets = { frame_info.vk_global_descriptor_set, vk_composition_descriptor_set };

        renderpass->next_subpass(frame_info.command_buffer);
        composition_pipeline->bind(frame_info.command_buffer);
        vkCmdBindDescriptorSets(frame_info.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_composition_pipeline_layout, 0, vk_composition_descriptor_sets.size(), vk_composition_descriptor_sets.data(), 0, nullptr);
        vkCmdDraw(frame_info.command_buffer, 3, 1, 0, 0);

        // forward pass
        renderpass->next_subpass(frame_info.command_buffer);
        forward_pass_pipeline->bind(frame_info.command_buffer);
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

                vkCmdPushConstants(frame_info.command_buffer, vk_forward_pass_pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstantData), &push);

                auto model = entity.get_component<ModelComponent>().model;
                model->bind(frame_info.command_buffer);
                model->draw(frame_info, vk_forward_pass_pipeline_layout);
            }
        });
    }

    void DeferredRenderingSystem::end(FrameInfo &frame_info) {
        renderpass->end(frame_info.command_buffer);
    }

    void DeferredRenderingSystem::create_images() {
        if(!first) {
            vkDeviceWaitIdle(device->vk_device);

            delete image;
            delete depth;
            delete albedo;
            delete position;
            delete normal;
            delete metallic_roughness;
            delete emissive;
        }

        VkFormat fb_depth_format = device->find_supported_format({VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT}, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

        image = new FrameBufferAttachment(device, {
                .format = ImageFormat::B8G8R8A8_UNORM,
                .dimensions = { width, height, 1 },
                .usage = ImageUsageFlagBits::COLOR_ATTACHMENT | ImageUsageFlagBits::SAMPLED,
        });

        depth = new FrameBufferAttachment(device, {
                .format = (ImageFormat)fb_depth_format,
                .dimensions = { width, height, 1 },
                .usage = ImageUsageFlagBits::DEPTH_STENCIL_ATTACHMENT,
        });

        albedo = new FrameBufferAttachment(device, {
                .format = ImageFormat::B8G8R8A8_UNORM,
                .dimensions = { width, height, 1 },
                .usage = ImageUsageFlagBits::COLOR_ATTACHMENT | ImageUsageFlagBits::SAMPLED | ImageUsageFlagBits::INPUT_ATTACHMENT,
        });

        position = new FrameBufferAttachment(device, {
                .format = ImageFormat::R16G16B16A16_SFLOAT,
                .dimensions = { width, height, 1 },
                .usage = ImageUsageFlagBits::COLOR_ATTACHMENT | ImageUsageFlagBits::SAMPLED | ImageUsageFlagBits::INPUT_ATTACHMENT,
        });

        normal = new FrameBufferAttachment(device, {
                .format = ImageFormat::R16G16B16A16_SFLOAT,
                .dimensions = { width, height, 1 },
                .usage = ImageUsageFlagBits::COLOR_ATTACHMENT | ImageUsageFlagBits::SAMPLED | ImageUsageFlagBits::INPUT_ATTACHMENT,
        });

        metallic_roughness = new FrameBufferAttachment(device, {
                .format = ImageFormat::R16G16B16A16_SFLOAT,
                .dimensions = { width, height, 1 },
                .usage = ImageUsageFlagBits::COLOR_ATTACHMENT | ImageUsageFlagBits::SAMPLED | ImageUsageFlagBits::INPUT_ATTACHMENT,
        });

        emissive = new FrameBufferAttachment(device, {
                .format = ImageFormat::B8G8R8A8_UNORM,
                .dimensions = { width, height, 1 },
                .usage = ImageUsageFlagBits::COLOR_ATTACHMENT | ImageUsageFlagBits::SAMPLED,
        });
    }

    void DeferredRenderingSystem::create_framebuffer() {
        if(!first) {
            delete framebuffer;
        }

        framebuffer = new Framebuffer(device, renderpass->vk_renderpass, { image, depth, albedo, position, normal, metallic_roughness, emissive });

        first = false;
    }
}