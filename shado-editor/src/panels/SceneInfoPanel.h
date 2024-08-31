#pragma once
#include <vector>

#include "scene/Entity.h"

namespace Shado {
    class Scene;
    
    class SceneInfoPanel {
    public:
        SceneInfoPanel() = default;
        ~SceneInfoPanel() = default;

        void onImGuiRender();
        void setScene(Scene* scene) { m_Scene = scene; }
    private:
        void listEntities(std::vector<Entity> entities, Scene* scene);
    private:
        Scene* m_Scene;
        Entity m_Selected;
    };
}
