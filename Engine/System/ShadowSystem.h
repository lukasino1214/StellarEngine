//
// Created by lukas on 07.05.22.
//

#ifndef ENGINE_SHADOWSYSTEM_H
#define ENGINE_SHADOWSYSTEM_H

#include "../Graphics/FrameBufferAttachment.h"
#include <vulkan/vulkan.h>
#include "../Data/Scene.h"
#include "../Data/Entity.h"
#include "../Graphics/Pipeline.h"
#include "../Graphics/RenderPass.h"

namespace Engine {
    class ShadowSystem {
    public:
        ShadowSystem(std::shared_ptr<Device> device);
        ~ShadowSystem();

        void Render(FrameInfo &frameInfo, std::shared_ptr<Scene> Scene);

        VkSampler GetSampler() { return m_Sampler->GetSampler(); }
        VkImageView GetImageView() { return depth->view->GetImageView(); }
        VkRenderPass GetRenderPass() { return renderpass->GetRenderPass(); }

    private:
        VkRenderPass m_RenderPass;
        uint32_t m_Width = 1024;
        uint32_t m_Height = 1024;
        FrameBufferAttachment* depth;
        Sampler* m_Sampler;
        VkFramebuffer m_FrameBuffer;
        RenderPass* renderpass;

        std::unique_ptr<Pipeline> m_Pipeline;
        VkPipelineLayout pipelineLayout;

        std::shared_ptr<Device> m_Device;
    };
}


#endif //ENGINE_SHADOWSYSTEM_H
