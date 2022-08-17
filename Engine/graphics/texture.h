#pragma once

#include <string>

#include "../pgepch.h"
#include "device.h"
#include "image.h"

namespace Engine {
    class Texture {
    public:
        Texture(std::shared_ptr<Device> _device, const std::string &filepath, ImageFormat format = ImageFormat::R8G8B8A8_UNORM, int components = 4);
        ~Texture();

        Texture(const Texture &) = delete;
        Texture &operator=(const Texture &) = delete;
        Texture(Texture &&) = delete;
        Texture &operator=(Texture &&) = delete;

        VkSampler get_sampler() { return sampler->vk_sampler; }
        VkImageView get_image_view() { return image_view->vk_image_view; }
        VkImageLayout get_image_layout() { return vk_image_layout; }

        VkDescriptorImageInfo get_descriptor_image_info();

        VkImageLayout vk_image_layout;
    private:
        std::shared_ptr<Device> device;
        Image* image;
        ImageView* image_view;
        Sampler* sampler;
        VkFormat vk_format;
    };
}