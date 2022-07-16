#pragma once

#include "FrameBufferAttachment.h"
#include <vulkan/vulkan_core.h>

namespace Engine {
    enum class LoadOp {
        LOAD = 0,
        CLEAR = 1,
        DONT_CARE = 2,
        NONE_EXT = 1000400000,
        MAX_ENUM = 0x7FFFFFFF
    };

    enum class StoreOp {
        STORE = 0,
        DONT_CARE = 1,
        NONE = 1000301000,
        NONE_KHR = VK_ATTACHMENT_STORE_OP_NONE,
        NONE_QCOM = VK_ATTACHMENT_STORE_OP_NONE,
        NONE_EXT = VK_ATTACHMENT_STORE_OP_NONE,
        MAX_ENUM = 0x7FFFFFFF
    };

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
        RenderPass(std::shared_ptr<Engine::Device> device, std::vector<Attachment> attachments, std::vector<SubpassInfo> subpassInfos, std::string debugName);
        ~RenderPass();

        void NextSubpass(VkCommandBuffer commandBuffer);

        void Start(VkCommandBuffer commandBuffer);
        void End(VkCommandBuffer commandBuffer);

        VkRenderPass GetRenderPass() { return VK_RenderPass; }
    private:
        VkRenderPass VK_RenderPass;
        VkFramebuffer VK_FrameBuffer;

        std::shared_ptr<Device> m_Device;

        std::vector<Attachment> m_Attachments;
        std::vector<VkClearValue> clearValues{};
    };
}