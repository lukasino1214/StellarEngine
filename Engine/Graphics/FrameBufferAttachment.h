#pragma once

#include "Image.h"
#include "Device.h"
#include "../pgepch.h"
#include <cstdint>

namespace Engine {
    struct FrameBufferAttachmentDecs {
        Format format;
        glm::ivec3 dimensions;
        Usage usage;
        VkImageLayout finalLayout;
        bool isDepth = false;
    };


    class FrameBufferAttachment {
    public:
        FrameBufferAttachment(std::shared_ptr<Engine::Device> device, const FrameBufferAttachmentDecs& decs);
        ~FrameBufferAttachment();

        Image* image;
        ImageView* view;

        glm::ivec3 dimensions;

        bool isDepth = false;
        VkImageLayout finalLayout;
    private:
        int GetAspectMask(Format format);
    };
}