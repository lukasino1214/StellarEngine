#include "framebuffer_attachment.h"
#include "framebuffer.h"

namespace Engine {
    Framebuffer::Framebuffer(std::shared_ptr<Device> _device, VkRenderPass vk_renderpass, const std::vector<FrameBufferAttachment*>& attachments) : device{_device} {
        std::vector<VkImageView> image_views;

        for(auto& attachment : attachments) {
            image_views.push_back(attachment->image_view->vk_image_view);
        }

        width = static_cast<uint32_t>(attachments[0]->dimensions.x);
        height = static_cast<uint32_t>(attachments[0]->dimensions.y);
        depth = static_cast<uint32_t>(attachments[0]->dimensions.z);
        
        VkFramebufferCreateInfo vk_framebuffer_create_info = {
                .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .renderPass = vk_renderpass,
                .attachmentCount = static_cast<uint32_t>(image_views.size()),
                .pAttachments = image_views.data(),
                .width = width,
                .height = height,
                .layers = depth,

        };

        if (vkCreateFramebuffer(device->vk_device, &vk_framebuffer_create_info, nullptr, &vk_framebuffer) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create framebuffer");
        }
    }

    Framebuffer::~Framebuffer() {
        vkDestroyFramebuffer(device->vk_device, vk_framebuffer, nullptr);
    }
}