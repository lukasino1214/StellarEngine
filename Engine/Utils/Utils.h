//
// Created by lukas on 17.11.21.
//

#ifndef ENGINEEDITOR_UTILS_H
#define ENGINEEDITOR_UTILS_H

#include <functional>

namespace Engine {

// from: https://stackoverflow.com/a/57595105
    template <typename T, typename... Rest>
    void hashCombine(std::size_t& seed, const T& v, const Rest&... rest) {
        seed ^= std::hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        (hashCombine(seed, rest), ...);
    };

}

#endif //ENGINEEDITOR_UTILS_H
