#pragma once

#include <string>
#include <filesystem>
#include "debug/Debug.h"
#include "Events/Event.h"

namespace Shado {

	struct ProjectConfig
	{
		std::string Name = "Untitled";

		std::filesystem::path StartScene;

		std::filesystem::path AssetDirectory;
		std::filesystem::path ScriptModulePath;
	};

	class Project : public RefCounted
	{
	public:
		static const std::filesystem::path& GetProjectDirectory()
		{
			SHADO_CORE_ASSERT(s_ActiveProject, "");
			return s_ActiveProject->m_ProjectDirectory;
		}

		static std::filesystem::path GetAssetDirectory()
		{
			SHADO_CORE_ASSERT(s_ActiveProject, "");
			return GetProjectDirectory() / s_ActiveProject->m_Config.AssetDirectory;
		}

		// TODO(Yan): move to asset manager when we have one
		static std::filesystem::path GetAssetFileSystemPath(const std::filesystem::path& path)
		{
			SHADO_CORE_ASSERT(s_ActiveProject, "");
			return GetAssetDirectory() / path;
		}

		ProjectConfig& GetConfig()				{ return m_Config; }
		const ProjectConfig& GetConfig() const	{ return m_Config; }

		static Ref<Project> GetActive() { return s_ActiveProject; }
		static void SetActive(const Ref<Project>& project) { s_ActiveProject = project; }

		static Ref<Project> New();
		static Ref<Project> Load(const std::filesystem::path& path);
		static bool SaveActive(const std::filesystem::path& path);
	private:
		ProjectConfig m_Config;
		std::filesystem::path m_ProjectDirectory;

		inline static Ref<Project> s_ActiveProject;
	};

	class ProjectChangedEvent : public Event {
	public:
		ProjectChangedEvent(const Ref<Project>& project);

		EVENT_CLASS_TYPE(ProjectChanged);
		EVENT_CLASS_CATEGORY(EventCategoryProject);

		const Ref<Project>& getProject() const { return m_Project; }
	private:
		const Ref<Project>& m_Project;
	};

	namespace ProjectUtils {
		const ProjectConfig& GenerateNewProject(const std::filesystem::path& fullpathToSProjFile);
	}
}
