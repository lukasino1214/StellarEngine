//
// Created by lukas on 14.11.21.
//

#ifndef ENGINEEDITOR_POINT_LIGHT_SYSTEM_H
#define ENGINEEDITOR_POINT_LIGHT_SYSTEM_H

#include "../Graphics/Camera.h"
#include "../Graphics/Device.h"
#include "../Graphics/Pipeline.h"
#include "../Graphics/FrameInfo.h"
#include "../Data/Scene.h"
#include "../Data/Entity.h"
#include "../pgepch.h"

namespace Engine {
    class PointLightSystem {
    public:
        PointLightSystem(std::shared_ptr<Device> device, VkRenderPass renderPass);

        ~PointLightSystem();

        PointLightSystem(const PointLightSystem &) = delete;

        PointLightSystem &operator=(const PointLightSystem &) = delete;

        void renderGameObjects(FrameInfo &frameInfo, const Ref<Scene> &Scene);

    private:
        void createPipelineLayout();

        void createPipeline(VkRenderPass renderPass);

        std::unique_ptr<Pipeline> m_Pipeline;
        VkPipelineLayout pipelineLayout;

        std::shared_ptr<Device> m_Device;
    };
}

#endif //ENGINEEDITOR_POINT_LIGHT_SYSTEM_H
