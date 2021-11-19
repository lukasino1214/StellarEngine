//
// Created by lukas on 14.11.21.
//

#ifndef ENGINEEDITOR_SIMPLE_RENDER_SYSTEM_H
#define ENGINEEDITOR_SIMPLE_RENDER_SYSTEM_H

#include "camera.h"
#include "device.h"
#include "game_object.h"
#include "pipeline.h"
#include "frame_info.h"

// std
#include <memory>
#include <vector>

namespace Engine {
    class SimpleRenderSystem {
    public:
        SimpleRenderSystem(Device &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
        ~SimpleRenderSystem();

        SimpleRenderSystem(const SimpleRenderSystem &) = delete;
        SimpleRenderSystem &operator=(const SimpleRenderSystem &) = delete;

        void renderGameObjects(FrameInfo &frameInfo, std::vector<GameObject> &gameObjects);

    private:
        void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
        void createPipeline(VkRenderPass renderPass);

        Device &lveDevice;

        std::unique_ptr<Pipeline> lvePipeline;
        VkPipelineLayout pipelineLayout;
    };
}

#endif //ENGINEEDITOR_SIMPLE_RENDER_SYSTEM_H
