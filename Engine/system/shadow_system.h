#pragma once

#include "../graphics/framebuffer_attachment.h"
#include <vulkan/vulkan.h>
#include "../data/scene.h"
#include "../data/entity.h"
#include "../graphics/pipeline.h"
#include "../graphics/renderpass.h"
#include "../graphics/framebuffer.h"

namespace Engine {
    class ShadowSystem {
    public:
        ShadowSystem(std::shared_ptr<Device> _device);
        ~ShadowSystem();

        ShadowSystem(const ShadowSystem &) = delete;
        ShadowSystem &operator=(const ShadowSystem &) = delete;

        void render(FrameInfo &frame_info, std::shared_ptr<Scene> scene);

        VkSampler get_sampler() { return sampler->vk_sampler; }
        VkImageView get_image_view() { return depth->image_view->vk_image_view; }
        VkRenderPass get_renderpass() { return renderpass->vk_renderpass; }

    private:
        u32 width = 1024;
        u32 height = 1024;

        Framebuffer* framebuffer;
        FrameBufferAttachment* depth;
        Sampler* sampler;
        RenderPass* renderpass;

        std::unique_ptr<Pipeline> pipeline;
        VkPipelineLayout vk_pipeline_layout;

        std::shared_ptr<Device> device;
    };
}