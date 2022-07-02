#include "Image.h"

namespace Engine {
    Image::Image(std::shared_ptr<Engine::Device> device, const ImageDecs& decs) : m_Device{device}, m_Decs{decs}  {
        VkImageCreateInfo imageCreateInfo = {};
        imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageCreateInfo.imageType = getImageType(decs.dimensions);
        imageCreateInfo.format = (VkFormat)decs.format;
        imageCreateInfo.extent.width = decs.dimensions.x;
        imageCreateInfo.extent.height = decs.dimensions.y;
        imageCreateInfo.extent.depth = decs.dimensions.z;
        imageCreateInfo.mipLevels = decs.mipLevels;
        imageCreateInfo.arrayLayers = 1;
        imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageCreateInfo.usage = decs.usage;


        if (vkCreateImage(device->device(), &imageCreateInfo, nullptr, &VK_Image) != VK_SUCCESS) {
            throw std::runtime_error("failed to create image!");
        }

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(device->device(), VK_Image, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = device->findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        if (vkAllocateMemory(device->device(), &allocInfo, nullptr, &VK_Memory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate image memory!");
        }

        if (vkBindImageMemory(device->device(), VK_Image, VK_Memory, 0) != VK_SUCCESS) {
            throw std::runtime_error("failed to bind image memory!");
        }
    }

    VkImageType Image::getImageType(const glm::ivec3& dim) {
        if(dim.x != 1 && dim.y != 1 && dim.z != 1) {
            return VK_IMAGE_TYPE_3D;
        }

        if(dim.x != 1 && dim.y != 1 && dim.z == 1) {
            return VK_IMAGE_TYPE_2D;
        }

        return VK_IMAGE_TYPE_1D;
    }

    void Image::TransitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout) {
        VkCommandBuffer commandBuffer = m_Device->beginSingleTimeCommands();

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = VK_Image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = m_Decs.mipLevels;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;

        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
                   newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        } else {
            std::cout << "fuck" << std::endl;
        }

        vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

        m_Device->endSingleTimeCommands(commandBuffer);
    }

    void Image::GenerateMipmaps() {
        VkFormatProperties formatProperties;
        vkGetPhysicalDeviceFormatProperties(m_Device->getPhysicalDevice(), (VkFormat)m_Decs.format, &formatProperties);

        if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
            throw std::runtime_error("texture image format does not support linear blitting!");
        }

        VkCommandBuffer commandBuffer = m_Device->beginSingleTimeCommands();

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.image = VK_Image;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
        barrier.subresourceRange.levelCount = 1;

        int32_t mipWidth = m_Decs.dimensions.x;
        int32_t mipHeight = m_Decs.dimensions.y;

        for (uint32_t i = 1; i < m_Decs.mipLevels; i++) {
            barrier.subresourceRange.baseMipLevel = i - 1;
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

            vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

            VkImageBlit blit{};
            blit.srcOffsets[0] = {0, 0, 0};
            blit.srcOffsets[1] = {mipWidth, mipHeight, 1};
            blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.srcSubresource.mipLevel = i - 1;
            blit.srcSubresource.baseArrayLayer = 0;
            blit.srcSubresource.layerCount = 1;
            blit.dstOffsets[0] = {0, 0, 0};
            blit.dstOffsets[1] = {mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1};
            blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.dstSubresource.mipLevel = i;
            blit.dstSubresource.baseArrayLayer = 0;
            blit.dstSubresource.layerCount = 1;

            vkCmdBlitImage(commandBuffer, VK_Image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);

            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

            if (mipWidth > 1) mipWidth /= 2;
            if (mipHeight > 1) mipHeight /= 2;
        }

        barrier.subresourceRange.baseMipLevel = m_Decs.mipLevels - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

        m_Device->endSingleTimeCommands(commandBuffer);
    }

    Image::~Image() {
        auto VK_Device = m_Device->device();
        vkDestroyImage(VK_Device, VK_Image, nullptr);
        vkFreeMemory(VK_Device, VK_Memory, nullptr);
    }

    Sampler::Sampler(std::shared_ptr<Engine::Device> device, const SamplerDecs& decs) : m_Device(device) {
        VkSamplerCreateInfo sampler{};
        sampler.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        sampler.magFilter = (VkFilter)decs.magFilter;
        sampler.minFilter = (VkFilter)decs.minFilter;
        sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        sampler.addressModeU = (VkSamplerAddressMode)decs.addressMode;
        sampler.addressModeV = (VkSamplerAddressMode)decs.addressMode;
        sampler.addressModeW = (VkSamplerAddressMode)decs.addressMode;
        sampler.mipLodBias = 0.0f;
        sampler.compareOp = VK_COMPARE_OP_NEVER;
        sampler.minLod = 0.0f;
        sampler.maxLod = static_cast<float>(decs.mipLevels);
        sampler.maxAnisotropy = decs.maxAnistropy;
        sampler.anisotropyEnable = VK_TRUE;
        sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

        vkCreateSampler(m_Device->device(), &sampler, nullptr, &VK_Sampler);
    }

    Sampler::~Sampler() {
        vkDestroySampler(m_Device->device(), VK_Sampler, nullptr);
    }

    ImageView::ImageView(std::shared_ptr<Engine::Device> device, const ImageViewDecs &decs) : m_Device{device} {
        VkImageViewCreateInfo view{};
        view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        view.viewType = (VkImageViewType)decs.type;
        view.format = (VkFormat)decs.format;
        view.components = decs.swizzelMapping;
        view.subresourceRange.aspectMask = decs.aspectMask;
        view.subresourceRange.baseMipLevel = 0;
        view.subresourceRange.baseArrayLayer = 0;
        view.subresourceRange.layerCount = 1;
        view.subresourceRange.levelCount = decs.mipLevels;
        view.image = decs.image->GetImage();

        vkCreateImageView(m_Device->device(), &view, nullptr, &VK_ImageView);
    }

    ImageView::~ImageView() {
        vkDestroyImageView(m_Device->device(), VK_ImageView, nullptr);
    }
}