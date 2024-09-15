#include "PropertiesPanel.h"

#include <fstream>
#include <glm/gtc/type_ptr.hpp>

#include "imgui.h"
#include "box2d/b2_body.h"
#include "debug/Profile.h"
#include "Project/Project.h"
#include "renderer/Renderer2D.h"
#include "ui/UI.h"
#include "scene/Components.h"
#include "scene/Prefab.h"
#include "scene/utils/SceneUtils.h"
#include "script/ScriptEngine.h"

namespace Shado {
    PropertiesPanel::PropertiesPanel(const std::string& title)
        : m_Title(title) {
    }

    /**
    * PropertiesPanel
    */
    PropertiesPanel::PropertiesPanel(const Ref<Scene>& scene, const std::string& title)
        : m_Title(title) {
        setContext(scene);
    }

    void PropertiesPanel::setContext(const Ref<Scene>& scene) {
        m_Context = scene;
        m_Selected = {};
    }

    void PropertiesPanel::onImGuiRender() {
        ImGui::Begin(m_Title.c_str());
        if (m_Selected) {
            drawComponents(m_Selected);
        }

        ImGui::End();
    }

    void PropertiesPanel::onEvent(Event& e) {
    }

    void PropertiesPanel::setSelected(Entity entity) {
        m_Selected = entity;
    }

    void PropertiesPanel::resetSelection() {
    }

    /**
     * Helper functions
     */

    template <typename T>
    static void drawComponent(const char* label, Entity entity, std::function<void(T&)> ui, bool allowDeletion = true);
    static void drawVec3Control(const std::string& label, glm::vec3& values, float resetValue = 0.0f,
                                float columnWidth = 100.0f);
    static void addComponentContextMenu(Entity m_Selected, uint32_t vpWidth, uint32_t vpHeight);
    static void drawTextureControl(void* spriteData, const std::string& type = "Quad");
    static void generateShaderFile(const std::filesystem::path& path, const std::string& type);

    void PropertiesPanel::drawComponents(Entity entity) {
        SHADO_PROFILE_FUNCTION();

        drawComponent<TagComponent>("Tag", entity, [&entity](TagComponent& tc) {
            // Display ID
            auto id = entity.getComponent<IDComponent>().id;
            UI::Text("ID: %llu", id);

            char buffer[512];
            memset(buffer, 0, sizeof(buffer));
            strcpy_s(buffer, sizeof(buffer), tc.tag.c_str());
            if (ImGui::InputText("##Tag", buffer, sizeof(buffer))) {
                tc.tag = std::string(buffer);
            }
        }, false);

        // Add component context menu
        addComponentContextMenu(m_Selected, m_Context->m_ViewportWidth, m_Context->m_ViewportHeight);

        drawComponent<TransformComponent>("Transform", entity, [](TransformComponent& transform) {
            drawVec3Control("Position", transform.position);

            auto rotation = glm::degrees(transform.rotation);
            drawVec3Control("Rotation", rotation);
            transform.rotation = glm::radians(rotation);

            drawVec3Control("Scale", transform.scale, 1.0);
        }, false);

        drawComponent<PrefabInstanceComponent>("Prefab Instance", entity,
                                               [](PrefabInstanceComponent& prefabInstanceComponent) {
                                                   Ref<Prefab> prefab = Prefab::GetPrefabById(
                                                       prefabInstanceComponent.prefabId);
                                                   std::string prefabIdStr = prefab
                                                                                 ? prefab->root.getComponent<
                                                                                     TagComponent>().tag
                                                                                 : std::to_string(
                                                                                     prefabInstanceComponent.prefabId);

                                                   UI::InputTextControl("Prefab ID:", prefabIdStr,
                                                                        ImGuiInputTextFlags_ReadOnly);
                                                   if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(
                                                       ImGuiMouseButton_Left)) {
                                                       Application::dispatchEvent(
                                                           PrefabEditorContextChanged(
                                                               Prefab::GetPrefabById(
                                                                   prefabInstanceComponent.prefabId)));
                                                   }

                                                   UI::Text("Prefab Element ID: %llu",
                                                            prefabInstanceComponent.prefabEntityUniqueId);
                                               });

        drawComponent<SpriteRendererComponent>("Sprite", entity, [](SpriteRendererComponent& sprite) {
            ImGui::ColorEdit4("Colour", glm::value_ptr(sprite.color));
            drawTextureControl(&sprite);
        });

        drawComponent<CircleRendererComponent>("Circle Renderer", entity, [](CircleRendererComponent& circle) {
            ImGui::ColorEdit4("Colour", glm::value_ptr(circle.color));
            ImGui::DragFloat("Thickness", &circle.thickness, 0.05);
            ImGui::DragFloat("Fade", &circle.fade, 0.01);

            drawTextureControl(&circle, "Circle");
        });

