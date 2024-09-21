#include "ContentBrowserPanel.h"
#include <imgui.h>
#include <filesystem>
#include <future>

#include "debug/Profile.h"
#include "renderer/Texture2D.h"
#include "scene/utils/SceneUtils.h"
#include "scene/SceneSerializer.h"
#include "util/TimeStep.h"
#include "project/Project.h"
#include "SceneHierarchyPanel.h"
#include "scene/Components.h"
#include "scene/Prefab.h"

namespace Shado {
    static bool isImage(const std::filesystem::path& path);

    static Ref<Texture2D> getTextureCached(std::unordered_map<std::string, Ref<Texture2D>>& cache,
                                           std::filesystem::path path) {
        std::string str = path.string();
        if (cache.find(str) != cache.end()) {
            return cache[str];
        }

        Ref<Texture2D> texture = CreateRef<Texture2D>(path.string());
        cache[str] = texture;
        return texture;
    }

    ContentBrowserPanel::ContentBrowserPanel()
        : ContentBrowserPanel(Project::GetActive()
                                  ? std::optional(Project::GetActive()->GetProjectDirectory())
                                  : std::nullopt) {
    }

    ContentBrowserPanel::ContentBrowserPanel(std::optional<std::filesystem::path> path)
        : m_CurrentDirectory(path) {
        m_DirectoryIcon = CreateRef<Texture2D>("resources/icons/DirectoryIcon.png");
        m_FileIcon = CreateRef<Texture2D>("resources/icons/FileIcon.png");
        m_SceneIcon = CreateRef<Texture2D>("resources/icons/scene.png");
        m_PrefabIcon = CreateRef<Texture2D>("resources/icons/Prefab.png");
        m_CSIcon = CreateRef<Texture2D>("resources/icons/CS.png");
        m_SlnIcon = CreateRef<Texture2D>("resources/icons/sln.png");

        if (path.has_value()) {
            setDirectory(path.value());
        }
    }

