#pragma once

#include "image.h"
#include "device.h"
#include "../pgepch.h"
#include "vk_types.h"
#include <cstdint>

namespace Engine {
    struct FrameBufferAttachmentDescription {
        ImageFormat format;
        glm::ivec3 dimensions;
        ImageUsageFlags usage;
        VkImageLayout final_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    };


    class FrameBufferAttachment {
    public:
        FrameBufferAttachment(std::shared_ptr<Device> device, const FrameBufferAttachmentDescription& _description);
        ~FrameBufferAttachment();

        Image* image;
        ImageView* image_view;

        glm::ivec3 dimensions;

        bool is_depth = false;
        VkImageLayout final_layout;
    private:
        int get_aspect_mask(ImageFormat format);

        FrameBufferAttachmentDescription description;
    };
}