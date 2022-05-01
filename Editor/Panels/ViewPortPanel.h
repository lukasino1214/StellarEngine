//
// Created by lukas on 24.04.22.
//

#ifndef ENGINEEDITOR_VIEWPORTPANEL_H
#define ENGINEEDITOR_VIEWPORTPANEL_H

#include "SceneHierarchyPanel.h"
#include "../../Engine/Graphics/Camera.h"

namespace Engine {
    class ViewPortPanel {
    public:
        ViewPortPanel();
    private:
        std::shared_ptr<SceneHierarchyPanel> m_SceneHierarchyPanel;
        std::shared_ptr<Camera> m_Camera;
    };
}


#endif //ENGINEEDITOR_VIEWPORTPANEL_H
