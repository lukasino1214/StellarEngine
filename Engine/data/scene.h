#pragma once

#include "../core/UUID.h"

#include <entt/entt.hpp>

#include "../core/timestamp.h"
#include "../graphics/frame_info.h"

namespace Engine {
    class Entity;

    class Scene {
    public:
        Scene();
        ~Scene();

        Entity create_entity(const std::string &name = std::string());
        Entity create_entity_with_UUID(UUID uuid, const std::string &name = std::string());
        void destroy_entity(Entity entity);

        void update_lights_ubo(GlobalUbo &ubo);
        void update(const float &deltaTime);
        void update_transforms();

    private:
        void update_children(Entity entity);

        entt::registry registry;

        friend class Entity;
        friend class SceneSerializer;
        friend class SceneHierarchyPanel;
        friend class RenderSystem;
        friend class NativeScript;
        friend class PointLightSystem;
        friend class DeferredRenderingSystem;
        friend class ShadowSystem;
        friend class FirstApp;
        friend class App;
    };
}