//
// Created by lukas on 27.11.21.
//

#ifndef ENGINEEDITOR_POSTPROCESSINGSYSTEM_H
#define ENGINEEDITOR_POSTPROCESSINGSYSTEM_H

#include <vulkan/vulkan.h>
#include "../Graphics/Device.h"
#include "../pgepch.h"
#include "../Graphics/FrameInfo.h"
#include "../Graphics/Core.h"
#include "../Graphics/FrameBufferAttachment.h"
#include "../Graphics/Pipeline.h"

namespace Engine {

    class PostProcessingSystem {
    public:
        PostProcessingSystem(std::shared_ptr<Device> device, uint32_t width, uint32_t height);

        ~PostProcessingSystem();

        VkSampler GetSampler() { return sampler->GetSampler(); }

        VkImageView GetImageView() { return color->view->GetImageView(); }

        VkRenderPass GetRenderPass() { return renderPass; }

        void SetViewportSize(const glm::vec2 &size) {
            m_Width = size.x;
            m_Height = size.y;

            CreateImages();
        }

        void Render(FrameInfo &frameInfo, VkDescriptorSet &set);

        void RenderWithoutRenderpass(FrameInfo &frameInfo, VkDescriptorSet &set);

    private:
        void CreateImages();

        bool first = true;
        uint32_t m_Width, m_Height;
        VkFramebuffer frameBuffer;
        FrameBufferAttachment* color;
        FrameBufferAttachment* depth;
        Sampler* sampler;
        VkRenderPass renderPass;
        VkDescriptorImageInfo descriptor;

        void createPipelineLayout();

        void Start(FrameInfo &frameInfo);

        void End(FrameInfo &frameInfo);

        void createPipeline(VkRenderPass renderPass);

        std::unique_ptr<Pipeline> m_Pipeline;
        VkPipelineLayout pipelineLayout;

        std::shared_ptr<Device> m_Device;
    };
}


#endif //ENGINEEDITOR_POSTPROCESSINGSYSTEM_H
