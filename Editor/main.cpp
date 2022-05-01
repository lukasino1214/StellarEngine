#include "../Engine/Engine.h"

#include "Panels/SceneHierarchyPanel.h"

using namespace Engine;

#include "app.h"

#include <cstdlib>
#include <iostream>
#include <stdexcept>

int main() {
    FirstApp app{};

    try {
        app.run();
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        return -1;
    }

    return 0;
}