    void ContentBrowserPanel::onImGuiRender() {
        SHADO_PROFILE_FUNCTION();

        ImGui::Begin("Content Browser");

        // Converting entities to Prefabs
        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(
                SceneHierarchyPanel::SceneHeirarchyEntityDragDropId.c_str())) {
                UUID childID = *(const UUID*)payload->Data;
                auto childEntity = Scene::ActiveScene->getEntityById(childID);

                Ref<Prefab> newPrefab = CreateRef<Prefab>(UUID());
                newPrefab->root = childEntity;

                // Copy script storage to prefab
                Scene::ActiveScene->GetScriptStorage().CopyTo(newPrefab->GetScriptStorage());

                SceneSerializer serializer(Scene::ActiveScene);
                serializer.serializePrefab(newPrefab);

                // Update the entitie's prefab component
                auto& prefabComponent = childEntity.addOrReplaceComponent<PrefabInstanceComponent>();
                prefabComponent.prefabId = newPrefab->GetId();
                prefabComponent.prefabEntityUniqueId = newPrefab->root.getUUID();

                Dialog::alert(
                    std::string("Successfully serialized prefab ") + childEntity.getComponent<TagComponent>().tag,
                    "Info",
                    Dialog::DialogIcon::INFORMATION
                );
            }
            ImGui::EndDragDropTarget();
        }

        // If m_CurrentDirectory is not defined then display "Open a project to browse
        if (!Project::GetActive() || (m_CurrentDirectory.has_value() && m_CurrentDirectory.value().empty())) {
            ImGui::Text("Open a project to browse");
            ImGui::End();
            return;
        }

        // Recheck the filesystem every 200 tick
        if (tick++ % 500 == 0 && m_CurrentDirectory.has_value()) {
            setDirectory(m_CurrentDirectory.value());
            tick = 1;
        }

        {
            const auto& projectPath = Project::GetActive()->GetProjectDirectory();
            if (m_CurrentDirectory != projectPath) {
                if (ImGui::Button("<-")) {
                    m_CurrentDirectory = m_CurrentDirectory.value().parent_path();
                    setDirectory(m_CurrentDirectory.value());
                }
            }

            static float padding = 25.0f;
            static float thumbnailSize = 100.0f;
            float cellSize = thumbnailSize + padding;

            float panelWidth = ImGui::GetContentRegionAvail().x;
            int columnCount = (int)(panelWidth / cellSize);
            if (columnCount < 1)
                columnCount = 1;

            ImGui::Columns(columnCount, 0, false);

            bool changeDir = false;
            for (auto& directoryEntry : directories) {
                const auto& path = directoryEntry.path();
                auto relativePath = std::filesystem::relative(path, projectPath);
                std::string filenameString = relativePath.filename().string();

                ImGui::PushID(filenameString.c_str());

                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));

                uint32_t rendererId = getThumbnail(directoryEntry);
                ImGui::ImageButton((ImTextureID)rendererId, {thumbnailSize, thumbnailSize}, {0, 1}, {1, 0});

                // Context menu to create 
                if (ImGui::BeginPopupContextItem()) {
                    if (ImGui::MenuItem("Open..."))
                        Dialog::openPathInExplorer(path.string());
                    if (ImGui::MenuItem("Open in Explorer"))
                        Dialog::openPathInExplorer(directoryEntry.is_directory()
                                                       ? path.string()
                                                       : path.parent_path().string());

                    ImGui::EndPopup();
                }

                if (ImGui::BeginDragDropSource()) {
                    const wchar_t* itemPath = relativePath.c_str();
                    ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", itemPath,
                                              (wcslen(itemPath) + 1) * sizeof(wchar_t));

                    // Drag and drop preview
                    if (isImage(path)) {
                        Ref<Texture2D> text = getTextureCached(imagesThumbnails, path);
                        ImGui::Image((void*)text->getRendererID(), {thumbnailSize / 2, thumbnailSize / 2}, {0, 1},
                                     {1, 0});
                    }
                    else {
                        ImGui::Text(path.string().c_str());
                    }

                    ImGui::EndDragDropSource();
                }

                ImGui::PopStyleColor();
                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                    if (directoryEntry.is_directory()) {
                        m_CurrentDirectory = m_CurrentDirectory.value() / path.filename();
                        changeDir = true;
                    }
                    else if (Prefab::IsPrefabPath(directoryEntry.path())) {
                        Application::dispatchEvent(PrefabEditorContextChanged(Prefab::CreateFromPath(path)));
                    }
                }

                // If it is a loaded prefab, show the name of the root entity
                if (Prefab::IsPrefabPath(directoryEntry.path())) {
                    // Check if the prefab is loaded
                    UUID prefabID = Prefab::PrefabPathToId(path);
                    if (Prefab::IsLoaded(prefabID)) {
                        auto prefab = Prefab::GetPrefabById(prefabID);
                        auto rootEntity = prefab->root;
                        auto& tag = rootEntity.getComponent<TagComponent>().tag;
                        filenameString = tag;
                    }
                    else {
                        // TODO: load the prefab asynchronously
                    }
                }
                ImGui::TextWrapped(filenameString.c_str());

                ImGui::NextColumn();

                ImGui::PopID();
            }

            ImGui::Columns(1);

            //ImGui::SliderFloat("Thumbnail Size", &thumbnailSize, 16, 512);
            //ImGui::SliderFloat("Padding", &padding, 0, 32);

            // TODO: status bar
            ImGui::End();

            if (changeDir)
                setDirectory(m_CurrentDirectory.value());
        }
    }

    void ContentBrowserPanel::setDirectory(const std::filesystem::path& path) {
        SHADO_PROFILE_FUNCTION();

        directories.clear();
        for (auto& directoryEntry : std::filesystem::directory_iterator(path)) {
            directories.push_back(directoryEntry);
        }

        using P = std::filesystem::directory_entry;
        std::sort(directories.begin(), directories.end(), [](const P& a, const P& b) {
            if (a.is_directory() && !b.is_directory()) return true;
            if (!a.is_directory() && b.is_directory()) return false;
            return a.path().filename() < b.path().filename();
        });
    }

    uint32_t ContentBrowserPanel::getThumbnail(const std::filesystem::directory_entry& directoryEntry) {
        Ref<Texture2D> icon = directoryEntry.is_directory() ? m_DirectoryIcon : m_FileIcon;

        // If it is an image
        const auto path = directoryEntry.path();
        if (isImage(path)) {
            icon = getTextureCached(imagesThumbnails, path);
        }
        else if (path.extension() == ".shadoscene") {
            icon = m_SceneIcon;
        }
        else if (path.extension() == ".prefab") {
            icon = m_PrefabIcon;
        }
        else if (path.extension() == ".cs") {
            icon = m_CSIcon;
        }
        else if (path.extension() == ".sln") {
            icon = m_SlnIcon;
        }
        return icon->getRendererID();
    }

    static bool isImage(const std::filesystem::path& path) {
        return path.extension() == ".jpg" || path.extension() == ".png";
    }
}
