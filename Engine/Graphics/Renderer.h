//
// Created by lukas on 14.11.21.
//

#ifndef VENDOR_RENDERER_H
#define VENDOR_RENDERER_H

#include "Device.h"
#include "SwapChain.h"
#include "../Core/Window.h"

// std
#include "../pgepch.h"

namespace Engine {
    class Renderer {
    public:
        Renderer(std::shared_ptr<Window> window, std::shared_ptr<Device> device);

        ~Renderer();

        Renderer(const Renderer &) = delete;

        Renderer &operator=(const Renderer &) = delete;

        VkRenderPass getSwapChainRenderPass() const { return m_SwapChain->getRenderPass(); }

        uint32_t getImageCount() const { return m_SwapChain->imageCount(); }

        float getAspectRatio() const { return m_SwapChain->extentAspectRatio(); }

        bool isFrameInProgress() const { return isFrameStarted; }

        VkImageView getImageView(int index) { return m_SwapChain->getImageView(index); }

        VkCommandBuffer getCurrentCommandBuffer() const {
            assert(isFrameStarted && "Cannot get command buffer when frame not in progress");
            return commandBuffers[currentFrameIndex];
        }

        int getFrameIndex() const {
            assert(isFrameStarted && "Cannot get frame index when frame not in progress");
            return currentFrameIndex;
        }

        VkCommandBuffer beginFrame();

        void endFrame();

        void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);

        void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

    private:
        void createCommandBuffers();

        void freeCommandBuffers();

        void recreateSwapChain();

        std::shared_ptr<Window> m_Window;
        std::shared_ptr<Device> m_Device;
        std::unique_ptr<SwapChain> m_SwapChain;
        std::vector<VkCommandBuffer> commandBuffers;

        uint32_t currentImageIndex;
        int currentFrameIndex{0};
        bool isFrameStarted{false};
    };
}


#endif //VENDOR_RENDERER_H