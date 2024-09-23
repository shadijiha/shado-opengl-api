#include "PrefabEditor.h"

#include "SceneHierarchyPanel.h"
#include "scene/Entity.h"
#include "scene/Prefab.h"
#include "asset/AssetManager.h" // <--- This is needed DO NOT REMOVE
#include "scene/SceneSerializer.h"
#include "util/TimeStep.h"

namespace Shado {
    class Scene;


    PrefabEditor::PrefabEditor() {
        m_PrefabEditorHierarchyPanel = snew(SceneHierarchyPanel) SceneHierarchyPanel(
            "Prefab Hierarchy", "Prefab Properties");
    }

    PrefabEditor::~PrefabEditor() {
        //sdelete(m_PrefabEditorHierarchyPanel);
    }

    void PrefabEditor::onImGuiRender() {
        // If a prefab is selected, show the prefab editor
        if (m_PrefabEditorPrefab) {
            ImGui::Begin("Prefab Editor");

            if (ImGui::Button("Save and propagate")) {
                /// Update the prefab with the changes from the editor
                Ref<Scene> prefabEditorScene = m_PrefabEditorHierarchyPanel->getContext();
                entt::registry& prefabEditorRegistry = (entt::registry&)prefabEditorScene->getRegistry();
                Ref<Prefab> newModifiedPrefab = CreateRef<Prefab>(
                    m_PrefabEditorPrefab->GetId(),
                    prefabEditorRegistry,
                    m_PrefabEditorPrefab->root.getComponent<PrefabInstanceComponent>().prefabEntityUniqueId,
                    prefabEditorScene->GetScriptStorage()
                ); // Create prefab with same ID to replace the old one

                Prefab::UpdatedLoadedPrefabs(newModifiedPrefab);

                /// Propagate changes to all instances
                Scene::ActiveScene->propagatePrefabChanges(newModifiedPrefab);
                m_PrefabEditorPrefab = newModifiedPrefab;

                /// Save the prefab to disk
                SceneSerializer serializer(Scene::ActiveScene);
                serializer.serializePrefab(newModifiedPrefab);
            }
            ImGui::SameLine();
            if (ImGui::Button("Close")) {
                m_PrefabEditorPrefab = nullptr;
            }

            m_PrefabEditorHierarchyPanel->onImGuiRender();

            ImGui::End();
        }
    }

    void PrefabEditor::onEvent(Event& e) {
        EventDispatcher dispatcher(e);
        dispatcher.dispatch<PrefabEditorContextChanged>([this](PrefabEditorContextChanged& e) {
            this->setPrefabEditorContext(e.m_Prefab);
            return false;
        });
    }

    void PrefabEditor::setPrefabEditorContext(Ref<Prefab> prefab) {
        m_PrefabEditorPrefab = prefab;

        // Create empty scene with the prefab as only entity
        Ref<Scene> scene = CreateRef<Scene>();

        // Copy the prefab ScriptStorage to the scene
        prefab->GetScriptStorage().CopyTo(scene->GetScriptStorage());

        Entity e = scene->instantiatePrefab(prefab, false);

        m_PrefabEditorHierarchyPanel->setContext(scene);
        m_PrefabEditorHierarchyPanel->setSelected(e);
    }
}
