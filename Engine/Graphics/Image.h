#pragma once

#include <cstdint>
#include <glm/glm.hpp>
#include "Device.h"
#include "../pgepch.h"

namespace Engine {
    enum Format {
        UNDEFINED = VK_FORMAT_UNDEFINED,
        RGBA8_UNORM = VK_FORMAT_R8G8B8A8_UNORM,
        BGRA8_SRGB = VK_FORMAT_B8G8R8A8_SRGB,
        D16 = VK_FORMAT_D16_UNORM,
        D32 = VK_FORMAT_D32_SFLOAT,
        S8 = VK_FORMAT_S8_UINT,
        D16S8 = VK_FORMAT_D16_UNORM_S8_UINT,
        D24S8 = VK_FORMAT_D24_UNORM_S8_UINT,
        D32S8 = VK_FORMAT_D32_SFLOAT_S8_UINT,
    };

    enum UsageFlags {
        TRANSFER_SRC = 0x00000001,
        TRANSFER_DST = 0x00000002,
        SAMPLED = 0x00000004,
        STORAGE = 0x00000008,
        COLOR_ATTACHMENT = 0x00000010,
        DEPTH_STENCIL_ATTACHMENT = 0x00000020,
        TRANSIENT_ATTACHMENT = 0x00000040,
        INPUT_ATTACHMENT = 0x00000080
    };

    typedef uint32_t Usage;

    enum Type {
        TYPE_1D = 0,
        TYPE_2D = 1,
        TYPE_3D = 2,
        TYPE_CUBE = 3,
        TYPE_1D_ARRAY = 4,
        TYPE_2D_ARRAY = 5,
        TYPE_CUBE_ARRAY = 6,
    };

    struct ImageDecs {
        Format format;
        Type type;
        glm::ivec3 dimensions;
        Usage usage;
        uint32_t mipLevels = 1;
    };

    class Image {
    public:
        Image(std::shared_ptr<Engine::Device> device, const ImageDecs& decs);
        ~Image();

        void TransitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout);
        void GenerateMipmaps();

        VkImage GetImage() { return VK_Image; }

    private:
        VkImage VK_Image = VK_NULL_HANDLE;
        VkDeviceMemory VK_Memory = VK_NULL_HANDLE;
        ImageDecs m_Decs;

        VkImageType getImageType(const glm::ivec3& dim);

        std::shared_ptr<Engine::Device> m_Device;
    };

    enum Filter {
        NEAREST = VK_FILTER_NEAREST,
        LINEAR = VK_FILTER_LINEAR
    };

    enum AddressMode {
        REPEAT = 0,
        MIRRORED_REPEAT = 1,
        CLAMP_TO_EDGE = 2,
        CLAMP_TO_BORDER = 3,
        MIRROR_CLAMP_TO_EDGE = 4
    };

    struct SamplerDecs {
        Filter minFilter;
        Filter magFilter;
        float maxAnistropy = 1.0;
        AddressMode addressMode = AddressMode::REPEAT;
        uint32_t mipLevels = 1;
    };

    class Sampler {
    public:
        Sampler(std::shared_ptr<Engine::Device> device, const SamplerDecs& decs);
        ~Sampler();

        VkSampler GetSampler() { return VK_Sampler; }
        VkSampler VK_Sampler;
        std::shared_ptr<Device> m_Device;
    };

    enum Swizzle {
        IDENTITY = 0,
        ZERO = 1,
        ONE = 2,
        R = 3,
        G = 4,
        B = 5,
        A = 6,
    };

    struct SwizzelMapping {
        Swizzle r;
        Swizzle g;
        Swizzle b;
        Swizzle a;

        operator VkComponentMapping const&() const noexcept {
            return *reinterpret_cast<const VkComponentMapping*>(this);
        }

        operator VkComponentMapping&() noexcept {
            return *reinterpret_cast<VkComponentMapping*>(this);
        }
    };

    enum Aspect {
        NONE = 0,
        COLOR = 0x00000001,
        DEPTH = 0x00000002,
        STENCIL = 0x00000004,
    };

    struct ImageViewDecs {
        Type type = Type::TYPE_2D;
        Format format = Format::RGBA8_UNORM;
        SwizzelMapping swizzelMapping = { Swizzle::R, Swizzle::G, Swizzle::B, Swizzle::A };
        Aspect aspectMask = Aspect::COLOR;
        uint32_t mipLevels = 1;
        Image* image;
    };

    class ImageView {
    public:
        ImageView(std::shared_ptr<Engine::Device> device, const ImageViewDecs& decs);
        ~ImageView();

        VkImageView& GetImageView() { return VK_ImageView; }
        VkImageView VK_ImageView;
        std::shared_ptr<Device> m_Device;
    };
}