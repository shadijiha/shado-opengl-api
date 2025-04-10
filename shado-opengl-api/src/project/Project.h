#pragma once

#include <string>
#include <filesystem>

#include "debug/Debug.h"
#include "Events/Event.h"
#include "util/Memory.h"

namespace Shado {
    class EditorAssetManager;
    class RuntimeAssetManager;
    class AssetManagerBase;

    struct ProjectConfig {
        std::string Name = "Untitled";

        std::filesystem::path StartScene;

        std::filesystem::path AssetDirectory;
        std::filesystem::path ScriptModulePath;
        std::filesystem::path AssetRegistryPath = "AssetRegistry.sar"; // Relative to ProjectDirectory
    };

    class Project : public RefCounted {
    public:
        static const std::filesystem::path& GetProjectDirectory() {
            SHADO_CORE_ASSERT(s_ActiveProject, "");
            return s_ActiveProject->m_ProjectDirectory;
        }

        static std::filesystem::path GetAssetDirectory() {
            SHADO_CORE_ASSERT(s_ActiveProject, "");
            return GetProjectDirectory() / s_ActiveProject->m_Config.AssetDirectory;
        }

        // TODO(Yan): move to asset manager when we have one
        static std::filesystem::path GetAssetFileSystemPath(const std::filesystem::path& path) {
            SHADO_CORE_ASSERT(s_ActiveProject, "");
            return GetAssetDirectory() / path;
        }

        static std::filesystem::path GetAssetRegistryPath() {
            SHADO_CORE_ASSERT(s_ActiveProject, "Cannot get asset registry path without an active project");
            return GetProjectDirectory() / s_ActiveProject->m_Config.AssetRegistryPath;
        }

        std::filesystem::path GetRelativePath(const std::filesystem::path& path);
        std::shared_ptr<AssetManagerBase> GetAssetManager() const;
        std::shared_ptr<RuntimeAssetManager> GetRuntimeAssetManager() const;
        std::shared_ptr<EditorAssetManager> GetEditorAssetManager() const;

        ProjectConfig& GetConfig() { return m_Config; }
        const ProjectConfig& GetConfig() const { return m_Config; }
        void ReloadScriptEngine();


        static Ref<Project> GetActive() { return s_ActiveProject; }
        static void SetActive(const Ref<Project>& project);

        static Ref<Project> New();
        static Ref<Project> Load(const std::filesystem::path& path);
        static bool SaveActive(const std::filesystem::path& path);

    private:
        ProjectConfig m_Config;
        std::filesystem::path m_ProjectDirectory;
        std::shared_ptr<AssetManagerBase> m_AssetManager;

        inline static Ref<Project> s_ActiveProject;
    };

    class ProjectChangedEvent : public Event {
    public:
        ProjectChangedEvent(const Ref<Project>& project);

        EVENT_CLASS_TYPE(ProjectChanged);
        EVENT_CLASS_CATEGORY(EventCategoryProject);

        const Ref<Project>& getProject() const { return m_Project; }

        std::string toString() const override {
            return std::format("{} => Name: {}, Path: {}", Event::toString(), m_Project->GetConfig().Name, m_Project->GetProjectDirectory().string());
        }
    private:
        const Ref<Project>& m_Project;
    };

    namespace ProjectUtils {
        const ProjectConfig& GenerateNewProject(const std::filesystem::path& fullpathToSProjFile);
    }
}
