#pragma once

#include "../graphics/camera.h"
#include "../graphics/device.h"
#include "../graphics/pipeline.h"
#include "../graphics/frame_info.h"
#include "../data/scene.h"
#include "../data/entity.h"
#include "../pgepch.h"


namespace Engine {
    class RenderSystem {
    public:
        RenderSystem(std::shared_ptr<Device> _device, VkRenderPass renderpass);
        ~RenderSystem();

        RenderSystem(const RenderSystem &) = delete;
        RenderSystem &operator=(const RenderSystem &) = delete;

        void render(FrameInfo &frame_info, const std::shared_ptr<Scene> &scene);

    private:
        std::unique_ptr<Pipeline> pipeline;
        VkPipelineLayout vk_pipeline_layout;

        std::shared_ptr<Device> device;
    };
}