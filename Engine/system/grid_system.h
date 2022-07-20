#pragma once

#include "../graphics/camera.h"
#include "../graphics/device.h"
#include "../graphics/frame_info.h"
#include "../graphics/pipeline.h"
#include "../pgepch.h"

namespace Engine {
    class GridSystem {
    public:
        GridSystem(std::shared_ptr<Device> _device, VkRenderPass renderpass);
        ~GridSystem();

        GridSystem(const GridSystem &) = delete;
        GridSystem &operator=(const GridSystem &) = delete;

        void render(FrameInfo &frame_info);

    private:
        std::unique_ptr<Pipeline> pipeline;
        VkPipelineLayout vk_pipeline_layout;

        std::shared_ptr<Device> device;
    };
}