#include "texture.h"
#include "buffer.h"
#include "vk_types.h"

#include <stb_image.h>

namespace Engine {
    Texture::Texture(std::shared_ptr<Device> _device, const std::string &path, ImageFormat format, int components) : device{_device} {
        int width, height, channels, m_BytesPerPixel;

        stbi_set_flip_vertically_on_load(0);
        stbi_uc* data = stbi_load(path.c_str(), &width, &height, &m_BytesPerPixel, components);

        u32 mip_levels = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;

        Buffer stagingBuffer{device, 4, static_cast<uint32_t>(width * height), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, MemoryFlagBits::HOST_ACCESS_SEQUENTIAL_WRITE};

        stagingBuffer.map();
        stagingBuffer.write_to_buffer(data);

        vk_format = (VkFormat)format;

        image = new Image(device, {
                .format = format,
                .dimensions = { width, height, 1 },
                .usage = ImageUsageFlagBits::TRANSFER_SRC | ImageUsageFlagBits::TRANSFER_DST | ImageUsageFlagBits::SAMPLED,
                .mip_levels = mip_levels
        });

        image->transition_image_layout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        device->copy_buffer_to_image(stagingBuffer.get_buffer(), image->vk_image, static_cast<uint>(width), static_cast<uint>(height), 1);
        vk_image_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        image->generate_mipmaps();

        sampler = new Sampler(device, {
            .min_filter = Filter::LINEAR,
            .mag_filter = Filter::LINEAR,
            .max_anistropy = 4.0,
            .mipLevels = mip_levels
        });

        image_view = new ImageView(device, {
            .mip_levels = mip_levels,
            .image = image
        });

        stbi_image_free(data);
    }

    Texture::~Texture() {
        delete image;
        delete image_view;
        delete sampler;
    }

    VkDescriptorImageInfo Texture::get_descriptor_image_info() {
        return VkDescriptorImageInfo {
                .sampler = sampler->vk_sampler,
                .imageView = image_view->vk_image_view,
                .imageLayout = vk_image_layout
        };
    }
}