#include "image.h"
#include <cstddef>
#include <vulkan/vulkan_core.h>

namespace Engine {
    Image::Image(std::shared_ptr<Device> _device, const ImageDescription& _description) : device{_device}, description{_description}  {
        VkImageCreateInfo vk_image_create_info = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .imageType = get_image_type(description.dimensions),
            .format = (VkFormat)description.format,
            .extent = {
                .width = static_cast<uint32_t>(description.dimensions.x),
                .height = static_cast<uint32_t>(description.dimensions.y),
                .depth = static_cast<uint32_t>(description.dimensions.z)
            },
            .mipLevels = description.mip_levels,
            .arrayLayers = 1,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .usage = description.usage,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 0, // TODO: change this ???
            .pQueueFamilyIndices = nullptr, // TODO: change this ???
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
        };

        if (vkCreateImage(device->vk_device, &vk_image_create_info, nullptr, &vk_image) != VK_SUCCESS) {
            throw std::runtime_error("failed to create image!");
        }

        VkMemoryRequirements memory_requirements;
        vkGetImageMemoryRequirements(device->vk_device, vk_image, &memory_requirements);

        VkMemoryAllocateInfo memory_allocate_info = {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .pNext = nullptr,
            .allocationSize = memory_requirements.size,
            .memoryTypeIndex = device->find_memory_type(memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
        };

        if (vkAllocateMemory(device->vk_device, &memory_allocate_info, nullptr, &vk_device_memory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate image memory!");
        }

        if (vkBindImageMemory(device->vk_device, vk_image, vk_device_memory, 0) != VK_SUCCESS) {
            throw std::runtime_error("failed to bind image memory!");
        }
    }

    VkImageType Image::get_image_type(const glm::ivec3& dim) {
        if(dim.x != 1 && dim.y != 1 && dim.z != 1) {
            return VK_IMAGE_TYPE_3D;
        }

        if(dim.x != 1 && dim.y != 1 && dim.z == 1) {
            return VK_IMAGE_TYPE_2D;
        }

        return VK_IMAGE_TYPE_1D;
    }

    void Image::transition_image_layout(VkImageLayout oldLayout, VkImageLayout newLayout) {
        VkCommandBuffer command_buffer = device->begin_single_time_command_buffer();

        VkImageMemoryBarrier vk_image_memory_barrier = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .pNext = nullptr,
            .srcAccessMask = 0,
            .dstAccessMask = 0,
            .oldLayout = oldLayout,
            .newLayout = newLayout,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = vk_image,
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = description.mip_levels,
                .baseArrayLayer = 0,
                .layerCount = 1
            }
        };

        VkPipelineStageFlags source_stage;
        VkPipelineStageFlags destination_stage;

        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            vk_image_memory_barrier.srcAccessMask = 0;
            vk_image_memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            vk_image_memory_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            vk_image_memory_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        } else {
            std::cout << "fuck" << std::endl;
        }

        vkCmdPipelineBarrier(command_buffer, source_stage, destination_stage, 0, 0, nullptr, 0, nullptr, 1, &vk_image_memory_barrier);

        device->end_single_time_command_buffer(command_buffer);
    }

    void Image::generate_mipmaps() {
        VkFormatProperties format_properties;
        vkGetPhysicalDeviceFormatProperties(device->vk_physical_device, (VkFormat)description.format, &format_properties);

        if (!(format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
            throw std::runtime_error("texture image format does not support linear blitting!");
        }

        VkCommandBuffer command_buffer = device->begin_single_time_command_buffer();

        VkImageMemoryBarrier vk_image_memory_barrier = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .pNext = nullptr,
            .srcAccessMask = 0,
            .dstAccessMask = 0,
            .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .newLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = vk_image,
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1
            }
        };

        int32_t mip_width = description.dimensions.x;
        int32_t mip_height = description.dimensions.y;

        for (uint32_t i = 1; i < description.mip_levels; i++) {
            vk_image_memory_barrier.subresourceRange.baseMipLevel = i - 1;
            vk_image_memory_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            vk_image_memory_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            vk_image_memory_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            vk_image_memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

            vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &vk_image_memory_barrier);

            VkImageBlit vk_image_blit = {
                .srcSubresource = {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .mipLevel = i - 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                },
                .srcOffsets = {
                    { 0, 0, 0 },
                    { mip_width, mip_height, 1 }
                },
                .dstSubresource = {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .mipLevel = i,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                },
                .dstOffsets = {
                    { 0, 0, 0 },
                    { mip_width > 1 ? mip_width / 2 : 1, mip_height > 1 ? mip_height / 2 : 1, 1 }
                }
            };

            vkCmdBlitImage(command_buffer, vk_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, vk_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &vk_image_blit, VK_FILTER_LINEAR);

            vk_image_memory_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            vk_image_memory_barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            vk_image_memory_barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            vk_image_memory_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &vk_image_memory_barrier);

            if (mip_width > 1) mip_width /= 2;
            if (mip_height > 1) mip_height /= 2;
        }

        vk_image_memory_barrier.subresourceRange.baseMipLevel = description.mip_levels - 1;
        vk_image_memory_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        vk_image_memory_barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        vk_image_memory_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        vk_image_memory_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &vk_image_memory_barrier);

        device->end_single_time_command_buffer(command_buffer);
    }

    Image::~Image() {
        auto vk_device = device->vk_device;
        vkDestroyImage(vk_device, vk_image, nullptr);
        vkFreeMemory(vk_device, vk_device_memory, nullptr);
    }

    Sampler::Sampler(std::shared_ptr<Device> _device, const SamplerDescription& _description) : device{_device}, description{_description} {
        VkSamplerCreateInfo vk_sampler_create_info = {
            .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .magFilter = (VkFilter)description.mag_filter,
            .minFilter = (VkFilter)description.min_filter,
            .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR, // TODO: change this ???
            .addressModeU = (VkSamplerAddressMode)description.address_mode,
            .addressModeV = (VkSamplerAddressMode)description.address_mode,
            .addressModeW = (VkSamplerAddressMode)description.address_mode,
            .mipLodBias = 0.0f,
            .anisotropyEnable = VK_TRUE,
            .maxAnisotropy = description.max_anistropy,
            .compareEnable = VK_FALSE,
            .compareOp = VK_COMPARE_OP_NEVER,
            .minLod = 0.0f,
            .maxLod = static_cast<float>(description.mipLevels),
            .borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,
            .unnormalizedCoordinates = VK_FALSE
        };

        if(vkCreateSampler(device->vk_device, &vk_sampler_create_info, nullptr, &vk_sampler) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create sampler");
        }
    }

    Sampler::~Sampler() {
        vkDestroySampler(device->vk_device, vk_sampler, nullptr);
    }

    ImageView::ImageView(std::shared_ptr<Device> _device, const ImageViewDescription &_description) : device{_device}, description{_description} {
        VkImageViewCreateInfo vk_image_view_create_info = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .pNext = nullptr,
            .flags = {},
            .image = description.image->vk_image,
            .viewType = (VkImageViewType)description.type,
            .format = (VkFormat)description.format,
            .components = description.swizzel_mapping,
            .subresourceRange = {
                .aspectMask = description.aspect_mask,
                .baseMipLevel = 0,
                .levelCount = description.mipLevels,
                .baseArrayLayer = 0,
                .layerCount = 1
            }
        };

        if(vkCreateImageView(device->vk_device, &vk_image_view_create_info, nullptr, &vk_image_view) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create image view");
        }
    }

    ImageView::~ImageView() {
        vkDestroyImageView(device->vk_device, vk_image_view, nullptr);
    }
}