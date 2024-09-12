#pragma once
#include "Events/Event.h"
#include "util/Memory.h"

namespace Shado {
    class Prefab;
    class SceneHierarchyPanel;
    
    class PrefabEditor {
    public:
        PrefabEditor();
        ~PrefabEditor();
        
        void onImGuiRender();
        void onEvent(Event& e);
    private:
        void setPrefabEditorContext(Ref<Prefab> prefab);
    private:
        // Prefab editor
        Ref<Prefab> m_PrefabEditorPrefab;
        SceneHierarchyPanel* m_PrefabEditorHierarchyPanel;
    };
}
