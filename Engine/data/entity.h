#pragma once

#include "../core/UUID.h"
#include "components.h"
#include "scene.h"

#include <entt/entt.hpp>

namespace Engine {
    class Entity {
    public:
        Entity() = default;
        Entity(entt::entity _handle, Scene *_scene);
        Entity(const Entity &other) = default;

        template<typename T, typename... Args>
        T &add_component(Args &&... args) {
            T &component = scene->registry.emplace<T>(handle, std::forward<Args>(args)...);
            return component;
        }

        template<typename T>
        T &get_component() {
            return scene->registry.get<T>(handle);
        }

        template<typename T>
        bool has_component() {
            return scene->registry.all_of<T>(handle);
        }

        template<typename T>
        void remove_component() {
            scene->registry.remove<T>(handle);
        }

        operator bool() const { return handle != entt::null; }
        operator entt::entity() const { return handle; }
        operator uint32_t() const { return (uint32_t) handle; }

        UUID GetUUID() { return get_component<IDComponent>().ID; }

        entt::entity get_handle() { return handle; }

        bool operator==(const Entity &other) const {
            return handle == other.handle && scene == other.scene;
        }

        bool operator!=(const Entity &other) const {
            return !(*this == other);
        }

    private:
        entt::entity handle{entt::null};
        Scene *scene = nullptr;
    };
}