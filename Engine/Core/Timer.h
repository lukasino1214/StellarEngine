//
// Created by lukas on 21.09.21.
//

#ifndef ARISE_ENGINE_TIMER_H
#define ARISE_ENGINE_TIMER_H

#include <chrono>

namespace Engine {

    class Timer {
    public:
        Timer() {
            Reset();
        }

        void Timer::Reset() {
            m_Start = std::chrono::high_resolution_clock::now();
        }

        float Timer::Elapsed() {
            return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - m_Start).count() * 0.001f * 0.001f * 0.001f;
        }

        float Timer::ElapsedMillis() {
            return Elapsed() * 1000.0f;
        }

    private:
        std::chrono::time_point<std::chrono::high_resolution_clock> m_Start;
    };

#endif //ARISE_ENGINE_TIMER_H
