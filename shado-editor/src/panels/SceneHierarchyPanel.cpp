#include "SceneHierarchyPanel.h"
#include "scene/Components.h"
#include <glm/gtc/type_ptr.hpp>
#include "imgui_internal.h"
#include "scene/utils/SceneUtils.h"
#include "debug/Profile.h"
#include <fstream>
#include "script/ScriptEngine.h"
#include "ui/UI.h"
#include "ui/imnodes.h"
#include "project/Project.h"
#include "scene/Prefab.h"

namespace Shado {
    SceneHierarchyPanel::SceneHierarchyPanel(const std::string& title, const std::string& propertiesPanelTitle)
        : m_Title(title) {
        m_PropertiesPanel = PropertiesPanel(propertiesPanelTitle);
    }

    SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene>& scene, const std::string& title,
                                             const std::string& propertiesPanelTitle)
        : m_Title(title) {
        setContext(scene);

        m_PropertiesPanel = PropertiesPanel(propertiesPanelTitle);
    }

    void SceneHierarchyPanel::setSelected(Entity entity) {
        m_Selected = entity;
        m_PropertiesPanel.setSelected(entity);
    }

    void SceneHierarchyPanel::setContext(const Ref<Scene>& scene) {
        m_Context = scene;
        setSelected({});
        m_PropertiesPanel.setContext(scene);
    }

    void SceneHierarchyPanel::onImGuiRender() {
        SHADO_PROFILE_FUNCTION();

        ImGui::Begin(m_Title.c_str());

        if (m_Context) {
            m_Context->m_Registry.each([this](auto entityID) {
                Entity entity = {entityID, m_Context.Raw()};

                // Only draw entities without parent
                if (entity && !entity.isChild(*m_Context.Raw()))
                    drawEntityNode(entity);
            });

            if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered()) {
                m_Selected = {};
            }

            // Context menu to create 
            if (ImGui::BeginPopupContextWindow(0, 1, false)) {
                if (ImGui::MenuItem("Create empty Entity"))
                    m_Context->createEntity("Empty Entity");

                ImGui::EndPopup();
            }
        }
        ImGui::End();

        m_PropertiesPanel.onImGuiRender();
    }

    void SceneHierarchyPanel::onEvent(Event& e) {
        if (m_Context->isRunning()) {
            //ScriptEngine::InvokeCustomEditorEvents(e);
        }

        m_PropertiesPanel.onEvent(e);
    }

    void SceneHierarchyPanel::resetSelection() {
        m_Selected = {};
        m_PropertiesPanel.resetSelection();
    }

    static bool isChildSelectedRecursively(Entity entity, Entity selected) {
        if (entity == selected)
            return true;

        for (Entity child : entity.getChildren()) {
            if (isChildSelectedRecursively(child, selected))
                return true;
        }

        return false;
    };

    void SceneHierarchyPanel::drawEntityNode(Entity entity) {
        SHADO_PROFILE_FUNCTION();

        TagComponent& tc = entity.getComponent<TagComponent>();

        auto flags = ImGuiTreeNodeFlags_OpenOnArrow | (m_Selected == entity ? ImGuiTreeNodeFlags_Selected : 0);
        flags |= ImGuiTreeNodeFlags_SpanAvailWidth;

        bool opened;
        {
            ScopedStyleColor textColor(ImGuiCol_Text, {163 / 255.0f, 199 / 255.0f, 245 / 255.0f, 1.0f},
                                       entity.hasComponent<PrefabInstanceComponent>());

            // if a child is selected then expand the parent
            flags |= isChildSelectedRecursively(entity, m_Selected) ? ImGuiTreeNodeFlags_DefaultOpen : 0;
            opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, tc.tag.c_str());
            if (ImGui::IsItemClicked()) {
                setSelected(entity);
            }
        }

        // Context menu item
        bool deleteEntity = false;
        if (ImGui::BeginPopupContextItem()) {
            if (ImGui::MenuItem("Duplicate entity"))
                m_Context->duplicateEntity(m_Selected);

            if (ImGui::MenuItem("Make prefab")) {
                Prefab::CreateFromEntity(m_Selected);
            }

            if (ImGui::MenuItem("Delete entity"))
                deleteEntity = true;

            if (ImGui::MenuItem("Reset parent"))
                entity.getComponent<TransformComponent>().setParent(entity, {});

            ImGui::EndPopup();
        }

        // Drag and drop child
        {
            ImGuiDragDropFlags src_flags = 0;
            src_flags |= ImGuiDragDropFlags_SourceNoDisableHover; // Keep the source displayed as hovered
            if (ImGui::BeginDragDropSource(src_flags)) {
                if (!(src_flags & ImGuiDragDropFlags_SourceNoPreviewTooltip))
                    ImGui::Text("Moving \"%s\"", tc.tag.c_str());
                UUID id = entity.getUUID();
                ImGui::SetDragDropPayload(SceneHeirarchyEntityDragDropId.c_str(), &id, sizeof(UUID));
                ImGui::EndDragDropSource();
            }


            if (ImGui::BeginDragDropTarget()) {
                if (const ImGuiPayload* payload =
                    ImGui::AcceptDragDropPayload(SceneHeirarchyEntityDragDropId.c_str())) {
                    UUID childID = *(const UUID*)payload->Data;
                    Entity childEntity = m_Context->getEntityById(childID);
                    childEntity.getComponent<TransformComponent>().setParent(childEntity, entity);
                }
                ImGui::EndDragDropTarget();
            }
        }

        if (opened) {
            // Display children
            for (Entity child : entity.getChildren()) {
                drawEntityNode(child);
            }
            ImGui::TreePop();
        }

        // Defer the entity deletion to avoid bugs
        if (deleteEntity) {
            m_Context->destroyEntity(entity);
            if (m_Selected == entity)
                m_Selected = {};
        }
    }
}
