#pragma once

#include "device.h"
#include "../pgepch.h"

namespace Engine {
    class Buffer {
    public:
        Buffer(std::shared_ptr<Device> _device, VkDeviceSize _instance_size, uint32_t _instance_count, VkBufferUsageFlags _usage_flags, MemoryFlags _memory_property_flags, VkDeviceSize min_offset_alignment = 1);
        ~Buffer();

        Buffer(const Buffer &) = delete;
        Buffer &operator=(const Buffer &) = delete;

        VkResult map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
        void unmap();

        void write_to_buffer(void* data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
        VkResult flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
        VkDescriptorBufferInfo get_descriptor_info(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
        VkResult invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

        void write_to_index(void* data, int index);
        VkResult flush_index(int index);
        VkDescriptorBufferInfo get_descriptor_info_for_index(int index);
        VkResult invalidate_index(int index);

        VkBuffer get_buffer() const { return vk_buffer; }
        void* get_mapped_memory() const { return mapped; }
        uint32_t get_instance_count() const { return instance_count; }
        VkDeviceSize get_instance_size() const { return instance_size; }
        VkDeviceSize get_alignment_size() const { return alignment_size; }
        VkBufferUsageFlags get_usage_flags() const { return usage_flags; }
        MemoryFlags get_memory_property_flags() const { return memory_property_flags; }
        VkDeviceSize get_buffersize() const { return buffer_size; }

    private:
        static VkDeviceSize get_alignment(VkDeviceSize _instance_size, VkDeviceSize min_offset_alignment);

        void *mapped = nullptr;
        VkBuffer vk_buffer = {};
        //VkDeviceMemory vk_device_memory = {};
        VmaAllocation vma_allocation = {};

        VkDeviceSize buffer_size;
        uint32_t instance_count;
        VkDeviceSize instance_size;
        VkDeviceSize alignment_size;
        VkBufferUsageFlags usage_flags;
        MemoryFlags memory_property_flags;
        std::shared_ptr<Device> device;
    };

}