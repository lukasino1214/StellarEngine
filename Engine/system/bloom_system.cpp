#include "bloom_system.h"

#include "../graphics/core.h"

namespace Engine {
    struct PushConstantData {
        glm::ivec2 src_resolution{};
        i32 mip_level = 0;
        f32 filter_radius;
    };

    BloomRenderingSystem::BloomRenderingSystem(std::shared_ptr<Device> _device, u32 _width, u32 _height) : device{_device}, width{_width}, height{_height} {
        u32 mip_width = width;
        u32 mip_height = height;

        sampler = new Sampler(device, {
                .min_filter = Filter::LINEAR,
                .mag_filter = Filter::LINEAR,
                .max_anistropy = 1.0,
        });

        FrameBufferAttachment* emissive_framebuffer_attachement = new FrameBufferAttachment(device, {
                .format = ImageFormat::B8G8R8A8_UNORM,
                .dimensions = { mip_width, mip_height, 1 },
                .usage = ImageUsageFlagBits::COLOR_ATTACHMENT | ImageUsageFlagBits::SAMPLED
        });

        {
            VkDescriptorImageInfo image_info = {};
            image_info.sampler = sampler->vk_sampler;
            image_info.imageView = emissive_framebuffer_attachement->image_view->vk_image_view;
            image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            DescriptorWriter(*Core::postprocessing_descriptor_set_layout, *Core::global_descriptor_pool)
                    .write_image(0, &image_info)
                    .build(device, emissive_set);
        }

        vk_descriptor_sets.resize(mip_levels);

        for(isize i = 0; i < mip_levels; i++) {
            mip_width = mip_width / 2;
            mip_height = mip_height / 2;
            sizes.push_back({ mip_width, mip_height });

            FrameBufferAttachment* attachment = new FrameBufferAttachment(device, {
                    .format = ImageFormat::B8G8R8A8_UNORM,
                    .dimensions = { mip_width, mip_height, 1 },
                    .usage = ImageUsageFlagBits::COLOR_ATTACHMENT | ImageUsageFlagBits::SAMPLED
            });

            VkDescriptorImageInfo image_info = {};
            image_info.sampler = sampler->vk_sampler;
            image_info.imageView = attachment->image_view->vk_image_view;
            image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            DescriptorWriter(*Core::postprocessing_descriptor_set_layout, *Core::global_descriptor_pool)
                    .write_image(0, &image_info)
                    .build(device, vk_descriptor_sets[i]);

            framebuffer_attachements.push_back(std::move(attachment));
        }

        renderpass = new RenderPass(device, {
                { .frameBufferAttachment = framebuffer_attachements[0], .loadOp = LoadOp::CLEAR, .storeOp = StoreOp::STORE },              // 0
        },{
                { .renderTargets = { 0 }, .subpassInputs = {} },
        });

        Framebuffer* emissive_framebuffer = new Framebuffer(device, renderpass->vk_renderpass, { emissive_framebuffer_attachement });

        for(isize i = 0; i < mip_levels; i++) {
            framebuffers.push_back(new Framebuffer(device, renderpass->vk_renderpass, { framebuffer_attachements[i] }));
        }

        VkPushConstantRange vk_push_constant_range = {
                .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                .offset = 0,
                .size = sizeof(PushConstantData)
        };

        std::vector<VkDescriptorSetLayout> descriptor_set_layouts = { Core::postprocessing_descriptor_set_layout->get_descriptor_set_layout() };

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
        pipeline_config.vk_renderpass = renderpass->vk_renderpass;
        pipeline_config.vk_pipeline_layout = vk_pipeline_layout;
        pipeline_config.attribute_descriptions.clear();
        pipeline_config.binding_descriptions.clear();
        pipeline_config.subpass = 0;

        downsample_pipeline = std::make_unique<Pipeline>(device, pipeline_config, ShaderFilepaths {
                .vertex = "assets/shaders/downsample.vert",
                .fragment = "assets/shaders/downsample.frag"
        });

        upsample_pipeline = std::make_unique<Pipeline>(device, pipeline_config, ShaderFilepaths {
                .vertex = "assets/shaders/upsample.vert",
                .fragment = "assets/shaders/upsample.frag"
        });
    }

