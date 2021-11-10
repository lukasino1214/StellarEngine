//
// Created by lukas on 08.11.21.
//

#ifndef ENGINEEDITOR_APP_H
#define ENGINEEDITOR_APP_H

#include "../Engine/Engine.h"

// std
#include <memory>
#include <vector>

namespace Engine {
    class FirstApp {
    public:
        static constexpr int WIDTH = 1280;
        static constexpr int HEIGHT = 720;

        FirstApp();
        ~FirstApp();

        FirstApp(const FirstApp &) = delete;
        FirstApp &operator=(const FirstApp &) = delete;

        void run();

    private:
        void loadModels();
        void createPipelineLayout();
        void createPipeline();
        void createCommandBuffers();
        void freeCommandBuffers();
        void drawFrame();
        void recreateSwapChain();
        void recordCommandBuffer(int imageIndex);

        Window lveWindow{WIDTH, HEIGHT, "Stellar Engine"};
        Device lveDevice{lveWindow};
        std::unique_ptr<SwapChain> lveSwapChain;
        std::unique_ptr<Pipeline> lvePipeline;
        VkPipelineLayout pipelineLayout;
        std::vector<VkCommandBuffer> commandBuffers;
        std::unique_ptr<Model> lveModel;
    };
}


#endif //ENGINEEDITOR_APP_H
