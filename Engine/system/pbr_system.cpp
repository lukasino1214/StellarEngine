#include "pbr_system.h"

#include "../graphics/texture.h"
#include "../graphics/core.h"

#include <glm/gtx/quaternion.hpp>

#include <stb_image.h>

namespace Engine {
    struct SkyboxPushConstantData {
        glm::mat4 model_matrix;
    };

    PBRSystem::PBRSystem(std::shared_ptr<Device> _device, VkRenderPass renderpass) : device{_device} {
        sampler = new Sampler(device, {
                .min_filter = Filter::LINEAR,
                .mag_filter = Filter::LINEAR,
                .address_mode = SamplerAddressMode::CLAMP_TO_EDGE,
                .max_anistropy = 4.0
        });

        cube = std::make_unique<Model>(device, "assets/models/cube.gltf");

        generate_BRDFLUT();
        generate_environment_map();
        generate_irradiance_cube();
        generate_prefiltered_cube();

        std::vector<VkDescriptorSetLayout> descriptor_set_layouts = { Core::global_descriptor_set_layout->get_descriptor_set_layout(), Core::postprocessing_descriptor_set_layout->get_descriptor_set_layout() };

        VkPushConstantRange vk_push_constant_range = {
                .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                .offset = 0,
                .size = sizeof(SkyboxPushConstantData)
        };

        VkPipelineLayoutCreateInfo vk_pipeline_create_info = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .setLayoutCount = static_cast<uint32_t>(descriptor_set_layouts.size()),
                .pSetLayouts = descriptor_set_layouts.data(),
                .pushConstantRangeCount = 1,
                .pPushConstantRanges = &vk_push_constant_range,
        };

        if (vkCreatePipelineLayout(device->vk_device, &vk_pipeline_create_info, nullptr, &vk_skybox_pipeline_layout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }

        PipelineConfigInfo pipeline_config = {};
        Pipeline::default_pipeline_config_info(pipeline_config);
        pipeline_config.depth_stencil_info.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
        pipeline_config.vk_renderpass = renderpass;
        pipeline_config.vk_pipeline_layout = vk_skybox_pipeline_layout;

        skybox_pipeline = std::make_unique<Pipeline>(device, pipeline_config, ShaderFilepaths {
                .vertex = "assets/shaders/skybox.vert",
                .fragment = "assets/shaders/skybox.frag"
        });
    }

    PBRSystem::~PBRSystem() {
        delete sampler;

        delete BRDFLUT;

        delete hdr_image;
        delete hdr_image_view;

        delete environment_cube_image;
        delete environment_cube_image_view;

        delete irradiance_cube_image;
        delete irradiance_cube_image_view;

        delete prefiltered_cube_image;
        delete prefiltered_cube_image_view;

        vkDestroyPipelineLayout(device->vk_device, vk_skybox_pipeline_layout, nullptr);
    }

    void PBRSystem::render_skybox(FrameInfo frame_info) {
        skybox_pipeline->bind(frame_info.command_buffer);

        SkyboxPushConstantData push{};
        push.model_matrix = glm::mat4(glm::mat3(frame_info.ubo.view_matrix));

        std::vector<VkDescriptorSet> sets { frame_info.vk_global_descriptor_set, prefiltered_cube_set };

        vkCmdPushConstants(frame_info.command_buffer, vk_skybox_pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(SkyboxPushConstantData), &push);
        vkCmdBindDescriptorSets(frame_info.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_skybox_pipeline_layout, 0, sets.size(), sets.data(), 0, nullptr);
        cube->bind(frame_info.command_buffer);
        cube->draw(frame_info.command_buffer);
    }

