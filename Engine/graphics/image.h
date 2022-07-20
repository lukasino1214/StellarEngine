#pragma once

#include <cstdint>
#include <glm/glm.hpp>
#include "device.h"
#include "../pgepch.h"
#include "vk_types.h"


namespace Engine {
    struct ImageDescription {
        ImageFormat format;
        ImageType type;
        glm::ivec3 dimensions;
        ImageUsageFlags usage;
        uint32_t mip_levels = 1;
    };

    class Image {
    public:
        Image(std::shared_ptr<Engine::Device> _device, const ImageDescription& _description);
        ~Image();

        void transition_image_layout(VkImageLayout oldLayout, VkImageLayout newLayout);
        void generate_mipmaps();

        ImageFormat get_format() { return description.format; }

        VkImage vk_image = {};
    private:
        VkImageType get_image_type(const glm::ivec3& dim);

        VkDeviceMemory vk_device_memory = {}; // this will be hidden since there is usage for it outside of this class

        ImageDescription description;
        std::shared_ptr<Device> device;
    };

    struct SamplerDescription {
        Filter min_filter;
        Filter mag_filter;
        float max_anistropy = 1.0;
        SamplerAddressMode address_mode = SamplerAddressMode::REPEAT;
        uint32_t mipLevels = 1;
    };

    class Sampler {
    public:
        Sampler(std::shared_ptr<Engine::Device> _device, const SamplerDescription& _description);
        ~Sampler();

        VkSampler vk_sampler = {};
    private:
        SamplerDescription description;
        std::shared_ptr<Device> device;
    };

    struct ImageViewDescription {
        ImageViewType type = ImageViewType::TYPE_2D;
        ImageFormat format = ImageFormat::R8G8B8A8_UNORM;
        ComponentMapping swizzel_mapping = { ComponentSwizzle::R, ComponentSwizzle::G, ComponentSwizzle::B, ComponentSwizzle::A };
        ImageAspectFlags aspect_mask = ImageAspectFlagBits::COLOR;
        uint32_t mipLevels = 1;
        Image* image;
    };

    class ImageView {
    public:
        ImageView(std::shared_ptr<Device> _device, const ImageViewDescription& _description);
        ~ImageView();

        VkImageView vk_image_view = {};
    private:
        ImageViewDescription description;
        std::shared_ptr<Device> device;
    };
}