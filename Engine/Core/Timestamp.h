//
// Created by lukas on 21.09.21.
//

#ifndef ARISE_ENGINE_TIMESTAMP_H
#define ARISE_ENGINE_TIMESTAMP_H

namespace Engine {

    class Timestep {
    public:
        Timestep(float time = 0.0f) : m_Time(time) {}

        operator float() const { return m_Time; }

        float GetSeconds() const { return m_Time; }
        float GetMilliseconds() const { return m_Time * 1000.0f; }
    private:
        float m_Time;
    };

}

#endif //ARISE_ENGINE_TIMESTAMP_H
