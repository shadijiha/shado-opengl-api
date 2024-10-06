#include "SceneInfoPanel.h"

#include <vector>

#include <imgui.h>
#include <box2d/b2_body.h>

#include "scene/Components.h"
#include "scene/Entity.h"
#include "scene/Prefab.h"
#include "script/ScriptEngine.h"
#include "ui/UI.h"
#include "asset/AssetManager.h" // <--- This is needed DO NOT REMOVE

#define INDENTED(block) { ImGui::Indent(); block; ImGui::Unindent(); }

namespace Shado {
    static void drawEntityNode(Entity entity, Entity& selected);

    static int sectionId = 0;

    static void drawSection(const std::string& name, std::function<void(const std::string&)> drawFn) {
        UI::TreeNode(sectionId++, name, [&name, drawFn]() {
            drawFn(name);
        }, ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_FramePadding);
    }

    void SceneInfoPanel::onImGuiRender() {
        static ImGuiTableFlags tableFlags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
        sectionId = 0;

        ImGui::Begin("Scene Info");

        if (m_Scene == nullptr) {
            UI::Text("No scene selected");
            ImGui::End();
            return;
        }

        drawSection("Scene Data", [this](const std::string& name) {
            UI::Text("Name: %s", m_Scene->name.c_str());
            UI::Text("Viewport: %d x %d", m_Scene->m_ViewportWidth, m_Scene->m_ViewportHeight);
            UI::Text("Is Running: %s", m_Scene->isRunning() ? "true" : "false");
            UI::Text("Physics Enabled: %s", m_Scene->m_PhysicsEnabled ? "true" : "false");

            UI::NewLine();
            drawSection("Entities:", [this](const auto& name) {
                listEntities(m_Scene->getAllEntities(), m_Scene);
            });

            drawSection("Physics (box2d)", [this](const std::string& name) {
                if (!m_Scene->m_World) {
                    UI::Text("No physics world");
                    return;
                }

                b2World* world = m_Scene->m_World;
                UI::Text("Gravity: %.2f, %.2f", world->GetGravity().x, world->GetGravity().y);
                UI::Text("Bodies: %d", world->GetBodyCount());
                UI::Text("Contacts: %d", world->GetContactCount());
                UI::Text("Joints: %d", world->GetJointCount());

                // Make it an iterator
                struct BodyIterator {
                    b2Body* body;

                    BodyIterator(b2Body* body) : body(body) {
                    }

                    b2Body*& operator*() { return body; }

                    BodyIterator& operator++() {
                        body = body->GetNext();
                        return *this;
                    }

                    bool operator!=(const BodyIterator& other) { return body != other.body; }
                };
                UI::Table("World bodies", BodyIterator(world->GetBodyList()), BodyIterator(nullptr), {
                              {
                                  "Entity", [this](b2Body*& body, int i) {
                                      uint64_t entityId = body->GetUserData().pointer;
                                      auto entity = m_Scene->getEntityById(entityId);
                                      UI::Text("%s (%llu)",
                                               entity ? entity.getComponent<TagComponent>().tag.c_str() : "invalid",
                                               entityId);
                                  }
                              },
                              {
                                  "Position", [](b2Body*& body, int i) {
                                      UI::Text("%.2f, %.2f", body->GetPosition().x, body->GetPosition().y);
                                  }
                              },
                              {
                                  "Type", [](b2Body*& body, int i) {
                                      UI::Text(body->GetType() == b2_dynamicBody
                                                   ? "Dynamic"
                                                   : body->GetType() == b2_staticBody
                                                   ? "Static"
                                                   : "Kinematic");
                                  }
                              },
                          }, tableFlags);
            });
        });

        drawSection("Prefabs", [this](const auto& name) {
            UI::Text("Loaded Prefabs: %d", Prefab::loadedPrefabs.size());
            INDENTED(
                for (auto& [uuid, prefab] : Prefab::loadedPrefabs) {
                UI::Text("%llu (root = %s)", uuid, prefab->root.getComponent<TagComponent>().tag.c_str());
                }
            );
        });

        drawSection("Entities to Destroy", [this](const auto& name) {
            UI::Text("Entities to Destroy: %d", m_Scene->toDestroy.size());
            INDENTED(
                for (auto& entity : m_Scene->toDestroy) {
                drawEntityNode(entity, m_Selected);
                }
            );
        });

        drawSection("Script Engine:", [](const auto& name) {
            auto& scriptEngine = ScriptEngine::GetInstance();
            auto scene = scriptEngine.GetCurrentScene();
            if (!scene) {
                UI::Text("No scene selected");
                return;
            }

            auto view = scene->getRegistry().view<ScriptComponent>();
            UI::Table("World bodies", view.begin(), view.end(), {
                          {
                              "Entity", [&scene](const entt::entity& entity, int i) {
                                  Entity e(entity, scene.Raw());
                                  auto& tc = e.getComponent<TagComponent>();
                                  UI::Text("%s", tc.tag.c_str());
                              }
                          },
                          {
                              "Script id", [view](const entt::entity& entity, int i) {
                                  auto& sc = view.get<const ScriptComponent>(entity);
                                  UI::Text("%llu", sc.ScriptID);
                              }
                          },
                          {
                              "Script Name", [view, &scriptEngine](const entt::entity& entity, int i) {
                                  auto& sc = view.get<const ScriptComponent>(entity);
                                  if (!scriptEngine.IsValidScript(sc.ScriptID)) {
                                      UI::Text("(Invalid)");
                                      return;
                                  }
                                  auto name = scriptEngine.GetScriptMetadata(sc.ScriptID).FullName;
                                  UI::Text("%s", name.c_str());
                              }
                          },
                          {
                              "Instance valid?", [view](const entt::entity& entity, int i) {
                                  auto& sc = view.get<const ScriptComponent>(entity);
                                  bool valid = sc.Instance.IsValid();
                                  UI::Checkbox("##valid", valid);
                              }
                          },

                      }, tableFlags);
        });

        ImGui::End();
    }

    static void drawEntityNode(Entity entity, Entity& selected) {
        TagComponent& tc = entity.getComponent<TagComponent>();

        auto flags = ImGuiTreeNodeFlags_OpenOnArrow | (selected == entity ? ImGuiTreeNodeFlags_Selected : 0);
        flags |= ImGuiTreeNodeFlags_SpanAvailWidth;

        bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, "%s (%llu)", tc.tag.c_str(),
                                        entity.getUUID());
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
