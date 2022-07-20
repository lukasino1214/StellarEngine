#include "descriptor_set.h"
#include "core.h"

namespace Engine {

// *************** Descriptor Set Layout Builder *********************

    DescriptorSetLayout::Builder& DescriptorSetLayout::Builder::add_binding(uint32_t binding, VkDescriptorType descriptor_type, VkShaderStageFlags stage_flags, uint32_t count) {
        assert(bindings.count(binding) == 0 && "Binding already in use");

        VkDescriptorSetLayoutBinding vk_descriptor_set_layout_binding = {
                .binding = binding,
                .descriptorType = descriptor_type,
                .descriptorCount = count,
                .stageFlags = stage_flags,
                .pImmutableSamplers = nullptr
        };

        bindings[binding] = vk_descriptor_set_layout_binding;
        return *this;
    }

    std::unique_ptr<DescriptorSetLayout> DescriptorSetLayout::Builder::build_unique() const {
        return std::make_unique<DescriptorSetLayout>(device, bindings);
    }

    std::shared_ptr<DescriptorSetLayout> DescriptorSetLayout::Builder::build_shared() const {
        return std::make_shared<DescriptorSetLayout>(device, bindings);
    }

// *************** Descriptor Set Layout *********************

    DescriptorSetLayout::DescriptorSetLayout(std::shared_ptr<Device> _device, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> _bindings) : bindings{_bindings}, device{_device} {
        std::vector<VkDescriptorSetLayoutBinding> vk_descriptor_set_layout_bindings = {};
        for (auto kv: bindings) {
            vk_descriptor_set_layout_bindings.push_back(kv.second);
        }

        VkDescriptorSetLayoutCreateInfo vk_descriptor_set_layout_create_info = {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .bindingCount = static_cast<uint32_t>(vk_descriptor_set_layout_bindings.size()),
                .pBindings = vk_descriptor_set_layout_bindings.data()
        };

        if (vkCreateDescriptorSetLayout(device->vk_device, &vk_descriptor_set_layout_create_info, nullptr, &vk_descriptor_set_layout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
    }

    DescriptorSetLayout::~DescriptorSetLayout() {
        vkDestroyDescriptorSetLayout(device->vk_device, vk_descriptor_set_layout, nullptr);
    }

// *************** Descriptor Pool Builder *********************

    DescriptorPool::Builder &DescriptorPool::Builder::add_pool_size(VkDescriptorType descriptor_type, uint32_t count) {
        pool_sizes.push_back({descriptor_type, count});
        return *this;
    }

    DescriptorPool::Builder &DescriptorPool::Builder::set_pool_flags(VkDescriptorPoolCreateFlags flags) {
        pool_flags = flags;
        return *this;
    }

    DescriptorPool::Builder &DescriptorPool::Builder::set_max_sets(uint32_t count) {
        max_sets = count;
        return *this;
    }

    std::unique_ptr<DescriptorPool> DescriptorPool::Builder::build_unique() const {
        return std::make_unique<DescriptorPool>(device, max_sets, pool_flags, pool_sizes);
    }

    std::shared_ptr<DescriptorPool> DescriptorPool::Builder::build_shared() const {
        return std::make_shared<DescriptorPool>(device, max_sets, pool_flags, pool_sizes);
    }

// *************** Descriptor Pool *********************

    DescriptorPool::DescriptorPool(std::shared_ptr<Device> _device, uint32_t max_sets, VkDescriptorPoolCreateFlags pool_flags, const std::vector<VkDescriptorPoolSize> &pool_sizes) : device{_device} {
        VkDescriptorPoolCreateInfo vk_descriptor_pool_create_info = {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
                .pNext = nullptr,
                .flags = pool_flags,
                .maxSets = max_sets,
                .poolSizeCount = static_cast<uint32_t>(pool_sizes.size()),
                .pPoolSizes = pool_sizes.data()
        };

        if (vkCreateDescriptorPool(device->vk_device, &vk_descriptor_pool_create_info, nullptr, &vk_descriptor_pool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor pool!");
        }
    }

    DescriptorPool::~DescriptorPool() {
        vkDestroyDescriptorPool(device->vk_device, vk_descriptor_pool, nullptr);
    }

    bool DescriptorPool::allocate_descriptor_set(const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet &descriptor_set) const {
        VkDescriptorSetAllocateInfo vk_descriptor_set_allocate_info = {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                .pNext = nullptr,
                .descriptorPool = vk_descriptor_pool,
                .descriptorSetCount = 1,
                .pSetLayouts = &descriptorSetLayout,
        };

        if (vkAllocateDescriptorSets(device->vk_device, &vk_descriptor_set_allocate_info, &descriptor_set) != VK_SUCCESS) {
            return false;
        }
        return true;
    }

    void DescriptorPool::free_descriptor_sets(std::vector<VkDescriptorSet> &descriptors) const {
        vkFreeDescriptorSets(device->vk_device, vk_descriptor_pool, static_cast<uint32_t>(descriptors.size()), descriptors.data());
    }

    void DescriptorPool::reset_pool() {
        vkResetDescriptorPool(device->vk_device, vk_descriptor_pool, 0);
    }

// *************** Descriptor Writer *********************

    DescriptorWriter::DescriptorWriter(DescriptorSetLayout &_descriptor_set_layout, DescriptorPool &_descriptor_pool) : descriptor_set_layout{_descriptor_set_layout}, descriptor_pool{_descriptor_pool} {}

    DescriptorWriter &DescriptorWriter::write_buffer(uint32_t binding, VkDescriptorBufferInfo *buffer_info) {
        assert(descriptor_set_layout.bindings.count(binding) == 1 && "Layout does not contain specified binding");

        auto &binding_description = descriptor_set_layout.bindings[binding];

        assert(binding_description.descriptorCount == 1 && "Binding single descriptor info, but binding expects multiple");

        VkWriteDescriptorSet vk_write_descriptor_set = {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .pNext = nullptr,
                .dstSet = {},
                .dstBinding = binding,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = binding_description.descriptorType,
                .pImageInfo = nullptr,
                .pBufferInfo = buffer_info,
                .pTexelBufferView = nullptr
        };

        writes.push_back(vk_write_descriptor_set);
        return *this;
    }

    DescriptorWriter &DescriptorWriter::write_image(uint32_t binding, VkDescriptorImageInfo *image_info) {
        assert(descriptor_set_layout.bindings.count(binding) == 1 && "Layout does not contain specified binding");

        auto &binding_description = descriptor_set_layout.bindings[binding];

        assert(binding_description.descriptorCount == 1 && "Binding single descriptor info, but binding expects multiple");

        VkWriteDescriptorSet vk_write_descriptor_set = {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .pNext = nullptr,
                .dstSet = {},
                .dstBinding = binding,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = binding_description.descriptorType,
                .pImageInfo = image_info,
                .pBufferInfo = nullptr,
                .pTexelBufferView = nullptr
        };

        writes.push_back(vk_write_descriptor_set);
        return *this;
    }

    bool DescriptorWriter::build(std::shared_ptr<Device> device, VkDescriptorSet &descriptor_set) {
        bool success = descriptor_pool.allocate_descriptor_set(descriptor_set_layout.get_descriptor_set_layout(), descriptor_set);
        if (!success) {
            return false;
        }
        overwrite(device, descriptor_set);
        return true;
    }

    void DescriptorWriter::overwrite(std::shared_ptr<Device> device, VkDescriptorSet &descriptor_set) {
        for (auto &write: writes) {
            write.dstSet = descriptor_set;
        }
        vkUpdateDescriptorSets(device->vk_device, writes.size(), writes.data(), 0, nullptr);
    }

}