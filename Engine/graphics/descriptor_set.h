#pragma once

#include "device.h"
#include "../pgepch.h"

namespace Engine {
    class DescriptorSetLayout {
    public:
        class Builder {
        public:
            Builder(std::shared_ptr<Device> _device) : device{_device} {}

            Builder &add_binding(uint32_t binding, VkDescriptorType descriptor_type, VkShaderStageFlags stage_flags, uint32_t count = 1);

            std::unique_ptr<DescriptorSetLayout> build_unique() const;
            std::shared_ptr<DescriptorSetLayout> build_shared() const;

        private:
            std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};
            std::shared_ptr<Device> device;
        };

        DescriptorSetLayout(std::shared_ptr<Device> _device, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> _bindings);
        ~DescriptorSetLayout();

        DescriptorSetLayout(const DescriptorSetLayout &) = delete;
        DescriptorSetLayout &operator=(const DescriptorSetLayout &) = delete;

        VkDescriptorSetLayout get_descriptor_set_layout() const { return vk_descriptor_set_layout; }

    private:
        VkDescriptorSetLayout vk_descriptor_set_layout;
        std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;
        std::shared_ptr<Device> device;

        friend class DescriptorWriter;
    };

    class DescriptorPool {
    public:
        class Builder {
        public:
            Builder(std::shared_ptr<Device> _device) : device{_device} {}

            Builder &add_pool_size(VkDescriptorType descriptor_type, uint32_t count);
            Builder &set_pool_flags(VkDescriptorPoolCreateFlags flags);
            Builder &set_max_sets(uint32_t count);

            std::unique_ptr<DescriptorPool> build_unique() const;
            std::shared_ptr<DescriptorPool> build_shared() const;

        private:
            std::vector<VkDescriptorPoolSize> pool_sizes{};
            uint32_t max_sets = 1000;
            VkDescriptorPoolCreateFlags pool_flags = 0;
            std::shared_ptr<Device> device;
        };

        DescriptorPool(std::shared_ptr<Device> _device, uint32_t max_sets, VkDescriptorPoolCreateFlags pool_flags, const std::vector<VkDescriptorPoolSize> &pool_sizes);
        ~DescriptorPool();

        DescriptorPool(const DescriptorPool &) = delete;
        DescriptorPool &operator=(const DescriptorPool &) = delete;

        bool allocate_descriptor_set(const VkDescriptorSetLayout descriptor_set_layout, VkDescriptorSet &descriptor_set) const;
        void free_descriptor_sets(std::vector<VkDescriptorSet> &descriptors) const;

        void reset_pool();

    private:
        VkDescriptorPool vk_descriptor_pool;
        std::shared_ptr<Device> device;

        friend class DescriptorWriter;
    };

    class DescriptorWriter {
    public:
        DescriptorWriter(DescriptorSetLayout &_descriptor_set_layout, DescriptorPool &_descriptor_pool);
        DescriptorWriter &write_buffer(uint32_t binding, VkDescriptorBufferInfo *buffer_info);
        DescriptorWriter &write_image(uint32_t binding, VkDescriptorImageInfo *image_info);

        bool build(std::shared_ptr<Device> device, VkDescriptorSet &descriptor_set);

        void overwrite(std::shared_ptr<Device> device, VkDescriptorSet &descriptor_set);

    private:
        DescriptorSetLayout &descriptor_set_layout;
        DescriptorPool &descriptor_pool;
        std::vector<VkWriteDescriptorSet> writes;
    };

}