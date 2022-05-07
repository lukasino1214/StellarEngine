//
// Created by lukas on 22.04.22.
//

#include "Core.h"

namespace Engine {
    std::shared_ptr<DescriptorPool> Core::m_GlobalPool;
    std::shared_ptr<DescriptorSetLayout> Core::m_GlobalSetLayout;
    std::shared_ptr<DescriptorSetLayout> Core::m_EntitySetLayout;
    std::shared_ptr<DescriptorSetLayout> Core::m_PostProcessingLayout;
    std::shared_ptr<DescriptorSetLayout> Core::m_ShadowLayout;
    std::shared_ptr<Device> Core::m_Device;

    void Core::Init() {
        m_GlobalPool = DescriptorPool::Builder()
                .setMaxSets(1000)
                .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2)
                .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 500)
                .SharedBuild();

        m_GlobalSetLayout = DescriptorSetLayout::Builder()
                .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
                .SharedBuild();

        m_EntitySetLayout = DescriptorSetLayout::Builder()
                .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
                .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
                .addBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
                .SharedBuild();

        m_PostProcessingLayout = DescriptorSetLayout::Builder()
                .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
                .SharedBuild();

        m_ShadowLayout = DescriptorSetLayout::Builder()
                .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_ALL_GRAPHICS)
                .SharedBuild();
    }
}