    BloomRenderingSystem::~BloomRenderingSystem() {
        delete renderpass;
        delete sampler;
        for(isize i = 0; i < mip_levels; i++) {
            delete framebuffer_attachements[i];
            delete framebuffers[i];
        }

        delete emissive_framebuffer;
        delete emissive_framebuffer_attachement;
    }

    void BloomRenderingSystem::render(FrameInfo frame_info, VkDescriptorSet vk_emissive_descriptor_set) {
        PushConstantData push{};
        push.filter_radius = 0.005;
        push.src_resolution = { 1280, 720 };

        renderpass->start(framebuffers[0], frame_info.command_buffer);
        downsample_pipeline->bind(frame_info.command_buffer);

        vkCmdPushConstants(frame_info.command_buffer, vk_pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstantData), &push);
        vkCmdBindDescriptorSets(frame_info.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_pipeline_layout, 0, 1, &vk_emissive_descriptor_set, 0, nullptr);

        vkCmdDraw(frame_info.command_buffer, 3, 1, 0, 0);
        renderpass->end(frame_info.command_buffer);


        push.src_resolution = { 640, 360 };
        renderpass->start(framebuffers[1], frame_info.command_buffer);
        downsample_pipeline->bind(frame_info.command_buffer);

        vkCmdPushConstants(frame_info.command_buffer, vk_pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstantData), &push);
        vkCmdBindDescriptorSets(frame_info.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_pipeline_layout, 0, 1, &vk_descriptor_sets[0], 0, nullptr);

        vkCmdDraw(frame_info.command_buffer, 3, 1, 0, 0);
        renderpass->end(frame_info.command_buffer);


        push.src_resolution = { 320, 180 };
        renderpass->start(framebuffers[0], frame_info.command_buffer);
        upsample_pipeline->bind(frame_info.command_buffer);

        vkCmdPushConstants(frame_info.command_buffer, vk_pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstantData), &push);
        vkCmdBindDescriptorSets(frame_info.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_pipeline_layout, 0, 1, &vk_descriptor_sets[1], 0, nullptr);

        vkCmdDraw(frame_info.command_buffer, 3, 1, 0, 0);
        renderpass->end(frame_info.command_buffer);


        push.src_resolution = { 640, 360 };
        renderpass->start(emissive_framebuffer, frame_info.command_buffer);
        upsample_pipeline->bind(frame_info.command_buffer);

        vkCmdPushConstants(frame_info.command_buffer, vk_pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstantData), &push);
        vkCmdBindDescriptorSets(frame_info.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_pipeline_layout, 0, 1, &vk_descriptor_sets[0], 0, nullptr);

        vkCmdDraw(frame_info.command_buffer, 3, 1, 0, 0);
        renderpass->end(frame_info.command_buffer);

        /*for(isize i = 0; i < mip_levels; i++) {
            push.mip_level = i;
            push.src_resolution = sizes[i];

            renderpass->start(framebuffers[i], frame_info.command_buffer);
            downsample_pipeline->bind(frame_info.command_buffer);

            vkCmdPushConstants(frame_info.command_buffer, vk_pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstantData), &push);
            vkCmdBindDescriptorSets(frame_info.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_pipeline_layout, 0, 1, &vk_descriptor_sets[i], 0, nullptr);

            vkCmdDraw(frame_info.command_buffer, 3, 1, 0, 0);
            renderpass->end(frame_info.command_buffer);
        }

        for(isize i = mip_levels; i > 0; i--) {
            push.mip_level = i;
            push.src_resolution = sizes[i];

            renderpass->start(framebuffers[i], frame_info.command_buffer);
            upsample_pipeline->bind(frame_info.command_buffer);

            vkCmdPushConstants(frame_info.command_buffer, vk_pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstantData), &push);
            vkCmdBindDescriptorSets(frame_info.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_pipeline_layout, 0, 1, &vk_descriptor_sets[i], 0, nullptr);

            vkCmdDraw(frame_info.command_buffer, 3, 1, 0, 0);
            renderpass->end(frame_info.command_buffer);
        }*/
    }
}
