//
// Created by lukas on 27.11.21.
//

#ifndef ENGINEEDITOR_OFFSCREEN_H
#define ENGINEEDITOR_OFFSCREEN_H

#include <vulkan/vulkan.h>
#include "device.h"
#include "pipeline.h"
#include <memory>
#include "frame_info.h"
#include "../Data/Scene.h"
#include "../Data/Entity.h"
#include "../System/GridSystem.h"
#include "core.h"

namespace Engine {

    struct FrameBufferAttachment {
        VkImage image;
        VkDeviceMemory mem;
        VkImageView view;
    };
    struct OffscreenPass {
        uint32_t width, height;
        VkFramebuffer frameBuffer;
        FrameBufferAttachment color, depth;
        VkRenderPass renderPass;
        VkSampler sampler;
        VkDescriptorImageInfo descriptor;
    };

    class OffScreen {
    public:
        OffScreen();
        //void Init(Device device);
        ~OffScreen() {
            auto device = Core::m_Device->device();
            vkDestroyImageView(device, pass.color.view, nullptr);
            vkDestroyImage(device, pass.color.image, nullptr);
            vkFreeMemory(device, pass.color.mem, nullptr);

            // Depth attachment
            vkDestroyImageView(device, pass.depth.view, nullptr);
            vkDestroyImage(device, pass.depth.image, nullptr);
            vkFreeMemory(device, pass.depth.mem, nullptr);

            vkDestroyRenderPass(device, pass.renderPass, nullptr);
            vkDestroySampler(device, pass.sampler, nullptr);
            vkDestroyFramebuffer(device, pass.frameBuffer, nullptr);
        }

        VkSampler GetSampler() { return pass.sampler; }
        VkImageView GetImageView() { return pass.color.view; }
        VkRenderPass GetRenderPass() { return pass.renderPass; }
        void SetViewportSize(const glm::vec2& size) {
            pass.width = size.x;
            pass.height = size.y;

            CreateImages();
        }

        void Start(FrameInfo frameInfo);
        void End(FrameInfo frameInfo);
    private:
        void CreateImages();

        OffscreenPass pass;
        bool after = false;
    };
}


#endif //ENGINEEDITOR_OFFSCREEN_H
