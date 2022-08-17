#pragma once

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
        OffScreenSystem(std::shared_ptr<Device> _device, i32 _width, i32 _height);
        ~OffScreenSystem();

        OffScreenSystem(const OffScreenSystem &) = delete;
        OffScreenSystem &operator=(const OffScreenSystem &) = delete;

        VkSampler get_sampler() { return sampler->vk_sampler; }
        VkImageView get_image_view() { return color->image_view->vk_image_view; }
        VkRenderPass get_renderpass() { return renderpass->vk_renderpass; }

        void resize(i32 _width, i32 _height);
        void start(FrameInfo frame_info);
        void end(FrameInfo frame_info);

        VkDescriptorSet& get_present_descriptor_set() { return vk_present_descriptor_set; }

    private:
        inline void create_images();
        inline void create_framebuffer();

        bool first = true;
        i32 width, height;
        Framebuffer* framebuffer;
        FrameBufferAttachment* color;
        FrameBufferAttachment* depth;
        Sampler* sampler;
        RenderPass* renderpass;

        VkDescriptorSet vk_present_descriptor_set;

        std::shared_ptr<Device> device;
    };
}