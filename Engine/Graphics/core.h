//
// Created by lukas on 22.04.22.
//

#ifndef ENGINEEDITOR_CORE_H
#define ENGINEEDITOR_CORE_H

#include <memory>
#include "device.h"

namespace Engine {
    class Core
    {

    public:

        static std::shared_ptr<Device> m_Device;

    };
}


#endif //ENGINEEDITOR_CORE_H
