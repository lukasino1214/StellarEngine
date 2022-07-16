//
// Created by lukas on 14.04.22.
//

#ifndef ENGINEEDITOR_TEXTURE_H
#define ENGINEEDITOR_TEXTURE_H


#include <string>

#include "../pgepch.h"
#include "Device.h"
#include "Image.h"

namespace Engine {
    class Texture {
    public:
        Texture(std::shared_ptr<Device> device, const std::string &path);
        ~Texture();

        Texture(const Texture &) = delete;
        Texture &operator=(const Texture &) = delete;
        Texture(Texture &&) = delete;
        Texture &operator=(Texture &&) = delete;

        VkSampler GetSampler() { return m_Sampler->GetSampler(); }
        VkImageView GetImageView() { return m_ImageView->GetImageView(); }
        VkImageLayout GetImageLayout() { return m_ImageLayout; }

    private:
        std::shared_ptr<Device> m_Device;
        Image* m_Image;
        ImageView* m_ImageView;
        Sampler* m_Sampler;
        VkFormat m_Format;
        VkImageLayout m_ImageLayout;
        uint32_t m_MipLevels;
    };
}

#endif //ENGINEEDITOR_TEXTURE_H
