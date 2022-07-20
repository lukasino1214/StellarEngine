#pragma once

#include <chrono>

namespace Engine {

    class Timer {
    public:
        Timer() {
            reset();
        }

        void reset() {
            start = std::chrono::high_resolution_clock::now();
        }

        float elapsed() {
            return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - start).count() * 0.001f * 0.001f * 0.001f;
        }

        float elapsed_milliseconds() {
            return elapsed() * 1000.0f;
        }

    private:
        std::chrono::time_point<std::chrono::high_resolution_clock> start;
    };
}
