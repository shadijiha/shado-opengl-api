#include "ContentBrowserPanel.h"
#include <imgui.h>
#include <filesystem>
#include <future>

#include "debug/Profile.h"
#include "renderer/Texture2D.h"
#include "scene/utils/SceneUtils.h"
#include "scene/SceneSerializer.h"
#include "project/Project.h"
#include "SceneHierarchyPanel.h"
#include "renderer/FrameBuffer.h"
#include "renderer/Renderer2D.h"
#include "scene/Components.h"
#include "scene/Prefab.h"
#include "util/FileSystem.h"

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

                Prefab::CreateFromEntity(childEntity);

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
            float cellSize = m_ThumbnailSize + padding;

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

                ThumbnailMetadata thumbnailMetadata = getThumbnail(directoryEntry);
                float diff = m_ThumbnailSize - (m_ThumbnailSize / thumbnailMetadata.aspectRatio); // width - height
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + diff); // <-- Make the text aligned
                ImGui::ImageButton((ImTextureID)thumbnailMetadata.rendererId,
                                   {m_ThumbnailSize, m_ThumbnailSize / thumbnailMetadata.aspectRatio},
                                   {0, 1}, {1, 0});

                // Context menu to create 
                if (ImGui::BeginPopupContextItem()) {
                    if (ImGui::MenuItem("Open..."))
                        Dialog::openPathInExplorer(path.string());
                    if (ImGui::MenuItem("Open in Explorer"))
                        Dialog::openPathInExplorer(directoryEntry.is_directory()
                                                       ? path.string()
                                                       : path.parent_path().string());
                    if (ImGui::MenuItem("Delete"))
                        FileSystem::DeleteFile(path);
                    ImGui::EndPopup();
                }

                if (ImGui::BeginDragDropSource()) {
                    const wchar_t* itemPath = relativePath.c_str();
                    ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", itemPath,
                                              (wcslen(itemPath) + 1) * sizeof(wchar_t));

                    // Drag and drop preview
                    if (isImage(path)) {
                        Ref<Texture2D> text = getTextureCached(imagesThumbnails, path);
                        ImGui::Image((void*)text->getRendererID(), {m_ThumbnailSize / 2, m_ThumbnailSize / 2}, {0, 1},
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
                        std::async(std::launch::async, [prefabID]() {
                            Prefab::GetPrefabById(prefabID);
                        });
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

    ContentBrowserPanel::ThumbnailMetadata ContentBrowserPanel::getThumbnail(
        const std::filesystem::directory_entry& directoryEntry) {
        Ref<Texture2D> icon = directoryEntry.is_directory() ? m_DirectoryIcon : m_FileIcon;

        // If it is an image
        const auto path = directoryEntry.path();
        if (isImage(path)) {
            icon = getTextureCached(imagesThumbnails, path);
        }
        else if (path.extension() == ".shadoscene") {
            //icon = m_SceneIcon;
            Ref<Framebuffer> fb;
            if (!m_GeneratedThumbnails.contains(path.string())) {
                fb = generateSceneThumbnail(path, m_ThumbnailSize);
            }
            else {
                fb = m_GeneratedThumbnails[path.string()];
            }

            return {
                fb->getColorAttachmentRendererID(),
                (float)fb->getSpecification().Width / (float)fb->getSpecification().Height
            };
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
        else if (path.extension() == ".glsl" || path.extension() == ".shader") {
            // TODO: currently this will keep trying to generate the thumbnail if try catch fails
            // We do this every frame to keep the shader thumbnail animated
            try {
                Ref<Framebuffer> fb = generateShaderThumbnail(path, m_ThumbnailSize);

                return {
                    fb->getColorAttachmentRendererID(),
                    (float)fb->getSpecification().Width / (float)fb->getSpecification().Height
                };
            }
            catch (const std::runtime_error& e) {
                SHADO_CORE_ERROR("Error generating shader thumbnail: {0}", e.what());
                icon = m_FileIcon;
            }
        }
        return {icon->getRendererID(), (float)icon->getWidth() / (float)icon->getHeight()};
    }

    Ref<Framebuffer> ContentBrowserPanel::generateSceneThumbnail(const std::filesystem::path& path, uint32_t width) {
        FramebufferSpecification specs;
        specs.Attachments = {
            FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::RED_INTEGER,
            FramebufferTextureFormat::DEPTH24STENCIL8
        };
        specs.Width = width;
        specs.Height = width / Application::get().getWindow().getAspectRatio();

        auto fb = Framebuffer::create(specs);

        Ref<Scene> scene = CreateRef<Scene>();
        SceneSerializer serializer(scene);
        serializer.deserialize(path);

        fb->bind();
        Renderer2D::Clear();

        EditorCamera camera;
        camera.SetViewportSize(specs.Width, specs.Height);
        camera.setPosition({0, 0, 10});
        scene->onViewportResize(specs.Width, specs.Height);
        scene->onDrawEditor(camera);
        fb->unbind();

        m_GeneratedThumbnails[path.string()] = fb;

        return fb;
    }

    Ref<Framebuffer> ContentBrowserPanel::generateShaderThumbnail(const std::filesystem::path& path, uint32_t width) {
        // Cache shaders to animate them
        static std::unordered_map<std::string, Ref<Shader>> m_CachedShaders;

        Ref<Framebuffer> fb;
        Ref<Shader> shader;
        if (!m_GeneratedThumbnails.contains(path.string())) {
            FramebufferSpecification specs;
            specs.Attachments = {
                FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::RED_INTEGER,
                FramebufferTextureFormat::DEPTH24STENCIL8
            };
            specs.Width = width;
            specs.Height = width;

            fb = Framebuffer::create(specs);
            shader = CreateRef<Shader>(path);
            m_CachedShaders[path.string()] = shader;

            m_GeneratedThumbnails[path.string()] = fb;
        }
        else {
            fb = m_GeneratedThumbnails[path.string()];
            shader = m_CachedShaders[path.string()];
        }

        fb->bind();
        Renderer2D::Clear();

        TransformComponent transform;
        transform.scale = {10.0f, 10.0f, 10.0f};
        SpriteRendererComponent spriteRenderer;
        spriteRenderer.color = {1.0f, 1.0f, 1.0f, 1.0f};
        spriteRenderer.shader = shader;

        EditorCamera camera;
        camera.SetViewportSize(fb->getSpecification().Width, fb->getSpecification().Height);
        camera.setPosition({0, 0, 10});
        Renderer2D::BeginScene(camera);
        Renderer2D::DrawSprite(transform.getLocalTransform(), spriteRenderer, -1);
        Renderer2D::EndScene();
        fb->unbind();

        return fb;
    }

    static bool isImage(const std::filesystem::path& path) {
        return path.extension() == ".jpg" || path.extension() == ".png";
    }
}
