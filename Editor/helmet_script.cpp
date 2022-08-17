#include "helmet_script.h"

namespace Engine {
    void HelmetScript::start() {

    }

    void HelmetScript::stop() {

    }

    void HelmetScript::update(const float &deltaTime) {
        auto &transform = registry.get<TransformComponent>(handle);
        transform.rotation.y += 2.1f * deltaTime;
        transform.is_dirty = true;
    }

    void HelmetScript::on_event() {

    }
}

