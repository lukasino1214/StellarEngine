//
// Created by lukas on 19.11.21.
//

#ifndef VENDOR_FRAME_INFO_H
#define VENDOR_FRAME_INFO_H

#include "Camera.h"

#include <vulkan/vulkan.h>

namespace Engine {
    struct FrameInfo {
        int frameIndex;
        float frameTime;
        VkCommandBuffer commandBuffer;
        Camera &camera;
        VkDescriptorSet globalDescriptorSet;
    };
}

#endif //VENDOR_FRAME_INFO_H