    void PBRSystem::generate_BRDFLUT() {
        BRDFLUT = new FrameBufferAttachment(device, {
                .format = ImageFormat::R16G16_SFLOAT,
                .dimensions = { 512, 512, 1 },
                .usage = ImageUsageFlagBits::COLOR_ATTACHMENT | ImageUsageFlagBits::SAMPLED,
        });

        RenderPass renderpass(device, {
                { .frameBufferAttachment = BRDFLUT, .loadOp = LoadOp::CLEAR, .storeOp = StoreOp::STORE }
        }, {
                { .renderTargets = { 0 }, .subpassInputs = {} }
        });

        Framebuffer framebuffer(device, renderpass.vk_renderpass, { BRDFLUT });

        VkPipelineLayout vk_pipeline_layout;

        VkPipelineLayoutCreateInfo vk_pipeline_create_info = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .setLayoutCount = 0,
                .pSetLayouts = nullptr,
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
        pipeline_config.vk_renderpass = renderpass.vk_renderpass;
        pipeline_config.vk_pipeline_layout = vk_pipeline_layout;

        Pipeline pipeline(device, pipeline_config, {
                .vertex = "assets/shaders/genbrdflut.vert",
                .fragment = "assets/shaders/genbrdflut.frag"
        });

        VkCommandBuffer command_buffer = device->begin_single_time_command_buffer();
        renderpass.start(&framebuffer, command_buffer);
        pipeline.bind(command_buffer);
        vkCmdDraw(command_buffer, 3, 1, 0, 0);
        renderpass.end(command_buffer);
        device->end_single_time_command_buffer(command_buffer);

        vkDeviceWaitIdle(device->vk_device); // Just in case
        vkDestroyPipelineLayout(device->vk_device, vk_pipeline_layout, nullptr);

        VkDescriptorImageInfo image_info = {};
        image_info.sampler = sampler->vk_sampler;
        image_info.imageView = BRDFLUT->image_view->vk_image_view;
        image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        DescriptorWriter(*Core::postprocessing_descriptor_set_layout, *Core::global_descriptor_pool)
                .write_image(0, &image_info)
                .build(device, vk_BRDFLUT_descriptor_set);
    }

    void PBRSystem::generate_environment_map() {
        int width, height, channels, m_BytesPerPixel;

        stbi_set_flip_vertically_on_load(1);
        void* data = stbi_load_16("assets/newport_loft.hdr", &width, &height, &channels, STBI_rgb_alpha);

        u32 mip_levels_hdr = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;

        Buffer stagingBuffer{device, 4 * sizeof(u16), static_cast<uint32_t>(width * height), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};

        stagingBuffer.map();
        stagingBuffer.write_to_buffer(data);


        hdr_image = new Image(device, {
                .format = ImageFormat::R16G16B16A16_UNORM,
                .dimensions = { width, height, 1 },
                .usage = ImageUsageFlagBits::TRANSFER_SRC | ImageUsageFlagBits::TRANSFER_DST | ImageUsageFlagBits::SAMPLED,
                .mip_levels = mip_levels_hdr
        });

        hdr_image->transition_image_layout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        device->copy_buffer_to_image(stagingBuffer.get_buffer(), hdr_image->vk_image, static_cast<uint>(width), static_cast<uint>(height), 1);
        hdr_image->generate_mipmaps();

        hdr_image_view = new ImageView(device, {
                .format = ImageFormat::R16G16B16A16_UNORM,
                .mip_levels = mip_levels_hdr,
                .image = hdr_image
        });

        {
            VkDescriptorImageInfo image_info = {};
            image_info.sampler = sampler->vk_sampler;
            image_info.imageView = hdr_image_view->vk_image_view;
            image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            DescriptorWriter(*Core::postprocessing_descriptor_set_layout, *Core::global_descriptor_pool)
                    .write_image(0, &image_info)
                    .build(device, hdr_set);
        }

        stbi_image_free(data);

        const ImageFormat format = ImageFormat::R32G32B32A32_SFLOAT;
        const glm::vec3 dim = { 512, 512, 1 };
        const u32 mip_levels = static_cast<uint32_t>(floor(log2(dim.x))) + 1;

        environment_cube_image = new Image(device, {
            .format = format,
            .type = ImageType::TYPE_2D,
            .dimensions = dim,
            .usage = ImageUsageFlagBits::SAMPLED | ImageUsageFlagBits::TRANSFER_DST,
            .mip_levels = mip_levels,
            .array_layers = 6,
            .flags = ImageCreateFlagBits::CUBE_COMPATIBLE
        });

        environment_cube_image_view = new ImageView(device, {
            .type = ImageViewType::TYPE_CUBE,
            .format = format,
            .mip_levels = mip_levels,
            .array_layers = 6,
            .image = environment_cube_image
        });

        FrameBufferAttachment offscreen(device, {
            .format = format,
            .dimensions = dim,
            .usage = ImageUsageFlagBits::COLOR_ATTACHMENT | ImageUsageFlagBits::TRANSFER_SRC,
            .final_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        });

        RenderPass renderpass(device, {
                { .frameBufferAttachment = &offscreen, .loadOp = LoadOp::CLEAR, .storeOp = StoreOp::STORE }
            }, {
                { .renderTargets = { 0 }, .subpassInputs = {} }
        });

        Framebuffer framebuffer(device, renderpass.vk_renderpass, { &offscreen });
        offscreen.image->transition_image_layout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);


