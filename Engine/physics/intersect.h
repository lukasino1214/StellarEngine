#pragma once

#include "../data/entity.h"

#include "contact.h"

namespace Engine {
    bool intersect(Entity& entity_A, Entity& entity_B, Contact& contact);
}