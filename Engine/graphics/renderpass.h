#pragma once

#include "framebuffer_attachment.h"

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
        RenderPass(std::shared_ptr<Engine::Device> device, std::vector<Attachment> attachments, std::vector<SubpassInfo> subpassInfos);
        ~RenderPass();

        void NextSubpass(VkCommandBuffer commandBuffer);

        void start(VkCommandBuffer commandBuffer);
        void end(VkCommandBuffer commandBuffer);

        VkRenderPass vk_renderpass;
    private:
        VkFramebuffer VK_FrameBuffer;

        std::shared_ptr<Device> m_Device;

        std::vector<Attachment> m_Attachments;
        std::vector<VkClearValue> clearValues{};
    };
}