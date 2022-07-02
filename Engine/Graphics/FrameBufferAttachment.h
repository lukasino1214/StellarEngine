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
    };


    class FrameBufferAttachment {
    public:
        FrameBufferAttachment(std::shared_ptr<Engine::Device> device, const FrameBufferAttachmentDecs& decs);
        ~FrameBufferAttachment();

        Image* image;
        ImageView* view;
    private:
        int GetAspectMask(Format format);
    };
}