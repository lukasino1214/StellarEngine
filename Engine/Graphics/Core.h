//
// Created by lukas on 22.04.22.
//

#ifndef ENGINEEDITOR_CORE_H
#define ENGINEEDITOR_CORE_H

#include "../pgepch.h"
#include "Device.h"
#include "Descriptors.h"

namespace Engine {
    class Core {

    public:
        static std::shared_ptr<DescriptorPool> m_GlobalPool;
        static std::shared_ptr<DescriptorSetLayout> m_GlobalSetLayout;
        static std::shared_ptr<DescriptorSetLayout> m_EntitySetLayout;
        static std::shared_ptr<DescriptorSetLayout> m_PostProcessingLayout;
        static std::shared_ptr<DescriptorSetLayout> m_ShadowLayout;

        static void Init(std::shared_ptr<Device> device);

    };
}


#endif //ENGINEEDITOR_CORE_H
