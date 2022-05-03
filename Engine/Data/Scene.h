//
// Created by lukas on 17.09.21.
//

#ifndef ENGINE_SCENE_H
#define ENGINE_SCENE_H

#include "../Core/UUID.h"

#include <entt/entt.hpp>

#include "../Core/Timestamp.h"
#include "../Graphics/FrameInfo.h"

namespace Engine {
    class Entity;

    class Scene {
    public:
        Scene();
        ~Scene();

        Entity CreateEntity(const std::string& name = std::string());
        Entity CreateEntityWithUUID(UUID uuid, const std::string& name = std::string());
        void DestroyEntity(Entity entity);

        void UpdateLightsUbo(GlobalUbo& ubo);
        void OnUpdate(const float& deltaTime);

        //void OnUpdateRuntime(Timestep ts);
        //void OnUpdateEditor(Timestep ts, EditorCamera& camera);
        //void OnViewportResize(uint32_t width, uint32_t height);

        //Entity GetPrimaryCameraEntity();
        //entt::registry m_Registry;
    private:
        entt::registry m_Registry;
        uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;

        friend class Entity;
        friend class SceneSerializer;
        friend class SceneHierarchyPanel;
        friend class RenderSystem;
        friend class PhysicsSystem;
        friend class NativeScript;
        friend class PointLightSystem;
    };

}


#endif //ENGINE_SCENE_H
