//
// Created by lukas on 07.11.21.
//

#ifndef ENGINEEDITOR_SCENEHIERARCHYPANEL_H
#define ENGINEEDITOR_SCENEHIERARCHYPANEL_H

#include "../../Engine/Core/Base.h"
#include "../../Engine/Data/Scene.h"
#include "../../Engine/Data/Entity.h"

namespace Engine {

    class SceneHierarchyPanel {
    public:
        SceneHierarchyPanel() = default;

        SceneHierarchyPanel(const std::shared_ptr<Scene> &scene);

        void SetContext(const std::shared_ptr<Scene> &scene);

        std::shared_ptr<Scene> GetContext() { return m_Context; }

        void OnImGuiRender();

        Entity GetSelectedEntity() const { return m_SelectionContext; }

        void SetSelectedEntity(Entity entity);

    private:
        void DrawEntityNode(Entity entity);

        void DrawComponents(Entity entity);

    private:
        std::shared_ptr<Scene> m_Context;
        Entity m_SelectionContext;
    };

}


#endif //ENGINEEDITOR_SCENEHIERARCHYPANEL_H
