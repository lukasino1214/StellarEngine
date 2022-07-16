//
// Created by lukas on 14.04.22.
//

#include "Texture.h"
#include "Buffer.h"
#include "Core.h"

#define STB_IMAGE_IMPLEMENTATION

#include <stb_image.h>
#include <iostream>

namespace Engine {
    Texture::Texture(std::shared_ptr<Device> device, const std::string &path) : m_Device{device} {
        int width, height, channels, m_BytesPerPixel;

        stbi_uc* data = stbi_load(path.c_str(), &width, &height, &m_BytesPerPixel, 4);

        m_MipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;

        Buffer stagingBuffer{m_Device, 4, static_cast<uint32_t>(width * height), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};

        stagingBuffer.map();
        stagingBuffer.writeToBuffer(data);

        m_Format = VK_FORMAT_R8G8B8A8_UNORM;

        m_Image = new Image(m_Device, {
                .format = Format::R8G8B8A8_UNORM,
                .dimensions = { width, height, 1 },
                .usage = UsageFlags::TRANSFER_SRC | UsageFlags::TRANSFER_DST | UsageFlags::SAMPLED,
                .mipLevels = m_MipLevels
        });

        m_Image->TransitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        m_Device->copyBufferToImage(stagingBuffer.getBuffer(), m_Image->GetImage(), static_cast<uint>(width), static_cast<uint>(height), 1);
        m_ImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        m_Image->GenerateMipmaps();

        m_Sampler = new Sampler(m_Device, {
                .minFilter = Filter::LINEAR,
                .magFilter = Filter::LINEAR,
                .maxAnistropy = 4.0,
                .mipLevels = m_MipLevels,
        });

        m_ImageView = new ImageView(m_Device, {
            .mipLevels = m_MipLevels,
            .image = m_Image
        });

        stbi_image_free(data);
    }

    Texture::~Texture() {
        delete m_Image;
        delete m_ImageView;
        delete m_Sampler;
    }
}