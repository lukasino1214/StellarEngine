//
// Created by lukas on 07.05.22.
//

#ifndef ENGINE_SHADOWSYSTEM_H
#define ENGINE_SHADOWSYSTEM_H

#include "OffScreen.h"
#include <vulkan/vulkan.h>
#include "../Data/Scene.h"
#include "../Data/Entity.h"

namespace Engine {
    class ShadowSystem {
    public:
        ShadowSystem();

        void Render(FrameInfo &frameInfo, std::shared_ptr<Scene> Scene);

        VkSampler GetSampler() { return m_Sampler; }
        VkImageView GetImageView() { return depth.view; }
        VkRenderPass GetRenderPass() { return m_RenderPass; }

    private:
        VkRenderPass m_RenderPass;
        uint32_t m_Width = 1024;
        uint32_t m_Height = 1024;
        FrameBufferAttachment depth;
        VkSampler m_Sampler;
        VkFramebuffer m_FrameBuffer;
        VkShaderModule vertShaderModule;
        VkPipeline graphicsPipeline;
        VkPipelineLayout m_PipelineLayout;
    };
}


#endif //ENGINE_SHADOWSYSTEM_H
