//
// Created by lukas on 25.11.21.
//

#ifndef ENGINE_GRIDSYSTEM_H
#define ENGINE_GRIDSYSTEM_H


#include "../Graphics/Camera.h"
#include "../Graphics/Device.h"
#include "../Graphics/FrameInfo.h"
#include "../Graphics/Pipeline.h"
#include "../pgepch.h"

namespace Engine {
    class GridSystem {
    public:
        GridSystem(std::shared_ptr<Device> device, VkRenderPass renderPass);

        ~GridSystem();

        GridSystem(const GridSystem &) = delete;

        GridSystem &operator=(const GridSystem &) = delete;

        void render(FrameInfo &frameInfo);

    private:
        void createPipelineLayout();

        void createPipeline(VkRenderPass renderPass);

        std::unique_ptr<Pipeline> m_Pipeline;
        VkPipelineLayout pipelineLayout;

        std::shared_ptr<Device> m_Device;
    };
}


#endif //ENGINE_GRIDSYSTEM_H