        drawComponent<LineRendererComponent>("Line Renderer", entity, [](LineRendererComponent& line) {
            drawVec3Control("Target", line.target);
            ImGui::ColorEdit4("Colour", glm::value_ptr(line.color));
        });

        drawComponent<CameraComponent>("Camera", entity, [](CameraComponent& camera) {
            // Select menu to change camera type
            const char* projectionTypesString[] = {"Orthographic", "Orbit (Perspective)"};
            const char* currentProjectionType = projectionTypesString[(int)camera.type];
            if (ImGui::BeginCombo("Type", currentProjectionType)) {
                for (int i = 0; i < 2; i++) {
                    bool isSelected = currentProjectionType == projectionTypesString[i];

                    if (ImGui::Selectable(projectionTypesString[i], isSelected)) {
                        currentProjectionType = projectionTypesString[i];

                        // Change camera type
                        camera.setType((CameraComponent::Type)i);
                    }

                    if (isSelected)
                        ImGui::SetItemDefaultFocus();
                }

                ImGui::EndCombo();
            }

            if (camera.type == CameraComponent::Type::Orthographic) {
                float size = camera.getSize();
                if (ImGui::DragFloat("Size", &size)) {
                    camera.setSize(size);
                }
            }

            if (camera.type == CameraComponent::Type::Orbit) {
                OrbitCamera* cam = (OrbitCamera*)camera.camera.Raw();

                float fov = cam->getFOV();
                if (ImGui::DragFloat("FOV", &fov)) {
                    cam->setFOV(fov);
                }
            }

            float nearClip = camera.camera->getNearClip(), farClip = camera.camera->getFarClip();
            if (ImGui::DragFloat("Near clip", &nearClip, 0.1f, camera.type == CameraComponent::Type::Orbit ? 0.1f : 0.1,
                                 1000)) {
                camera.camera->setNearClip(nearClip);
            }

            if (ImGui::DragFloat("Far clip", &farClip)) {
                camera.camera->setFarClip(farClip);
            }

            ImGui::Checkbox("Primary", &camera.primary);
            ImGui::Checkbox("Fixed aspect ratio", &camera.fixedAspectRatio);
        });

        drawComponent<RigidBody2DComponent>("Rigid Body 2D", entity, [](RigidBody2DComponent& rb) {
            // Select menu to change body type
            const char* bodyTypes[] = {"Static", "Kinematic", "Dynamic"};
            const char* currentBodyType = bodyTypes[(int)rb.type];
            if (ImGui::BeginCombo("Type", currentBodyType)) {
                for (int i = 0; i < 3; i++) {
                    bool isSelected = currentBodyType == bodyTypes[i];

                    if (ImGui::Selectable(bodyTypes[i], isSelected)) {
                        currentBodyType = bodyTypes[i];

                        // Change body type
                        rb.type = (RigidBody2DComponent::BodyType)i;

                        // Check if runtime body exists, if so, change it
                        if (rb.runtimeBody)
                            reinterpret_cast<b2Body*>(rb.runtimeBody)->SetType((b2BodyType)i);
                    }

                    if (isSelected)
                        ImGui::SetItemDefaultFocus();
                }

                ImGui::EndCombo();
            }

            ImGui::Checkbox("Fixed rotation", &rb.fixedRotation);
        });

        drawComponent<BoxCollider2DComponent>("Box Collider 2D", entity, [](BoxCollider2DComponent& bc) {
            ImGui::DragFloat2("Size modifier", glm::value_ptr(bc.size), 0.1f);
            ImGui::DragFloat("Density", &bc.density, 0.01f, 0, 1.0f);
            ImGui::DragFloat("Friction", &bc.friction, 0.01f, 0, 1.0f);

            ImGui::Separator();
            ImGui::DragFloat("Restitution", &bc.restitution, 0.01f, 0);
            ImGui::DragFloat("Restitution Threshold", &bc.restitutionThreshold, 0.01f, 0);
        });

        drawComponent<CircleCollider2DComponent>("Circle Collider 2D", entity, [](CircleCollider2DComponent& bc) {
            ImGui::DragFloat("Radius modifier", &bc.radius.x, 0.1f);
            ImGui::DragFloat("Density", &bc.density, 0.01f, 0, 1.0f);
            ImGui::DragFloat("Friction", &bc.friction, 0.01f, 0, 1.0f);

            ImGui::Separator();
            ImGui::DragFloat("Restitution", &bc.restitution, 0.01f, 0);
            ImGui::DragFloat("Restitution Threshold", &bc.restitutionThreshold, 0.01f, 0);
        });

