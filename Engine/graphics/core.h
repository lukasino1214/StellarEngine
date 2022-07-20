#pragma once

#include "../pgepch.h"
#include "device.h"
#include "descriptor_set.h"

namespace Engine {
    class Core {

    public:
        static std::shared_ptr<DescriptorPool> global_descriptor_pool;
        static std::shared_ptr<DescriptorSetLayout> global_descriptor_set_layout;
        static std::shared_ptr<DescriptorSetLayout> pbr_material_descriptor_set_layout;
        static std::shared_ptr<DescriptorSetLayout> postprocessing_descriptor_set_layout;
        static std::shared_ptr<DescriptorSetLayout> shadow_descriptor_set_layout;

        static void init(std::shared_ptr<Device> device);

    };
}