        struct PushConstantData {
            glm::mat4 mvp;
        } push_constant_data;

        VkPushConstantRange vk_push_constant_range = {
                .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                .offset = 0,
                .size = sizeof(PushConstantData)
        };

        std::vector<VkDescriptorSetLayout> descriptor_set_layouts = {Core::postprocessing_descriptor_set_layout->get_descriptor_set_layout()};

        VkPipelineLayout vk_pipeline_layout;

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
        pipeline_config.vk_renderpass = renderpass.vk_renderpass;
        pipeline_config.vk_pipeline_layout = vk_pipeline_layout;

        Pipeline pipeline(device, pipeline_config, {
                .vertex = "assets/shaders/equirectangular_to_cubemap.vert",
                .fragment = "assets/shaders/equirectangular_to_cubemap.frag"
        });

        std::vector<glm::mat4> matrices = {
                // POSITIVE_X
                glm::rotate(glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
                // NEGATIVE_X
                glm::rotate(glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f)), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
                // POSITIVE_Y
                glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
                // NEGATIVE_Y
                glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
                // POSITIVE_Z
                glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
                // NEGATIVE_Z
                glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
        };

        VkCommandBuffer command_buffer = device->begin_single_time_command_buffer();

        environment_cube_image->transition_image_layout(command_buffer, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        for(isize j = 0; j < mip_levels; j++) {
            for(isize i = 0; i < 6; i++) {
                renderpass.start(&framebuffer, command_buffer, {dim.x * std::pow(0.5f, j), dim.y * std::pow(0.5f, j)});

                push_constant_data.mvp = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 512.0f) * matrices[i];

                pipeline.bind(command_buffer);
                vkCmdPushConstants(command_buffer, vk_pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstantData), &push_constant_data);
                vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_pipeline_layout, 0, 1, &hdr_set, 0, nullptr);
                cube->bind(command_buffer);
                cube->draw(command_buffer);
                renderpass.end(command_buffer);
                offscreen.image->transition_image_layout(command_buffer, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

                VkImageCopy copyRegion = {};
                copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                copyRegion.srcSubresource.baseArrayLayer = 0;
                copyRegion.srcSubresource.mipLevel = 0;
                copyRegion.srcSubresource.layerCount = 1;
                copyRegion.srcOffset = { 0, 0, 0 };

                copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                copyRegion.dstSubresource.baseArrayLayer = i;
                copyRegion.dstSubresource.mipLevel = j;
                copyRegion.dstSubresource.layerCount = 1;
                copyRegion.dstOffset = { 0, 0, 0 };

                copyRegion.extent.width = dim.x * std::pow(0.5f, j);
                copyRegion.extent.height = dim.y * std::pow(0.5f, j);
                copyRegion.extent.depth = 1;

                vkCmdCopyImage(
                        command_buffer,
                        offscreen.image->vk_image,
                        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                        environment_cube_image->vk_image,
                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                        1,
                        &copyRegion);

                offscreen.image->transition_image_layout(command_buffer, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
            }
        }

