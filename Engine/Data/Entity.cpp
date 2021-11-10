//
// Created by lukas on 17.09.21.
//

#include "Entity.h"

namespace Engine {

    Entity::Entity(entt::entity handle, Scene* scene) : m_EntityHandle(handle), m_Scene(scene) {}

}