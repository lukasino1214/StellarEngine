//
// Created by lukas on 14.11.21.
//

#ifndef ENGINEEDITOR_SIMPLE_RENDER_SYSTEM_H
#define ENGINEEDITOR_SIMPLE_RENDER_SYSTEM_H

#include "camera.h"
#include "device.h"
#include "game_object.h"
#include "pipeline.h"

// std
#include <memory>
#include <vector>

namespace Engine {
    class SimpleRenderSystem {
    public:
        SimpleRenderSystem(Device &device, VkRenderPass renderPass);
        ~SimpleRenderSystem();

        SimpleRenderSystem(const SimpleRenderSystem &) = delete;
        SimpleRenderSystem &operator=(const SimpleRenderSystem &) = delete;

        void renderGameObjects(VkCommandBuffer commandBuffer, std::vector<GameObject> &gameObjects, const Camera& camera);

    private:
        void createPipelineLayout();
        void createPipeline(VkRenderPass renderPass);

        Device &lveDevice;

        std::unique_ptr<Pipeline> lvePipeline;
        VkPipelineLayout pipelineLayout;
    };
}

#endif //ENGINEEDITOR_SIMPLE_RENDER_SYSTEM_H