        environment_cube_image->transition_image_layout(command_buffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        device->end_single_time_command_buffer(command_buffer);

        {
            VkDescriptorImageInfo image_info = {};
            image_info.sampler = sampler->vk_sampler;
            image_info.imageView = environment_cube_image_view->vk_image_view;
            image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            DescriptorWriter(*Core::postprocessing_descriptor_set_layout, *Core::global_descriptor_pool)
                    .write_image(0, &image_info)
                    .build(device, environment_map_set);
        }

        vkDeviceWaitIdle(device->vk_device); // Just in case
        vkDestroyPipelineLayout(device->vk_device, vk_pipeline_layout, nullptr);
    }

    void PBRSystem::generate_irradiance_cube() {
        const ImageFormat format = ImageFormat::R32G32B32A32_SFLOAT;
        const glm::vec3 dim = { 32, 32, 1 };
        const u32 mip_levels = static_cast<uint32_t>(floor(log2(dim.x))) + 1;

        irradiance_cube_image = new Image(device, {
                .format = format,
                .type = ImageType::TYPE_2D,
                .dimensions = dim,
                .usage = ImageUsageFlagBits::SAMPLED | ImageUsageFlagBits::TRANSFER_DST,
                .mip_levels = mip_levels,
                .array_layers = 6,
                .flags = ImageCreateFlagBits::CUBE_COMPATIBLE
        });

        irradiance_cube_image_view = new ImageView(device, {
                .type = ImageViewType::TYPE_CUBE,
                .format = format,
                .mip_levels = mip_levels,
                .array_layers = 6,
                .image = irradiance_cube_image
        });

        FrameBufferAttachment offscreen(device, {
                .format = format,
                .dimensions = dim,
                .usage = ImageUsageFlagBits::COLOR_ATTACHMENT | ImageUsageFlagBits::TRANSFER_SRC,
                .final_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        });

        RenderPass renderpass(device, {
                { .frameBufferAttachment = &offscreen, .loadOp = LoadOp::CLEAR, .storeOp = StoreOp::STORE }
        }, {
                                      { .renderTargets = { 0 }, .subpassInputs = {} }
                              });

        Framebuffer framebuffer(device, renderpass.vk_renderpass, { &offscreen });
        offscreen.image->transition_image_layout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);


        struct PushConstantData {
            glm::mat4 mvp;
            float deltaPhi = (2.0f * float(M_PI)) / 180.0f;
            float deltaTheta = (0.5f * float(M_PI)) / 64.0f;
        } push_constant_data;

        VkPushConstantRange vk_push_constant_range = {
                .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                .offset = 0,
                .size = sizeof(PushConstantData)
        };

        std::vector<VkDescriptorSetLayout> descriptor_set_layouts = {Core::postprocessing_descriptor_set_layout->get_descriptor_set_layout()};

        VkPipelineLayout vk_pipeline_layout;

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
        pipeline_config.vk_renderpass = renderpass.vk_renderpass;
        pipeline_config.vk_pipeline_layout = vk_pipeline_layout;

        Pipeline pipeline(device, pipeline_config, {
                .vertex = "assets/shaders/irradiancecube.vert",
                .fragment = "assets/shaders/irradiancecube.frag"
        });

        std::vector<glm::mat4> matrices = {
                // POSITIVE_X
                glm::rotate(glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
                // NEGATIVE_X
                glm::rotate(glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f)), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
                // POSITIVE_Y
                glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
                // NEGATIVE_Y
                glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
                // POSITIVE_Z
                glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
                // NEGATIVE_Z
                glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
        };

        VkCommandBuffer command_buffer = device->begin_single_time_command_buffer();

