#pragma once

#include <vulkan/vulkan.h>
#include "../graphics/device.h"
#include "../pgepch.h"
#include "../graphics/frame_info.h"
#include "../graphics/core.h"
#include "../graphics/framebuffer_attachment.h"
#include "../graphics/pipeline.h"
#include "../graphics/renderpass.h"
#include "../graphics/framebuffer.h"

namespace Engine {
    class PostProcessingSystem {
    public:
        PostProcessingSystem(std::shared_ptr<Device> _device, uint32_t _width, uint32_t _height);
        ~PostProcessingSystem();

        PostProcessingSystem(const PostProcessingSystem &) = delete;
        PostProcessingSystem &operator=(const PostProcessingSystem &) = delete;

        VkSampler get_sampler() { return sampler->vk_sampler; }
        VkImageView get_image_view() { return color->image_view->vk_image_view; }
        VkRenderPass get_renderpass() { return renderpass->vk_renderpass; }

        void resize(u32 _width, u32 _height) { width = _width; height = _height; create_images(); create_framebuffer(); }
        void render(FrameInfo &frame_info, VkDescriptorSet &vk_descriptor_set);

    private:
        inline void create_images();
        inline void create_framebuffer();

        bool first = true;
        u32 width, height;

        Framebuffer* framebuffer;
        FrameBufferAttachment* color;
        FrameBufferAttachment* depth;
        Sampler* sampler;
        RenderPass* renderpass;
        VkDescriptorImageInfo vk_descriptor_image_info;

        std::unique_ptr<Pipeline> pipeline;
        VkPipelineLayout vk_pipeline_layout;

        std::shared_ptr<Device> device;
    };
}