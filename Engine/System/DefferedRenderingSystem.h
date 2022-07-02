//
// Created by lukas on 18.6.22.
//

/*#ifndef ENGINE_DEFFEREDRENDERINGSYSTEM_H
#define ENGINE_DEFFEREDRENDERINGSYSTEM_H

#include <memory>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include "../Graphics/FrameInfo.h"
#include "../Data/Scene.h"
#include "../pgepch.h"
#include "OffScreen.h"

namespace Engine {
    class DefferedRenderingSystem {
    public:
        DefferedRenderingSystem(std::shared_ptr<Device> device, int width, int height);
        void Render(FrameInfo &frameInfo, const std::shared_ptr<Scene> &Scene);
    private:
        void createAttachment(VkFormat format, VkImageUsageFlags usage, FrameBufferAttachment *attachment);
        void createGBufferAttachments();
        void setupFrameBuffer();
        void clearAttachment(FrameBufferAttachment* attachment);

        int m_Width;
        int m_Height; 
        std::shared_ptr<Device> m_Device;
        VkRenderPass renderPass;
        VkFramebuffer frameBuffer;
        VkDescriptorSet compositionSet;
        VkDescriptorSet forwardSet;

        struct Attachments {
            FrameBufferAttachment position, normal, albedo;
            int32_t width;
            int32_t height;
	    } attachments;
    };
}


#endif //ENGINE_DEFFEREDRENDERINGSYSTEM_H
*/