        drawComponent<ScriptComponent>("Script", entity, [entity, scene = m_Context](auto& component) mutable {
            bool scriptClassExists = ScriptEngine::EntityClassExists(component.ClassName);

            ScopedStyleColor textColor(ImGuiCol_Text, ImVec4(0.9f, 0.2f, 0.3f, 1.0f), !scriptClassExists);

            std::string currentItem = component.ClassName;
            std::vector<std::string> hints;
            for (auto& [name, scriptClass] : ScriptEngine::GetEntityClasses())
                hints.push_back(name);
            // Sort the hints
            std::sort(hints.begin(), hints.end());

            if (ImGui::BeginCombo("Class", currentItem.c_str())) {
                for (std::size_t n = 0; n < hints.size(); n++) {
                    bool is_selected = (currentItem == hints[n]);
                    // You can store your selection however you want, outside or inside your objects
                    if (ImGui::Selectable(hints[n].c_str(), is_selected)) {
                        currentItem = hints[n];
                        component.ClassName = currentItem;
                    }
                    if (is_selected)
                        ImGui::SetItemDefaultFocus();
                    // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
                }

                ImGui::EndCombo();
                return;
            }

            // Fields
            bool sceneRunning = scene->isRunning();
            Ref<ScriptInstance> scriptInstance = ScriptEngine::GetEntityScriptInstance(entity.getUUID());

            const std::map<std::string, ScriptField>* fields = nullptr;
            if (sceneRunning && scriptInstance)
                fields = &scriptInstance->GetScriptClass()->GetFields();
            else if (scriptClassExists) {
                Ref<ScriptClass> entityClass = ScriptEngine::GetEntityClass(component.ClassName);
                fields = &entityClass->GetFields();
            }

            if (fields) {
                for (const auto& [name, field] : *fields) {
                    if (sceneRunning && scriptInstance) {
                        ScriptTypeRenderer& renderer = GetRendererForType(field.Type);
                        ScriptTypeRendererDataRunning context = {
                            scene,
                            entity,
                            name,
                            scriptInstance,
                            *fields
                        };
                        renderer.onRenderSceneRunning(context);
                    }
                    else if (scriptClassExists) {
                        Ref<ScriptClass> entityClass = ScriptEngine::GetEntityClass(component.ClassName);
                        auto& entityFields = ScriptEngine::GetScriptFieldMap(entity);

                        ScriptTypeRenderer& renderer = GetRendererForType(field.Type);
                        ScriptTypeRendererDataStopped context = {
                            scene,
                            entity,
                            name,
                            field,
                            entityClass,
                            entityFields,
                            *fields
                        };
                        renderer.onRenderSceneStopped(context);
                    }
                }
            }
        });

