//
// Created by lukas on 17.09.21.
//

#ifndef ENGINE_ENTITY_H
#define ENGINE_ENTITY_H

#include "../Core/UUID.h"
#include "Components.h"
#include "Scene.h"

#include <entt/entt.hpp>

namespace Engine {

    class Entity {
    public:
        Entity() = default;

        Entity(entt::entity handle, Scene *scene);

        Entity(const Entity &other) = default;

        template<typename T, typename... Args>
        T &AddComponent(Args &&... args) {
            T &component = m_Scene->m_Registry.emplace<T>(m_EntityHandle, std::forward<Args>(args)...);
            //m_Scene->OnComponentAdded<T>(*this, component);
            return component;
        }

        template<typename T>
        T &GetComponent() {
            return m_Scene->m_Registry.get<T>(m_EntityHandle);
        }

        template<typename T>
        bool HasComponent() {
            //return m_Scene->m_Registry.has<T>(m_EntityHandle);
            return m_Scene->m_Registry.all_of<T>(m_EntityHandle); // Please work because I will hang my self
        }

        template<typename T>
        void RemoveComponent() {
            m_Scene->m_Registry.remove<T>(m_EntityHandle);
        }

        operator bool() const { return m_EntityHandle != entt::null; }

        operator entt::entity() const { return m_EntityHandle; }

        operator uint32_t() const { return (uint32_t) m_EntityHandle; }

        UUID GetUUID() { return GetComponent<IDComponent>().ID; }

        entt::entity GetHandle() { return m_EntityHandle; }

        bool operator==(const Entity &other) const {
            return m_EntityHandle == other.m_EntityHandle && m_Scene == other.m_Scene;
        }

        bool operator!=(const Entity &other) const {
            return !(*this == other);
        }

    private:
        entt::entity m_EntityHandle{entt::null};
        Scene *m_Scene = nullptr;
    };

}


#endif //ENGINE_ENTITY_H
