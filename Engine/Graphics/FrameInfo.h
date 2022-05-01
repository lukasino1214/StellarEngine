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
        VkDescriptorSet globalDescriptorSet;
    };

    struct PointLight {
        glm::vec4 position{};  // ignore w
        glm::vec4 color{};     // w is intensity
    };

    struct GlobalUbo {
        glm::mat4 projectionMat{1.0f};
        glm::mat4 viewMat{1.0f};
        glm::vec4 cameraPos{0.0f, 0.0f, 0.0f, 0.0f};
        PointLight pointLights[10];
        int numLights;
    };
}

#endif //VENDOR_FRAME_INFO_H