        drawComponent<TextComponent>("Text Renderer", entity, [](TextComponent& text) {
            char buffer[512] = {0};
            std::memcpy(buffer, text.text.c_str(), text.text.size());
            buffer[text.text.size()] = '\0';

            if (ImGui::InputTextMultiline("Text", buffer, 512, ImVec2(200, 100))) {
                text.text = std::string(buffer);
            }

            std::string fontPath = text.font ? text.font->getPath().string() : "(empty)";
            UI::InputTextWithChooseFile("Font", fontPath, {".ttf"},
                                        typeid(TextComponent).hash_code(),
                                        [&text](std::string path) {
                                            text.font = CreateRef<Font>(path);
                                        });
            UI::ColorControl("Colour", text.color);
            UI::Vec1Control("Line Spacing", text.lineSpacing, 0.0f);
            UI::Vec1Control("Kerning", text.kerning, 0.0f);
        });
    }

    void ScriptFloatRenderer::onRenderSceneStopped(const ScriptTypeRendererDataStopped& context) {
        auto [scene, entity, fieldName, field, scriptClass, scriptModifiedFields, scriptClassFields] = context;

        if (scriptModifiedFields.find(fieldName) != scriptModifiedFields.end()) {
            ScriptFieldInstance& scriptField = scriptModifiedFields.at(fieldName);
            float data = scriptField.GetValue<float>();
            if (UI::Vec1Control(fieldName, data))
                scriptField.SetValue(data);
        }
        else {
            float data = 0.0f;
            if (UI::Vec1Control(fieldName, data)) {
                ScriptFieldInstance& fieldInstance = scriptModifiedFields[fieldName];
                fieldInstance.Field = field;
                fieldInstance.SetValue(data);
            }
        }
    }

    void ScriptFloatRenderer::onRenderSceneRunning(const ScriptTypeRendererDataRunning& context) {
        auto [scene, entity, fieldName, scriptInstance, scriptClassFields] = context;

        float data = scriptInstance->GetFieldValue<float>(fieldName);
        if (UI::Vec1Control(fieldName, data)) {
            scriptInstance->SetFieldValue(fieldName, data);
        }
    }

    void ScriptIntRenderer::onRenderSceneStopped(const ScriptTypeRendererDataStopped& context) {
        auto [scene, entity, fieldName, field, scriptClass, scriptModifiedFields, scriptClassFields] = context;

        if (scriptModifiedFields.find(fieldName) != scriptModifiedFields.end()) {
            ScriptFieldInstance& scriptField = scriptModifiedFields.at(fieldName);
            int data = scriptField.GetValue<int>();
            if (UI::Vec1Control(fieldName, data))
                scriptField.SetValue(data);
        }
        else {
            int data = 0;
            if (UI::Vec1Control(fieldName, data)) {
                ScriptFieldInstance& fieldInstance = scriptModifiedFields[fieldName];
                fieldInstance.Field = field;
                fieldInstance.SetValue(data);
            }
        }
    }

    void ScriptIntRenderer::onRenderSceneRunning(const ScriptTypeRendererDataRunning& context) {
        auto [scene, entity, fieldName, scriptInstance, scriptClassFields] = context;

        int data = scriptInstance->GetFieldValue<int>(fieldName);
        if (UI::Vec1Control(fieldName, data)) {
            scriptInstance->SetFieldValue(fieldName, data);
        }
    }

    void ScriptBoolRenderer::onRenderSceneStopped(const ScriptTypeRendererDataStopped& context) {
        auto [scene, entity, fieldName, field, scriptClass, scriptModifiedFields, scriptClassFields] = context;

        if (scriptModifiedFields.find(fieldName) != scriptModifiedFields.end()) {
            ScriptFieldInstance& scriptField = scriptModifiedFields.at(fieldName);
            bool data = scriptField.GetValue<bool>();
            if (UI::Checkbox(fieldName, data))
                scriptField.SetValue(data);
        }
        else {
            bool data = false;
            if (UI::Checkbox(fieldName, data)) {
                ScriptFieldInstance& fieldInstance = scriptModifiedFields[fieldName];
                fieldInstance.Field = field;
                fieldInstance.SetValue(data);
            }
        }
    }

    void ScriptBoolRenderer::onRenderSceneRunning(const ScriptTypeRendererDataRunning& context) {
        auto [scene, entity, fieldName, scriptInstance, scriptClassFields] = context;

        bool data = scriptInstance->GetFieldValue<bool>(fieldName);
        if (UI::Checkbox(fieldName, data)) {
            scriptInstance->SetFieldValue(fieldName, data);
        }
    }

    void ScriptVector3Renderer::onRenderSceneStopped(const ScriptTypeRendererDataStopped& context) {
        auto [scene,
            entity,
            fieldName,
            field,
            scriptClass,
            scriptModifiedFields,
            scriptClassFields] = context;

        // Field has been set in editor
        if (scriptModifiedFields.find(fieldName) != scriptModifiedFields.end()) {
            ScriptFieldInstance& scriptField = scriptModifiedFields.at(fieldName);
            glm::vec3 data = scriptField.GetValue<glm::vec3>();
            if (UI::Vec3Control(fieldName, data))
                scriptField.SetValue(data);
        }
        else {
            glm::vec3 data = {0, 0, 0};
            if (UI::Vec3Control(fieldName, data)) {
                ScriptFieldInstance& fieldInstance = scriptModifiedFields[fieldName];
                fieldInstance.Field = field;
                fieldInstance.SetValue(data);
            }
        }
    }

    void ScriptVector3Renderer::onRenderSceneRunning(const ScriptTypeRendererDataRunning& context) {
        auto [scene, entity, fieldName, scriptInstance, scriptClassFields] = context;
        glm::vec3 data = scriptInstance->GetFieldValue<glm::vec3>(fieldName);
        if (UI::Vec3Control(fieldName, data)) {
            scriptInstance->SetFieldValue(fieldName, data);
        }
    }

    void ScriptColourRenderer::onRenderSceneStopped(const ScriptTypeRendererDataStopped& context) {
        auto [scene,
            entity,
            fieldName,
            field,
            scriptClass,
            scriptModifiedFields,
            scriptClassFields] = context;

        // Field has been set in editor
        if (scriptModifiedFields.find(fieldName) != scriptModifiedFields.end()) {
            ScriptFieldInstance& scriptField = scriptModifiedFields.at(fieldName);

            glm::vec4 data = scriptField.GetValue<glm::vec4>();
            if (UI::ColorControl(fieldName, data))
                scriptField.SetValue(data);
        }
        else {
            glm::vec4 data = {0, 0, 0, 1.0f};
            if (UI::ColorControl(fieldName, data)) {
                ScriptFieldInstance& fieldInstance = scriptModifiedFields[fieldName];
                fieldInstance.Field = field;
                fieldInstance.SetValue(data);
            }
        }
    }

    void ScriptColourRenderer::onRenderSceneRunning(const ScriptTypeRendererDataRunning& context) {
        auto [scene, entity, fieldName, scriptInstance, scriptClassFields] = context;
        glm::vec4 data = scriptInstance->GetFieldValue<glm::vec4>(fieldName);
        if (UI::ColorControl(fieldName, data)) {
            scriptInstance->SetFieldValue(fieldName, data);
        }
    }

    void ScriptPrefabRenderer::onRenderSceneStopped(const ScriptTypeRendererDataStopped& context) {
        auto [scene,
            entity,
            fieldName,
            field,
            scriptClass,
            scriptModifiedFields,
            scriptClassFields] = context;

        if (scriptModifiedFields.find(fieldName) != scriptModifiedFields.end()) {
            ScriptFieldInstance& scriptField = scriptModifiedFields.at(fieldName);

            PrefabCSMirror data = scriptField.GetValue<PrefabCSMirror>();
            if (data.id != 0) {
                Ref<Prefab> prefab = Prefab::GetPrefabById(data.id);
                ScopedStyleColor textColor(ImGuiCol_Text, ImVec4(0.9f, 0.2f, 0.3f, 1.0f), prefab == nullptr);
                std::string prefabTextValue = prefab
                                                  ? prefab->root.getComponent<TagComponent>().tag
                                                  : std::to_string(data.id);
                prefabTextValue += " (prefab)";

                UI::InputTextControl(fieldName, prefabTextValue, ImGuiInputTextFlags_ReadOnly);
            }
            else {
                std::string prefabIdStr = "(empty)";
                UI::InputTextControl(fieldName, prefabIdStr, ImGuiInputTextFlags_ReadOnly);
            }

            // TODO: refactor duplicate code
            if (ImGui::BeginDragDropTarget()) {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM")) {
                    const wchar_t* pathStr = (const wchar_t*)payload->Data;
                    auto path = Project::GetActive()->GetProjectDirectory() / pathStr;

                    if (Prefab::IsPrefabPath(path)) {
                        Ref<Prefab> prefab = Prefab::CreateFromPath(path);
                        ScriptFieldInstance& fieldInstance = scriptField;
                        fieldInstance.Field = field;
                        fieldInstance.SetValue(PrefabCSMirror{prefab->GetId()});
                    }
                }

                ImGui::EndDragDropTarget();
            }
        }
        else {
            // TODO: Make it so you can drag and drop prefab from content plane or scene hiarchy
            UUID prefabId = 0;
            std::string prefabIdStr = prefabId == 0 ? "(empty)" : std::to_string(prefabId);

            UI::InputTextControl(fieldName, prefabIdStr, ImGuiInputTextFlags_ReadOnly);

            // TODO: refactor duplicate code
            if (ImGui::BeginDragDropTarget()) {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM")) {
                    const wchar_t* pathStr = (const wchar_t*)payload->Data;
                    auto path = Project::GetActive()->GetProjectDirectory() / pathStr;

                    if (Prefab::IsPrefabPath(path)) {
                        Ref<Prefab> prefab = Prefab::CreateFromPath(path);
                        ScriptFieldInstance& fieldInstance = scriptModifiedFields[fieldName];
                        fieldInstance.Field = field;
                        fieldInstance.SetValue(PrefabCSMirror{prefab->GetId()});
                    }
                }

                ImGui::EndDragDropTarget();
            }
        }
    }

    void ScriptPrefabRenderer::onRenderSceneRunning(const ScriptTypeRendererDataRunning& context) {
        auto [scene, entity, fieldName, scriptInstance, scriptClassFields] = context;
        PrefabCSMirror data = scriptInstance->GetFieldValue<PrefabCSMirror>(fieldName);
        if (data.id != 0) {
            std::string prefabIdStr = std::to_string(data.id);
            UI::InputTextControl(fieldName, prefabIdStr, ImGuiInputTextFlags_ReadOnly);
        }
        else {
            std::string prefabIdStr = "(empty)";
            UI::InputTextControl(fieldName, prefabIdStr, ImGuiInputTextFlags_ReadOnly);
        }
    }

    void ScriptCustomEditorRenderer::onRenderSceneStopped(const ScriptTypeRendererDataStopped& context) {
        auto [scene,
            entity,
            fieldName,
            field,
            scriptClass,
            scriptModifiedFields,
            scriptClassFields] = context;

        if (scriptModifiedFields.find(fieldName) != scriptModifiedFields.end()) {
            // TODO: How should we deal with complex types when seralizing?
            ScriptEngine::DrawCustomEditorForFieldStopped(fieldName, field, entity, scriptClass, true);
        }
        else {
            ScriptEngine::DrawCustomEditorForFieldStopped(fieldName, field, entity, scriptClass, false);
        }
    }

    ScriptTypeRenderer& GetRendererForType(ScriptFieldType type) {
        static std::unordered_map<ScriptFieldType, ScriptTypeRenderer*> s_Renderers = {
            {ScriptFieldType::Float, snew(ScriptFloatRenderer) ScriptFloatRenderer()},
            {ScriptFieldType::Int, snew(ScriptIntRenderer) ScriptIntRenderer()},
            {ScriptFieldType::Bool, snew(ScriptBoolRenderer) ScriptBoolRenderer()},
            {ScriptFieldType::Vector3, snew(ScriptVector3Renderer) ScriptVector3Renderer()},
            {ScriptFieldType::Colour, snew(ScriptColourRenderer) ScriptColourRenderer()},
            {ScriptFieldType::Prefab, snew(ScriptPrefabRenderer) ScriptPrefabRenderer()},
        };
        static auto* customEditor = snew(ScriptCustomEditorRenderer) ScriptCustomEditorRenderer();

        if (s_Renderers.find(type) != s_Renderers.end())
            return *s_Renderers[type];
        else
            return *customEditor;
    }

    void ScriptCustomEditorRenderer::onRenderSceneRunning(const ScriptTypeRendererDataRunning& context) {
        auto& field = context.scriptClassFields.at(context.fieldName);
        ScriptEngine::DrawCustomEditorForFieldRunning(field, context.scriptInstance, context.fieldName);
    }

    template <typename T>
    static void drawComponent(const char* label, Entity entity, std::function<void(T&)> ui, bool allowDeletion) {
        SHADO_PROFILE_FUNCTION();

        const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap
            | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_FramePadding
            | ImGuiTreeNodeFlags_AllowItemOverlap;

        if (entity.hasComponent<T>()) {
            auto& component = entity.getComponent<T>();
            ImVec2 contentRegionAvail = ImGui::GetContentRegionAvail();

            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {4, 4});
            float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
            ImGui::Separator();

            bool open = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), treeNodeFlags, label);

            ImGui::PopStyleVar();

            // Component settings menu
            if (allowDeletion) {
                UI::SameLine(contentRegionAvail.x - lineHeight * 0.5f);
                if (UI::ButtonControl("...", {lineHeight, lineHeight})) {
                    ImGui::OpenPopup("ComponentSettings");
                }
            }


            bool removeComponent = false;
            if (allowDeletion && ImGui::BeginPopup("ComponentSettings")) {
                if (ImGui::MenuItem("Remove component"))
                    removeComponent = true;

                ImGui::EndPopup();
            }

            if (open) {
                ui(component);
                ImGui::TreePop();
            }

            if (removeComponent) {
                entity.removeComponent<T>();
            }
        }
    }

    // Draw a vector with colourful x, y and z
    static void drawVec3Control(const std::string& label, glm::vec3& values, float resetValue, float columnWidth) {
        ImGuiIO& io = ImGui::GetIO();
        auto boldFont = io.Fonts->Fonts[ImguiLayer::FontIndex::BOLD];

        ImGui::PushID(label.c_str());

        ImGui::Columns(2);
        ImGui::SetColumnWidth(0, columnWidth);
        ImGui::Text(label.c_str());
        ImGui::NextColumn();

        ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {0, 0});

        float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
        ImVec2 buttonSize = {lineHeight + 3.0f, lineHeight};

        ImGui::PushStyleColor(ImGuiCol_Button, {0.8f, 0.1f, 0.15f, 1.0f});
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, {0.9f, 0.2f, 0.2f, 1.0f});
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, {0.8f, 0.1f, 0.15f, 1.0f});
        ImGui::PushFont(boldFont);
        if (ImGui::Button("X", buttonSize))
            values.x = resetValue;
        ImGui::PopFont();
        ImGui::PopStyleColor(3);


        ImGui::SameLine();
        ImGui::DragFloat("##X", &values.x, 0.1f, 0, 0, "%.2f");
        ImGui::PopItemWidth();
        ImGui::SameLine();


        ImGui::PushStyleColor(ImGuiCol_Button, {0.2f, 0.7f, 0.2f, 1.0f});
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, {0.3f, 0.8f, 0.3f, 1.0f});
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, {0.2f, 0.7f, 0.2f, 1.0f});
        ImGui::PushFont(boldFont);
        if (ImGui::Button("Y", buttonSize))
            values.y = resetValue;
        ImGui::PopFont();
        ImGui::PopStyleColor(3);


        ImGui::SameLine();
        ImGui::DragFloat("##Y", &values.y, 0.1f, 0, 0, "%.2f");
        ImGui::PopItemWidth();
        ImGui::SameLine();


        ImGui::PushStyleColor(ImGuiCol_Button, {0.1f, 0.25f, 0.8f, 1.0f});
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, {0.2f, 0.35f, 0.9f, 1.0f});
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, {0.1f, 0.25f, 0.8f, 1.0f});
        ImGui::PushFont(boldFont);
        if (ImGui::Button("Z", buttonSize))
            values.z = resetValue;
        ImGui::PopFont();
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        ImGui::DragFloat("##Z", &values.z, 0.1f, 0, 0, "%.2f");
        ImGui::PopItemWidth();

        ImGui::PopStyleVar();

        ImGui::Columns(1);

        ImGui::PopID();
    }

    static void addComponentContextMenu(Entity m_Selected, uint32_t vpWidth, uint32_t vpHeight) {
        ImGui::SameLine();
        ImGui::PushItemWidth(-1);
        // To Add components context menu
        if (ImGui::Button("Add Component"))
            ImGui::OpenPopup("AddComponent");

        if (ImGui::BeginPopup("AddComponent")) {
            if (!m_Selected.hasComponent<SpriteRendererComponent>() && ImGui::MenuItem("Sprite renderer")) {
                m_Selected.addComponent<SpriteRendererComponent>();
                ImGui::CloseCurrentPopup();
            }

            if (!m_Selected.hasComponent<CircleRendererComponent>() && ImGui::MenuItem("Circle renderer")) {
                m_Selected.addComponent<CircleRendererComponent>();
                ImGui::CloseCurrentPopup();
            }

            if (!m_Selected.hasComponent<LineRendererComponent>() && ImGui::MenuItem("Line renderer")) {
                m_Selected.addComponent<LineRendererComponent>();
                ImGui::CloseCurrentPopup();
            }

            if (!m_Selected.hasComponent<RigidBody2DComponent>() && ImGui::MenuItem("Rigid Body 2D")) {
                m_Selected.addComponent<RigidBody2DComponent>();
                ImGui::CloseCurrentPopup();
            }

            if (!m_Selected.hasComponent<BoxCollider2DComponent>() && ImGui::MenuItem("Box Collider 2D")) {
                m_Selected.addComponent<BoxCollider2DComponent>();
                ImGui::CloseCurrentPopup();
            }

            if (!m_Selected.hasComponent<CircleCollider2DComponent>() && ImGui::MenuItem("Circle Collider 2D")) {
                m_Selected.addComponent<CircleCollider2DComponent>();
                ImGui::CloseCurrentPopup();
            }

            if (!m_Selected.hasComponent<CameraComponent>() && ImGui::MenuItem("Camera")) {
                m_Selected.addComponent<CameraComponent>(CameraComponent::Type::Orthographic, vpWidth, vpHeight);
                ImGui::CloseCurrentPopup();
            }

            if (!m_Selected.hasComponent<NativeScriptComponent>() && ImGui::MenuItem("Native script")) {
                m_Selected.addComponent<NativeScriptComponent>();
                ImGui::CloseCurrentPopup();
            }

            if (!m_Selected.hasComponent<ScriptComponent>() && ImGui::MenuItem("Script component")) {
                m_Selected.addComponent<ScriptComponent>();
                ImGui::CloseCurrentPopup();
            }

            if (!m_Selected.hasComponent<TextComponent>() && ImGui::MenuItem("Text renderer")) {
                m_Selected.addComponent<TextComponent>();
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }
        ImGui::PopItemWidth();
    }

    // TODOOOOO: THis should be deleted and replaced with UI.InputText...
    static void drawTextureControl(void* spriteData, const std::string& type) {
        SpriteRendererComponent& sprite = *(SpriteRendererComponent*)spriteData;

        // =========== Texture
        std::string texturePath = sprite.texture ? sprite.texture->getFilePath().c_str() : "No Texture";

        UI::InputTextWithChooseFile("Texture", texturePath, {".jpg", ".png"}, typeid(sprite.texture).hash_code(),
                                    [&](std::string path) {
                                        Ref<Texture2D> texture = CreateRef<Texture2D>(path);
                                        if (texture->isLoaded())
                                            sprite.texture = texture;
                                        else
                                            SHADO_CORE_WARN("Could not load texture {0}", path);
                                        SHADO_CORE_INFO("Loaded texture {0}", path.c_str());
                                    }
        );

        // Image
        if (sprite.texture) {
            ImGui::Image((void*)sprite.texture->getRendererID(), {60, 60}, ImVec2(0, 1), ImVec2(1, 0));

            UI::SameLine();
            if (UI::ButtonControl("X")) {
                sprite.texture = nullptr;
            }
        }

        // =========== Tilling factor
        ImGui::DragFloat("Tilling factor", &sprite.tilingFactor, 0.01);

        ImGui::Separator();

        // =========== Shader
        std::string shaderPath = sprite.shader
                                     ? Project::GetActive()->GetRelativePath(sprite.shader->getFilepath()).string().
                                                             c_str()
                                     : "Default Shader";
        UI::InputTextWithChooseFile("Shader", shaderPath, {".glsl", ".shader"}, typeid(sprite.shader).hash_code(),
                                    [&](std::string path) {
                                        sprite.shader = CreateRef<Shader>(path);
                                    }
        );

        UI::SameLine();
        if (UI::ButtonControl("x")) {
            sprite.shader = nullptr;
        }

        if (UI::ButtonControl("+")) {
            std::filesystem::path path = FileDialogs::saveFile("Shader file (*glsl)\0*.glsl\0");
            if (!path.empty()) {
                generateShaderFile(path, type);
                Dialog::openPathInExplorer(path);

                // Create shader
                try {
                    sprite.shader = CreateRef<Shader>(path);
                }
                catch (const ShaderCompilationException& e) {
                    SHADO_CORE_ERROR("Failed to compile shader: {0}", e.what());
                }
            }
        }

        UI::SameLine();
        if (UI::ButtonControl("Recompile")) {
            if (sprite.shader) {
                try {
                    sprite.shader = CreateRef<Shader>(sprite.shader->getFilepath());
                }
                catch (const ShaderCompilationException& e) {
                    SHADO_CORE_ERROR("Failed to recompile shader: {0}", e.what());
                }
            }
        }

        // Draw shader uniforms
        if (sprite.shader) {
            auto& shader = sprite.shader;
            auto uniforms = sprite.shader->getActiveUniforms();

            UI::NewLine();
            UI::Text("Shader active uniforms");
            for (auto& [name, dataType] : uniforms) {
                switch (dataType) {
                case ShaderDataType::Float: {
                    float value = shader->getFloat(name);
                    if (UI::Vec1Control(name, value)) {
                        shader->setFloat(name, value);
                        shader->saveCustomUniformValue(name, dataType, value);
                    }
                    break;
                }
                case ShaderDataType::Float2: {
                    glm::vec3 value = glm::vec3(shader->getFloat2(name), 0.0f);
                    if (UI::Vec3Control(name, value)) {
                        shader->setFloat2(name, value);
                        shader->saveCustomUniformValue(name, dataType, value);
                    }
                    break;
                }
                case ShaderDataType::Float3: {
                    glm::vec3 value = shader->getFloat3(name);
                    if (UI::Vec3Control(name, value)) {
                        shader->setFloat3(name, value);
                        shader->saveCustomUniformValue(name, dataType, value);
                    }
                    break;
                }
                case ShaderDataType::Float4: {
                    glm::vec4 value = shader->getFloat4(name);
                    if (UI::ColorControl(name, value)) {
                        shader->setFloat4(name, value);
                        shader->saveCustomUniformValue(name, dataType, value);
                    }
                    break;
                }
                case ShaderDataType::Int: {
                    int value = shader->getInt(name);
                    if (UI::Vec1Control(name, value)) {
                        shader->setInt(name, value);
                        shader->saveCustomUniformValue(name, dataType, value);
                    }
                    break;
                }
                }
            }
        }
    }

    static void generateShaderFile(const std::filesystem::path& path, const std::string& type) {
        auto filepath = type == "Circle" ? CIRCLE_SHADER : QUAD_SHADER;

        std::ifstream file(filepath);
        std::string shader;
        if (file) {
            shader = std::string((std::istreambuf_iterator<char>(file)),
                                 std::istreambuf_iterator<char>());
            file.close();
        }
        else {
            SHADO_CORE_ERROR("Failed to generate shader file!");
            return;
        }

        std::ofstream output(path);
        output << shader << std::endl;
        output.close();
    }
}