        irradiance_cube_image->transition_image_layout(command_buffer, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        for(isize j = 0; j < mip_levels; j++) {
            for(isize i = 0; i < 6; i++) {
                renderpass.start(&framebuffer, command_buffer, {dim.x * std::pow(0.5f, j), dim.y * std::pow(0.5f, j)});

                push_constant_data.mvp = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 512.0f) * matrices[i];

                vkCmdPushConstants(command_buffer, vk_pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstantData), &push_constant_data);

                pipeline.bind(command_buffer);
                vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_pipeline_layout, 0, 1, &environment_map_set, 0, nullptr);
                cube->bind(command_buffer);
                cube->draw(command_buffer);
                renderpass.end(command_buffer);
                offscreen.image->transition_image_layout(command_buffer, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

                VkImageCopy copyRegion = {};
                copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                copyRegion.srcSubresource.baseArrayLayer = 0;
                copyRegion.srcSubresource.mipLevel = 0;
                copyRegion.srcSubresource.layerCount = 1;
                copyRegion.srcOffset = { 0, 0, 0 };

                copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                copyRegion.dstSubresource.baseArrayLayer = i;
                copyRegion.dstSubresource.mipLevel = j;
                copyRegion.dstSubresource.layerCount = 1;
                copyRegion.dstOffset = { 0, 0, 0 };

                copyRegion.extent.width = dim.x * std::pow(0.5f, j);
                copyRegion.extent.height = dim.y * std::pow(0.5f, j);
                copyRegion.extent.depth = 1;

                vkCmdCopyImage(
                        command_buffer,
                        offscreen.image->vk_image,
                        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                        irradiance_cube_image->vk_image,
                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                        1,
                        &copyRegion);

                offscreen.image->transition_image_layout(command_buffer, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
            }
        }

        irradiance_cube_image->transition_image_layout(command_buffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        device->end_single_time_command_buffer(command_buffer);

        {
            VkDescriptorImageInfo image_info = {};
            image_info.sampler = sampler->vk_sampler;
            image_info.imageView = irradiance_cube_image_view->vk_image_view;
            image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            DescriptorWriter(*Core::postprocessing_descriptor_set_layout, *Core::global_descriptor_pool)
                    .write_image(0, &image_info)
                    .build(device, irradiance_cube_set);
        }

        vkDeviceWaitIdle(device->vk_device); // Just in case
        vkDestroyPipelineLayout(device->vk_device, vk_pipeline_layout, nullptr);
    }

    void PBRSystem::generate_prefiltered_cube() {
        const ImageFormat format = ImageFormat::R32G32B32A32_SFLOAT;
        const glm::vec3 dim = { 512, 512, 1 };
        const u32 mip_levels = static_cast<uint32_t>(floor(log2(dim.x))) + 1;

        prefiltered_cube_image = new Image(device, {
                .format = format,
                .type = ImageType::TYPE_2D,
                .dimensions = dim,
                .usage = ImageUsageFlagBits::SAMPLED | ImageUsageFlagBits::TRANSFER_DST,
                .mip_levels = mip_levels,
                .array_layers = 6,
                .flags = ImageCreateFlagBits::CUBE_COMPATIBLE
        });

        prefiltered_cube_image_view = new ImageView(device, {
                .type = ImageViewType::TYPE_CUBE,
                .format = format,
                .mip_levels = mip_levels,
                .array_layers = 6,
                .image = prefiltered_cube_image
        });

        FrameBufferAttachment offscreen(device, {
                .format = format,
                .dimensions = dim,
                .usage = ImageUsageFlagBits::COLOR_ATTACHMENT | ImageUsageFlagBits::TRANSFER_SRC,
                .final_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        });

        RenderPass renderpass(device, {
                { .frameBufferAttachment = &offscreen, .loadOp = LoadOp::CLEAR, .storeOp = StoreOp::STORE }
        }, {
                                      { .renderTargets = { 0 }, .subpassInputs = {} }
                              });

        Framebuffer framebuffer(device, renderpass.vk_renderpass, { &offscreen });
        offscreen.image->transition_image_layout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);


