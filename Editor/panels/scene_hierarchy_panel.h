#pragma once

#include "../../Engine/data/scene.h"
#include "../../Engine/data/entity.h"

namespace Engine {
    class SceneHierarchyPanel {
    public:
        SceneHierarchyPanel() = default;
        SceneHierarchyPanel(const std::shared_ptr<Scene> &scene);

        void render();

        void set_context(const std::shared_ptr<Scene> &scene);
        std::shared_ptr<Scene> get_context() { return context; }

        Entity get_selected_entity() const { return selection_context; }
        void set_selected_entity(Entity entity);

    private:
        void draw_entity_node(Entity entity);
        void draw_components(Entity entity);

    private:
        std::shared_ptr<Scene> context;
        Entity selection_context;
    };

}