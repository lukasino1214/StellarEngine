#include "UUID.h"

#include <random>

namespace Engine {
    static std::random_device random_device;
    static std::mt19937_64 engine(random_device());
    static std::uniform_int_distribution<uint64_t> uniform_distribution;

    UUID::UUID() : uuid(uniform_distribution(engine)) {

    }

    UUID::UUID(uint64_t _uuid) : uuid(_uuid) {

    }
}