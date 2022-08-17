#pragma once

namespace Engine {

    class Timestep {
    public:
        Timestep(float _time = 0.0f) : time(_time) {}

        operator float() const { return time; }

        float get_seconds() const { return time; }
        float get_milliseconds() const { return time / 1000.0f; }

    private:
        float time;
    };

}
