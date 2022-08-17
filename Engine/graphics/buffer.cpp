#include "buffer.h"

#include <utility>

namespace Engine {
    VkDeviceSize Buffer::get_alignment(VkDeviceSize _instance_size, VkDeviceSize min_offset_alignment) {
        if (min_offset_alignment > 0) {
            return (_instance_size + min_offset_alignment - 1) & ~(min_offset_alignment - 1);
        }
        return _instance_size;
    }

    Buffer::Buffer(std::shared_ptr<Device> _device, VkDeviceSize _instance_size, uint32_t _instance_count, VkBufferUsageFlags _usage_flags, MemoryFlags _memory_property_flags, VkDeviceSize min_offset_alignment)
            : instance_size{_instance_size},
              instance_count{_instance_count},
              usage_flags{_usage_flags},
              memory_property_flags{_memory_property_flags},
              device{std::move(_device)} {
        alignment_size = get_alignment(instance_size, min_offset_alignment);
        buffer_size = alignment_size * instance_count;
        device->create_buffer(buffer_size, usage_flags, memory_property_flags, vk_buffer, vma_allocation);
    }

    Buffer::~Buffer() {
        unmap();
        vmaDestroyBuffer(device->vma_allocator, vk_buffer, vma_allocation);
    }

    VkResult Buffer::map(VkDeviceSize size, VkDeviceSize offset) {
        // TODO: size and offset
        return vmaMapMemory(device->vma_allocator, vma_allocation, &mapped);
    }

    void Buffer::unmap() {
        if (mapped) {
            vmaUnmapMemory(device->vma_allocator, vma_allocation);
            mapped = nullptr;
        }
    }

    void Buffer::write_to_buffer(void *data, VkDeviceSize size, VkDeviceSize offset) {
        assert(mapped && "Cannot copy to unmapped buffer");

        if (size == VK_WHOLE_SIZE) {
            memcpy(mapped, data, buffer_size);
        } else {
            char *mem_offset = (char *) mapped;
            mem_offset += offset;
            memcpy(mem_offset, data, size);
        }
    }

    VkResult Buffer::flush(VkDeviceSize size, VkDeviceSize offset) {
        return vmaFlushAllocation(device->vma_allocator, vma_allocation, offset, size);
    }

    VkResult Buffer::invalidate(VkDeviceSize size, VkDeviceSize offset) {
        return vmaInvalidateAllocation(device->vma_allocator, vma_allocation, offset, size);
    }

    VkDescriptorBufferInfo Buffer::get_descriptor_info(VkDeviceSize size, VkDeviceSize offset) {
        return VkDescriptorBufferInfo { vk_buffer, offset, size };
    }

    void Buffer::write_to_index(void *data, int index) {
        write_to_buffer(data, instance_size, index * alignment_size);
    }

    VkResult Buffer::flush_index(int index) { return flush(alignment_size, index * alignment_size); }

    VkDescriptorBufferInfo Buffer::get_descriptor_info_for_index(int index) {
        return get_descriptor_info(alignment_size, index * alignment_size);
    }

    VkResult Buffer::invalidate_index(int index) {
        return invalidate(alignment_size, index * alignment_size);
    }

}
