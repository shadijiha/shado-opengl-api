#include "SceneInfoPanel.h"

#include <vector>

#include "imgui.h"
#include "box2d/b2_body.h"
#include "scene/Components.h"
#include "scene/Entity.h"
#include "scene/Prefab.h"
#include "script/ScriptEngine.h"

#define INDENTED(block) { ImGui::Indent(); block; ImGui::Unindent(); }

namespace Shado {
    static void drawEntityNode(Entity entity, Entity& selected);

    static void drawSection(const std::string& name, std::function<void(const std::string&)> drawFn) {
        if (ImGui::TreeNodeEx((void*)&name, 0, name.c_str())) {
            drawFn(name);
            ImGui::TreePop();
        }
    }
    
    static bool typeDropDown(const char* id, int& selectedItem) {
        static constexpr int enumLength = 18;
        static std::array<std::string, enumLength> fieldTypeStr;
        static bool initialized = false;
        if (!initialized) {
            for (int i = 0; i < enumLength; i++) {
                fieldTypeStr[i] = Utils::ScriptFieldTypeToString((ScriptFieldType)i);
            }
            initialized = true;
        }
        
        return ImGui::Combo(id, &selectedItem, fieldTypeStr.data()->data(), fieldTypeStr.size());
    } 
    
    void SceneInfoPanel::onImGuiRender()  {
        static ImGuiTableFlags tableFlags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
        
        ImGui::Begin("Scene Info");

        if (m_Scene == nullptr) {
            ImGui::Text("No scene selected");
            ImGui::End();
            return;
        }

        drawSection("Scene Data", [this](const auto& name) {
            ImGui::Text("Name: %s", m_Scene->name.c_str());
            ImGui::Text("Viewport: %d x %d", m_Scene->m_ViewportWidth, m_Scene->m_ViewportHeight);
            ImGui::Text("Is Running: %s", m_Scene->isRunning() ? "true" : "false");
            ImGui::Text("Physics Enabled: %s", m_Scene->m_PhysicsEnabled ? "true" : "false");

            ImGui::NewLine();
            drawSection("Entities:", [this](const auto& name) {
                listEntities(m_Scene->getAllEntities(), m_Scene);
            });

            drawSection("Physics (box2d)", [this](const auto& name) {
                if (!m_Scene->m_World) {
                    ImGui::Text("No physics world");
                    return;
                }

                b2World* world = m_Scene->m_World;
                ImGui::Text("Gravity: %.2f, %.2f", world->GetGravity().x, world->GetGravity().y);
                ImGui::Text("Bodies: %d", world->GetBodyCount());
                ImGui::Text("Contacts: %d", world->GetContactCount());
                ImGui::Text("Joints: %d", world->GetJointCount());

                
                if (ImGui::BeginTable("World bodies", 3, tableFlags))
                {
                    for (b2Body* body = world->GetBodyList(); body != nullptr; body = body->GetNext()) 
                    {
                        ImGui::TableNextRow();

                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text(body->GetType() == b2_dynamicBody ? "Dynamic" : body->GetType() == b2_staticBody ? "Static" : "Kinematic");

                        ImGui::TableSetColumnIndex(1);
                        auto entityId = (uint64_t)body->GetUserData().pointer;
                        auto entity = m_Scene->getEntityById(entityId);
                        ImGui::Text("Entity: %s (%llu)", entity ? entity.getComponent<TagComponent>().tag.c_str() : "invalid", entityId);

                        ImGui::TableSetColumnIndex(2);
                        ImGui::Text("Position: %.2f, %.2f", body->GetPosition().x, body->GetPosition().y);
                    }
                    ImGui::EndTable();
                }

            });
        });

        drawSection("Prefabs", [this](const auto& name) {
            ImGui::Text("Loaded Prefabs: %d", Prefab::loadedPrefabs.size());
            INDENTED(
                for (auto& [uuid, prefab] : Prefab::loadedPrefabs) {
                    ImGui::Text("%llu (root = %s)", uuid, prefab->root.getComponent<TagComponent>().tag.c_str());
                }        
            );
        });

        drawSection("Entities to Destroy", [this](const auto& name) {
            ImGui::Text("Entities to Destroy: %d", m_Scene->toDestroy.size());
            INDENTED(
                for (auto& entity : m_Scene->toDestroy) {
                    drawEntityNode(entity, m_Selected);
                }
            );
        });

        drawSection("Script Engine:", [this](const auto& name) {
            const ScriptEngineData& data = ScriptEngine::GetData();
            if (!&data) {
                ImGui::Text("No script engine data");
                return;
            }

            ImGui::Text("Root:");
            INDENTED({

                //ImGui::Text("Root domain: %s", mono_domain_get_friendly_name(data.RootDomain));
                //ImGui::Text("Core image filename: %s", mono_image_get_filename(data.CoreAssemblyImage));
                ImGui::Text("Core assembly filepath: %s", data.CoreAssemblyFilepath.string().c_str());
            });

            ImGui::Text("App:");
            INDENTED({
                //ImGui::Text("App domain: %s", mono_domain_get_friendly_name(data.AppDomain));            
                //ImGui::Text("App image name: %s", mono_image_get_filename(data.AppAssemblyImage));
                ImGui::Text("Core assembly filepath: %s", data.AppAssemblyFilepath.string().c_str());
            });

            drawSection("Entity Classes:", [this, &data](const auto& name) {
                INDENTED(
                    for (auto& [klassName, klassRef] : data.EntityClasses) {
                        ImGui::Text("%s", klassRef->GetClassFullName().c_str());
                    }
                );
            });

            drawSection("Entity Instances:", [this, &data](const auto& name) {
                INDENTED(
                    ImGui::BeginTable("##Entity Instances", 3, tableFlags);

                    ImGui::TableSetupColumn("Entity");
                    ImGui::TableSetupColumn("Class fullname");
                    ImGui::TableSetupColumn("MonoObject alive?");
                    ImGui::TableHeadersRow();
                    
                    for (auto& [entityId, instance] : data.EntityInstances) {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        
                        auto entity = m_Scene->getEntityById(entityId);
                        if (!entity) {
                            ImGui::Text("Invalid entity (%llu)", entityId);
                            continue;
                        }
                        
                        ImGui::Text("%s (%llu)", entity.getComponent<TagComponent>().tag.c_str(), entityId);

                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("Script: %s", instance->GetScriptClass()->GetClassFullName().c_str());

                        ImGui::TableSetColumnIndex(2);
                        ImGui::Text("%s", instance->GetManagedObject() ? "true" : "false");

                        
                    }
                    ImGui::EndTable();
                );
            });

            // drawSection("Entity Script fields editor", [this, &data](const auto& name) {
            //     INDENTED(
            //         for (auto& [entityId, fieldMap] : data.EntityScriptFields) {
            //             Entity e = m_Scene->getEntityById(entityId);
            //             if (!e) {
            //                 ImGui::Text("Invalid entity (%llu)", entityId);
            //                 continue;
            //             }
            //             
            //             ImGui::Text("Entity: %s (%llu)", e.getComponent<TransformComponent>(), entityId);
            //             INDENTED(
            //                 for (auto& [fieldName, fieldInstance] : fieldMap) {
            //                     ImGui::Text("%s: ", fieldName.c_str());
            //                     ImGui::SameLine();
            //                     
            //                     double value = fieldInstance.GetValue<double>();                                
            //                     ImGui::Text("%s: %f", fieldName.c_str(), value);
            //
            //                     
            //                 }
            //             );
            //         }
            //     );
            // });
        });
                
        ImGui::End();
    }

    static void drawEntityNode(Entity entity, Entity& selected) {
        TagComponent& tc = entity.getComponent<TagComponent>();
		
        auto flags = ImGuiTreeNodeFlags_OpenOnArrow | (selected == entity ? ImGuiTreeNodeFlags_Selected : 0);
        flags |= ImGuiTreeNodeFlags_SpanAvailWidth;

        bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, "%s (%llu)", tc.tag.c_str(), entity.getUUID());
        if (ImGui::IsItemClicked()) {
            selected = entity;
        }

        if (opened) {
            // Display children
            for (Entity child : entity.getChildren()) {
                drawEntityNode(child, selected);
            }
            ImGui::TreePop();
        }
    }
    void SceneInfoPanel::listEntities(std::vector<Entity> entities, Scene* scene) {
        for (auto entity : entities) {
            if (entity && !entity.isChild(*scene)) {
                drawEntityNode(entity, m_Selected);
            }
        }
    }
}
