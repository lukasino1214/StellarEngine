#include "../Engine/engine.h"


using namespace Engine;

#include "app.h"

#include <cstdlib>
#include <iostream>
#include <stdexcept>

int main() {
    App app{};

    try {
        app.run();
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        return -1;
    }

    return 0;
}