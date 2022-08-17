#pragma once

#include "../graphics/camera.h"
#include "../graphics/device.h"
#include "../graphics/pipeline.h"
#include "../graphics/frame_info.h"
#include "../data/scene.h"
#include "../data/entity.h"
#include "../pgepch.h"

namespace Engine {
    class PointLightSystem {
    public:
        PointLightSystem(std::shared_ptr<Device> _device, VkRenderPass renderpass);
        ~PointLightSystem();

        PointLightSystem(const PointLightSystem &) = delete;
        PointLightSystem &operator=(const PointLightSystem &) = delete;

        void render(FrameInfo &frame_info, const std::shared_ptr<Scene> &scene);

    private:
        std::unique_ptr<Pipeline> pipeline;
        VkPipelineLayout vk_pipeline_layout;

        std::shared_ptr<Device> device;
    };
}