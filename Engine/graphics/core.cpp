#include "core.h"

namespace Engine {
    std::shared_ptr<DescriptorPool> Core::global_descriptor_pool;
    std::shared_ptr<DescriptorSetLayout> Core::global_descriptor_set_layout;
    std::shared_ptr<DescriptorSetLayout> Core::pbr_material_descriptor_set_layout;
    std::shared_ptr<DescriptorSetLayout> Core::postprocessing_descriptor_set_layout;
    std::shared_ptr<DescriptorSetLayout> Core::shadow_descriptor_set_layout;

    void Core::init(std::shared_ptr<Device> device) {
        global_descriptor_pool = DescriptorPool::Builder(device)
                .set_max_sets(1000)
                .add_pool_size(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2)
                .add_pool_size(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 500)
                .build_shared();

        global_descriptor_set_layout = DescriptorSetLayout::Builder(device)
                .add_binding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
                .build_shared();

        pbr_material_descriptor_set_layout = DescriptorSetLayout::Builder(device)
                .add_binding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
                .add_binding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
                .add_binding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
                .build_shared();

        postprocessing_descriptor_set_layout = DescriptorSetLayout::Builder(device)
                .add_binding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
                .build_shared();

        shadow_descriptor_set_layout = DescriptorSetLayout::Builder(device)
                .add_binding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
                .build_shared();
    }
}