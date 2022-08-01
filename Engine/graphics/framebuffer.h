#pragma once

#include "framebuffer_attachment.h"
#include <memory>

namespace Engine {
    class Framebuffer {
        public:
            Framebuffer(std::shared_ptr<Device> _device, VkRenderPass vk_renderpass, const std::vector<FrameBufferAttachment*>& attachments);
            ~Framebuffer();

            Framebuffer(const Framebuffer &) = delete;
            Framebuffer &operator=(const Framebuffer &) = delete;
            Framebuffer(Framebuffer &&) = delete;
            Framebuffer &operator=(Framebuffer &&) = delete;

            u32 width, height, depth;
            VkFramebuffer vk_framebuffer = {};
        private:
            std::shared_ptr<Device> device;
    };
}