        struct PushConstantData {
            glm::mat4 mvp;
            float roughness;
            uint32_t numSamples = 32u;
        } push_constant_data;

        VkPushConstantRange vk_push_constant_range = {
                .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                .offset = 0,
                .size = sizeof(PushConstantData)
        };

        std::vector<VkDescriptorSetLayout> descriptor_set_layouts = {Core::postprocessing_descriptor_set_layout->get_descriptor_set_layout()};

        VkPipelineLayout vk_pipeline_layout;

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
        pipeline_config.vk_renderpass = renderpass.vk_renderpass;
        pipeline_config.vk_pipeline_layout = vk_pipeline_layout;

        Pipeline pipeline(device, pipeline_config, {
                .vertex = "assets/shaders/prefilterenv.vert",
                .fragment = "assets/shaders/prefilterenv.frag"
        });

        std::vector<glm::mat4> matrices = {
                // POSITIVE_X
                glm::rotate(glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
                // NEGATIVE_X
                glm::rotate(glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f)), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
                // POSITIVE_Y
                glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
                // NEGATIVE_Y
                glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
                // POSITIVE_Z
                glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
                // NEGATIVE_Z
                glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
        };

        VkCommandBuffer command_buffer = device->begin_single_time_command_buffer();

        prefiltered_cube_image->transition_image_layout(command_buffer, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        for(isize j = 0; j < mip_levels; j++) {
            push_constant_data.roughness = (float)j / (float)(mip_levels - 1);
            for(isize i = 0; i < 6; i++) {
                renderpass.start(&framebuffer, command_buffer, {dim.x * std::pow(0.5f, j), dim.y * std::pow(0.5f, j)});

                push_constant_data.mvp = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 512.0f) * matrices[i];

                vkCmdPushConstants(command_buffer, vk_pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstantData), &push_constant_data);

                pipeline.bind(command_buffer);
                vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_pipeline_layout, 0, 1, &environment_map_set, 0, nullptr);
                cube->bind(command_buffer);
                cube->draw(command_buffer);
                renderpass.end(command_buffer);
                offscreen.image->transition_image_layout(command_buffer, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

                VkImageCopy copyRegion = {};
                copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                copyRegion.srcSubresource.baseArrayLayer = 0;
                copyRegion.srcSubresource.mipLevel = 0;
                copyRegion.srcSubresource.layerCount = 1;
                copyRegion.srcOffset = { 0, 0, 0 };

                copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                copyRegion.dstSubresource.baseArrayLayer = i;
                copyRegion.dstSubresource.mipLevel = j;
                copyRegion.dstSubresource.layerCount = 1;
                copyRegion.dstOffset = { 0, 0, 0 };

                copyRegion.extent.width = dim.x * std::pow(0.5f, j);
                copyRegion.extent.height = dim.y * std::pow(0.5f, j);
                copyRegion.extent.depth = 1;

                vkCmdCopyImage(
                        command_buffer,
                        offscreen.image->vk_image,
                        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                        prefiltered_cube_image->vk_image,
                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                        1,
                        &copyRegion);

                offscreen.image->transition_image_layout(command_buffer, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
            }
        }

        prefiltered_cube_image->transition_image_layout(command_buffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        device->end_single_time_command_buffer(command_buffer);

        {
            VkDescriptorImageInfo image_info = {};
            image_info.sampler = sampler->vk_sampler;
            image_info.imageView = prefiltered_cube_image_view->vk_image_view;
            image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            DescriptorWriter(*Core::postprocessing_descriptor_set_layout, *Core::global_descriptor_pool)
                    .write_image(0, &image_info)
                    .build(device, prefiltered_cube_set);
        }

        vkDeviceWaitIdle(device->vk_device); // Just in case
        vkDestroyPipelineLayout(device->vk_device, vk_pipeline_layout, nullptr);
    }
}