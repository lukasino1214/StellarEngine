//
// Created by lukas on 27.11.21.
//

#ifndef ENGINEEDITOR_OFFSCREEN_H
#define ENGINEEDITOR_OFFSCREEN_H

#include <vulkan/vulkan.h>
#include "../Graphics/Device.h"
#include "../pgepch.h"
#include "../Graphics/FrameInfo.h"
#include "../Graphics/Core.h"
#include "../Graphics/FrameBufferAttachment.h"

namespace Engine {
    struct NewFrameBufferAttachment {
        Image* image;
        ImageView* view;
    };

    class OffScreen {
    public:
        OffScreen(std::shared_ptr<Device> device, uint32_t width, uint32_t height);
        ~OffScreen();

        VkSampler GetSampler() { return sampler->GetSampler(); }
        VkImageView GetImageView() { return color->view->GetImageView(); }
        VkRenderPass GetRenderPass() { return renderPass; }

        void SetViewportSize(const glm::vec2 &size) {
            m_Width = size.x;
            m_Height = size.y;

            CreateImages();
        }

        void Start(FrameInfo frameInfo);

        void End(FrameInfo frameInfo);

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

        std::shared_ptr<Device> m_Device;
    };
}


#endif //ENGINEEDITOR_OFFSCREEN_H
