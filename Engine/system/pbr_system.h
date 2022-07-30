#pragma once

#include "../pgepch.h"
#include "../graphics/device.h"
#include "../graphics/renderpass.h"
#include "../graphics/pipeline.h"
#include "../graphics/model.h"

namespace Engine {
    class PBRSystem {
    public:
        PBRSystem(std::shared_ptr<Device> _device, VkRenderPass renderpass);
        ~PBRSystem();

        void render_skybox(FrameInfo frame_info);

        VkDescriptorSet vk_BRDFLUT_descriptor_set;
        VkDescriptorSet hdr_set;
        VkDescriptorSet environment_map_set;
        VkDescriptorSet irradiance_cube_set;
        VkDescriptorSet prefiltered_cube_set;

        VkSampler get_sampler() { return sampler->vk_sampler; }
        VkImageView get_BRDFLUT_image_view() { return BRDFLUT->image_view->vk_image_view; };
        VkImageView get_irradiance_image_view() { return irradiance_cube_image_view->vk_image_view; };
        VkImageView get_prefiltered_map_image_view() { return prefiltered_cube_image_view->vk_image_view; };

    private:
        void generate_BRDFLUT();
        void generate_environment_map();
        void generate_irradiance_cube();
        void generate_prefiltered_cube();

        std::unique_ptr<Model> cube;

        Sampler *sampler;

        FrameBufferAttachment *BRDFLUT; // holds image and image view of Look Up Table for BDRF
        Image *environment_cube_image;
        ImageView *environment_cube_image_view;

        Image *irradiance_cube_image;
        ImageView *irradiance_cube_image_view;

        Image *prefiltered_cube_image;
        ImageView *prefiltered_cube_image_view;

        Image *hdr_image;
        ImageView *hdr_image_view;

        VkPipelineLayout vk_skybox_pipeline_layout;
        std::unique_ptr<Pipeline> skybox_pipeline;

        std::shared_ptr<Device> device;
    };
}
