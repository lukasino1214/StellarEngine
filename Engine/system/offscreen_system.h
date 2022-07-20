#pragma once

#include <vulkan/vulkan.h>
#include "../graphics/device.h"
#include "../pgepch.h"
#include "../graphics/frame_info.h"
#include "../graphics/core.h"
#include "../graphics/framebuffer_attachment.h"
#include "../graphics/renderpass.h"

namespace Engine {
    struct NewFrameBufferAttachment {
        Image* image;
        ImageView* view;
    };

    class OffScreenSystem {
    public:
        OffScreenSystem(std::shared_ptr<Device> _device, uint32_t _width, uint32_t _height);
        ~OffScreenSystem();

        OffScreenSystem(const OffScreenSystem &) = delete;
        OffScreenSystem &operator=(const OffScreenSystem &) = delete;

        VkSampler get_sampler() { return sampler->vk_sampler; }
        VkImageView get_image_view() { return color->image_view->vk_image_view; }
        VkRenderPass get_renderpass() { return renderpass->vk_renderpass; }

        void resize(u32 _width, u32 _height) { width = _width; height = _height; create_images(); }
        void start(FrameInfo frame_info);
        void end(FrameInfo frame_info);

    private:
        void create_images();

        bool first = true;
        uint32_t width, height;
        FrameBufferAttachment* color;
        FrameBufferAttachment* depth;
        Sampler* sampler;
        VkDescriptorImageInfo vk_descriptor_image_info;
        RenderPass* renderpass;

        std::shared_ptr<Device> device;
    };
}