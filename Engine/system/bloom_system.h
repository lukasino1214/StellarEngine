#pragma once
#include "../graphics/renderpass.h"
#include "../graphics/frame_info.h"
#include "../graphics/pipeline.h"

namespace Engine {
    class BloomRenderingSystem {
    public:
        BloomRenderingSystem(std::shared_ptr<Device> _device, u32 _width, u32 _height);
        ~BloomRenderingSystem();

        void render(FrameInfo frame_info, VkDescriptorSet vk_emissive_descriptor_set);

        VkDescriptorSet get_emissive_descriptor_set() { return emissive_set; }

    private:
        u32 mip_levels = 2;
        u32 width, height;

        FrameBufferAttachment* emissive_framebuffer_attachement;
        Framebuffer* emissive_framebuffer;
        VkDescriptorSet emissive_set;

        std::vector<FrameBufferAttachment*> framebuffer_attachements {};
        std::vector<Framebuffer*> framebuffers {};
        std::vector<glm::ivec2> sizes {};
        std::vector<VkDescriptorSet> vk_descriptor_sets {};

        Sampler* sampler;

        RenderPass* renderpass;

        std::unique_ptr<Pipeline> upsample_pipeline;
        std::unique_ptr<Pipeline> downsample_pipeline;
        VkPipelineLayout vk_pipeline_layout;

        std::shared_ptr<Device> device;
    };
}
