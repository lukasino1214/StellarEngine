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
        OffScreen(Device &device, VkDescriptorSetLayout globalSetLayout);
        //void Init(Device device);
        ~OffScreen() {
            vkDestroyImageView(m_Device.device(), pass.color.view, nullptr);
            vkDestroyImage(m_Device.device(), pass.color.image, nullptr);
            vkFreeMemory(m_Device.device(), pass.color.mem, nullptr);

            // Depth attachment
            vkDestroyImageView(m_Device.device(), pass.depth.view, nullptr);
            vkDestroyImage(m_Device.device(), pass.depth.image, nullptr);
            vkFreeMemory(m_Device.device(), pass.depth.mem, nullptr);

            vkDestroyRenderPass(m_Device.device(), pass.renderPass, nullptr);
            vkDestroySampler(m_Device.device(), pass.sampler, nullptr);
            vkDestroyFramebuffer(m_Device.device(), pass.frameBuffer, nullptr);
        }

        void render(FrameInfo frameInfo, const Ref<Scene> &Scene);
        VkSampler GetSampler() { return pass.sampler; }
        VkImageView GetImageView() { return pass.color.view; }
        VkRenderPass GetRenderPass() { return pass.renderPass; }
        void SetViewportSize(const glm::vec2& size) { pass.width = size.x; pass.height = size.y; }

        void Start(FrameInfo frameInfo);
        void End(FrameInfo frameInfo);
    private:
        void setupPipelineLayout(VkDescriptorSetLayout globalSetLayout);
        void preparePipelines();

        OffscreenPass pass;
        Device &m_Device;
        VkPipelineLayout pipelineLayout;
        std::unique_ptr<Pipeline> m_Pipeline;
        VkDescriptorSetLayout DescriptorSetLayout;
    };
}


#endif //ENGINEEDITOR_OFFSCREEN_H
