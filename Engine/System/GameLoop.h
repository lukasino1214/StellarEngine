//
// Created by lukas on 01.05.22.
//

#ifndef ENGINE_GAMELOOP_H
#define ENGINE_GAMELOOP_H

#include "../pgepch.h"
#include "../Data/Scene.h"

namespace Engine {
    class GameLoop {
    public:
        GameLoop(std::shared_ptr<Scene> scene);

        void Update();

    private:
        std::shared_ptr<Scene> m_Scene;
    };
}


#endif //ENGINE_GAMELOOP_H
