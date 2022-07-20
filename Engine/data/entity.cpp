#include "entity.h"

namespace Engine {
    Entity::Entity(entt::entity _handle, Scene *_scene) : handle(_handle), scene(_scene) {}
}