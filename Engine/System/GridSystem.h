//
// Created by lukas on 25.11.21.
//

#ifndef ENGINE_GRIDSYSTEM_H
#define ENGINE_GRIDSYSTEM_H



#include "../Graphics/Camera.h"
#include "../Graphics/device.h"
#include "../Graphics/frame_info.h"
#include "../Graphics/pipeline.h"

// std
#include <memory>
#include <vector>

namespace Engine {
    class GridSystem {
    public:
        GridSystem(VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
        ~GridSystem();

        GridSystem(const GridSystem &) = delete;
        GridSystem &operator=(const GridSystem &) = delete;

        void render(FrameInfo &frameInfo);

    private:
        void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
        void createPipeline(VkRenderPass renderPass);

        std::unique_ptr<Pipeline> m_Pipeline;
        VkPipelineLayout pipelineLayout;
    };
}


#endif //ENGINE_GRIDSYSTEM_H
