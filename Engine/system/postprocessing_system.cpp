#include "postprocessing_system.h"

#include <iostream>
#include <glm/glm.hpp>

namespace Engine {
    PostProcessingSystem::PostProcessingSystem(std::shared_ptr<Device> _device, uint32_t _width, uint32_t _height) : width{_width}, height{_height}, device{_device} {
        create_images();

        renderpass = new RenderPass(device, {
                { .frameBufferAttachment = color, .loadOp = LoadOp::CLEAR, .storeOp = StoreOp::STORE },
                { .frameBufferAttachment = depth, .loadOp = LoadOp::CLEAR, .storeOp = StoreOp::STORE }
        }, { { .renderTargets = { 0, 1 }, .subpassInputs = {} } });

        create_framebuffer();

        std::vector<VkDescriptorSetLayout> descriptor_set_layouts = {Core::postprocessing_descriptor_set_layout->get_descriptor_set_layout(), Core::global_descriptor_set_layout->get_descriptor_set_layout()};

        VkPipelineLayoutCreateInfo vk_pipeline_layout_create_info = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .setLayoutCount = static_cast<uint32_t>(descriptor_set_layouts.size()),
                .pSetLayouts = descriptor_set_layouts.data(),
                .pushConstantRangeCount = 0,
                .pPushConstantRanges = nullptr
        };

        if (vkCreatePipelineLayout(device->vk_device, &vk_pipeline_layout_create_info, nullptr, &vk_pipeline_layout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }

        PipelineConfigInfo pipeline_config = {};
        Pipeline::default_pipeline_config_info(pipeline_config);
        pipeline_config.attribute_descriptions.clear();
        pipeline_config.binding_descriptions.clear();
        pipeline_config.vk_renderpass = renderpass->vk_renderpass;
        pipeline_config.vk_pipeline_layout = vk_pipeline_layout;

        pipeline = std::make_unique<Pipeline>(device, pipeline_config, ShaderFilepaths {
                .vertex = "assets/shaders/post_processing.vert",
                .fragment = "assets/shaders/post_processing.frag"
        });
    }

    void PostProcessingSystem::create_framebuffer() {
        if(!first) {
            delete framebuffer;
        }

        framebuffer = new Framebuffer(device, renderpass->vk_renderpass, { color, depth });

        first = false;
    }

    void PostProcessingSystem::create_images() {
        if(!first) {
            vkDeviceWaitIdle(device->vk_device);
            delete color;
            delete depth;

            delete sampler;
        }

        VkFormat fb_depth_format = device->find_supported_format({VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT}, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

        color = new FrameBufferAttachment(device, {
                .format = ImageFormat::B8G8R8A8_UNORM,
                .dimensions = { width, height, 1 },
                .usage = ImageUsageFlagBits::COLOR_ATTACHMENT | ImageUsageFlagBits::SAMPLED,
        });

        depth = new FrameBufferAttachment(device, {
                .format = (ImageFormat)fb_depth_format,
                .dimensions = { width, height, 1 },
                .usage = ImageUsageFlagBits::DEPTH_STENCIL_ATTACHMENT,
        });

        sampler = new Sampler(device, {
                .min_filter = Filter::LINEAR,
                .mag_filter = Filter::LINEAR,
                .max_anistropy = 1.0,
        });

        /*renderpass = new RenderPass(device, {
            { .frameBufferAttachment = color, .loadOp = LoadOp::CLEAR, .storeOp = StoreOp::STORE },
            { .frameBufferAttachment = depth, .loadOp = LoadOp::CLEAR, .storeOp = StoreOp::STORE }
        }, { { .renderTargets = { 0, 1 }, .subpassInputs = {} } });*/

        vk_descriptor_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        vk_descriptor_image_info.imageView = color->image_view->vk_image_view;
        vk_descriptor_image_info.sampler = sampler->vk_sampler;
    };

    void PostProcessingSystem::render(FrameInfo &frame_info, VkDescriptorSet &vk_descriptor_set) {
        renderpass->start(framebuffer, frame_info.command_buffer);

        pipeline->bind(frame_info.command_buffer);
        std::vector<VkDescriptorSet> vk_descriptor_sets = {vk_descriptor_set, frame_info.vk_global_descriptor_set};
        vkCmdBindDescriptorSets(frame_info.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_pipeline_layout, 0, 2, vk_descriptor_sets.data(), 0, nullptr);
        vkCmdDraw(frame_info.command_buffer, 6, 1, 0, 0);

        renderpass->end(frame_info.command_buffer);
    }

    PostProcessingSystem::~PostProcessingSystem() {
        delete color;
        delete depth;

        delete sampler;
        delete framebuffer;

        delete renderpass;

        vkDestroyPipelineLayout(device->vk_device, vk_pipeline_layout, nullptr);
    }
}