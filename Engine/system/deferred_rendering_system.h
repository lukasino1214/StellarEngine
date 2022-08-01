#pragma once

#include "../pgepch.h"
#include "../graphics/device.h"
#include "../graphics/renderpass.h"
#include "../data/scene.h"
#include "../graphics/pipeline.h"
#include "../graphics/descriptor_set.h"

namespace Engine {
    class DeferredRenderingSystem {
    public:
        DeferredRenderingSystem(std::shared_ptr<Device> _device, u32 _width, u32 _height);
        ~DeferredRenderingSystem();

        void start(FrameInfo &frame_info, const std::shared_ptr<Scene> &scene);
        void end(FrameInfo &frame_info);

        void resize(u32 _width, u32 _height);

        VkDescriptorSet& get_present_descriptor_set() { return vk_present_descriptor_set; }
        VkRenderPass get_renderpass() { return renderpass->vk_renderpass; }

    private:
        inline void create_images();
        inline void create_framebuffer();
        void write_composition_descriptor();

        bool first = true;
        u32 width, height;

        RenderPass* renderpass;

        Sampler* sampler;

        Framebuffer* framebuffer;
        FrameBufferAttachment *image;
        FrameBufferAttachment *depth;
        FrameBufferAttachment *albedo;
        FrameBufferAttachment *position; // remove this in future
        FrameBufferAttachment *normal;
        FrameBufferAttachment *metallic_roughness; // material ID in future
        FrameBufferAttachment *emissive;

        std::unique_ptr<DescriptorSetLayout> composition_descriptor_set_layout;
        VkDescriptorSetLayout vk_forward_pass_descriptor_set_layout;

        VkDescriptorSet vk_composition_descriptor_set;
        VkDescriptorSet vk_forward_pass_descriptor_set;
        VkDescriptorSet vk_present_descriptor_set;

        VkPipelineLayout vk_deferred_pipeline_layout;
        std::unique_ptr<Pipeline> deferred_pipeline;

        VkPipelineLayout vk_composition_pipeline_layout;
        std::unique_ptr<Pipeline> composition_pipeline;

        VkPipelineLayout vk_forward_pass_pipeline_layout;
        std::unique_ptr<Pipeline> forward_pass_pipeline;

        std::shared_ptr<Device> device;
    };
}