//
// Created by lukas on 14.04.22.
//

#ifndef ENGINEEDITOR_TEXTURE_H
#define ENGINEEDITOR_TEXTURE_H


#include <string>

#include <memory>
#include "Device.h"

namespace Engine {
    class Texture {
    public:
        Texture(const std::string &path);
        ~Texture();

        VkSampler GetSampler() { return m_Sampler; }
        VkImageView GetImageView() { return m_ImageView; }
        VkImageLayout GetImageLayout() { return m_ImageLayout; }

    private:
        void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

        std::shared_ptr<Device> m_Device;
        VkImage m_Image;
        VkDeviceMemory m_ImageMemory;
        VkImageView m_ImageView;
        VkSampler m_Sampler;
        VkFormat m_Format;
        VkImageLayout m_ImageLayout;

    };
}

#endif //ENGINEEDITOR_TEXTURE_H
