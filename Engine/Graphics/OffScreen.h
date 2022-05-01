//
// Created by lukas on 27.11.21.
//

#ifndef ENGINEEDITOR_OFFSCREEN_H
#define ENGINEEDITOR_OFFSCREEN_H

#include <vulkan/vulkan.h>
#include "Device.h"
#include "pipeline.h"
#include <memory>
#include "FrameInfo.h"
#include "../Data/Scene.h"
#include "../Data/Entity.h"
#include "../System/GridSystem.h"
#include "Core.h"

namespace Engine {

    struct FrameBufferAttachment {
        VkImage image;
        VkDeviceMemory mem;
        VkImageView view;
    };

    class OffScreen {
    public:
        OffScreen(uint32_t width, uint32_t height);
        //void Init(Device device);
        ~OffScreen() {
            auto device = Core::m_Device->device();
            vkDestroyImageView(device, color.view, nullptr);
            vkDestroyImage(device, color.image, nullptr);
            vkFreeMemory(device, color.mem, nullptr);

            // Depth attachment
            vkDestroyImageView(device, depth.view, nullptr);
            vkDestroyImage(device, depth.image, nullptr);
            vkFreeMemory(device, depth.mem, nullptr);

            vkDestroyRenderPass(device, renderPass, nullptr);
            vkDestroySampler(device, sampler, nullptr);
            vkDestroyFramebuffer(device, frameBuffer, nullptr);
        }

        VkSampler GetSampler() { return sampler; }
        VkImageView GetImageView() { return color.view; }
        VkRenderPass GetRenderPass() { return renderPass; }
        void SetViewportSize(const glm::vec2& size) {
            m_Width = size.x;
            m_Height = size.y;

            CreateImages();
        }

        void Start(FrameInfo frameInfo);
        void End(FrameInfo frameInfo);
    private:
        void CreateImages();

        uint32_t m_Width, m_Height;
        VkFramebuffer frameBuffer;
        FrameBufferAttachment color, depth;
        VkRenderPass renderPass;
        VkSampler sampler;
        VkDescriptorImageInfo descriptor;
        bool after = false;
    };
}


#endif //ENGINEEDITOR_OFFSCREEN_H
