//
// Created by lukas on 14.11.21.
//

#ifndef ENGINEEDITOR_RENDER_SYSTEM_H
#define ENGINEEDITOR_RENDER_SYSTEM_H

#include "../Graphics/Camera.h"
#include "../Graphics/Device.h"
#include "../Graphics/Pipeline.h"
#include "../Graphics/FrameInfo.h"
#include "../Data/Scene.h"
#include "../Data/Entity.h"
#include "../pgepch.h"


namespace Engine {
    class RenderSystem {
    public:
        RenderSystem(std::shared_ptr<Device> device, VkRenderPass renderPass);

        ~RenderSystem();

        RenderSystem(const RenderSystem &) = delete;

        RenderSystem &operator=(const RenderSystem &) = delete;

        void renderGameObjects(FrameInfo &frameInfo, const Ref<Scene> &Scene);

    private:
        void createPipelineLayout();

        void createPipeline(VkRenderPass renderPass);

        std::unique_ptr<Pipeline> m_Pipeline;
        VkPipelineLayout pipelineLayout;

        std::shared_ptr<Device> m_Device;
    };
}

#endif //ENGINEEDITOR_RENDER_SYSTEM_H
