#pragma once

#include "framebuffer_attachment.h"
#include "framebuffer.h"

namespace Engine {
    struct Attachment {
        FrameBufferAttachment* frameBufferAttachment;
        LoadOp loadOp;
        StoreOp storeOp;
    };

    struct SubpassInfo {
        std::vector<u32> renderTargets;
        std::vector<u32> subpassInputs;
    };

    struct SubpassDescription {
        VkAttachmentReference depthReference{};
        std::vector<VkAttachmentReference> colorReferences{};
        std::vector<VkAttachmentReference> inputReferences{};
        VkSubpassDescription subpassDescription;
    };

    class RenderPass {
    public:
        RenderPass(std::shared_ptr<Device> device, std::vector<Attachment> attachments, std::vector<SubpassInfo> subpassInfos);
        ~RenderPass();

        void next_subpass(VkCommandBuffer command_buffer);

        void start(Framebuffer* framebuffer, VkCommandBuffer command_buffer, glm::vec2 viewport_size = { 0.0, 0.0});
        void end(VkCommandBuffer command_buffer);

        VkRenderPass vk_renderpass;
    private:
        std::shared_ptr<Device> m_Device;

        u32 width, height;

        std::vector<VkClearValue> clearValues{};
    };
}