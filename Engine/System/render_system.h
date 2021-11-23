//
// Created by lukas on 14.11.21.
//

#ifndef ENGINEEDITOR_RENDER_SYSTEM_H
#define ENGINEEDITOR_RENDER_SYSTEM_H

#include "../Graphics/Camera.h"
#include "../Graphics/device.h"
#include "../Graphics/pipeline.h"
#include "../Graphics/frame_info.h"
#include "../Data/Scene.h"
#include "../Data/Entity.h"
#include "../pgepch.h"

// std
#include <memory>
#include <vector>

namespace Engine {
    class RenderSystem {
    public:
        RenderSystem(Device &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
        ~RenderSystem();

        RenderSystem(const RenderSystem &) = delete;
        RenderSystem &operator=(const RenderSystem &) = delete;

        void renderGameObjects(FrameInfo &frameInfo, const Ref<Scene> &Scene);

    private:
        void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
        void createPipeline(VkRenderPass renderPass);

        Device &lveDevice;

        std::unique_ptr<Pipeline> m_Pipeline;
        VkPipelineLayout pipelineLayout;
    };
}

#endif //ENGINEEDITOR_RENDER_SYSTEM_H
