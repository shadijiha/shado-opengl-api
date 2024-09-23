#include "Project.h"

#include <fstream>

#include "Application.h"
#include "ProjectSerializer.h"
#include "asset/AssetManager.h"
#include "script/ScriptEngine.h"
#include "util/FileSystem.h"
#include "scene/Scene.h"

namespace Shado {
    std::filesystem::path Project::GetRelativePath(const std::filesystem::path& path) {
        return std::filesystem::relative(path, m_ProjectDirectory);
    }

    std::shared_ptr<AssetManagerBase> Project::GetAssetManager() const {
        return m_AssetManager;
    }

    std::shared_ptr<RuntimeAssetManager> Project::GetRuntimeAssetManager() const {
        return std::static_pointer_cast<RuntimeAssetManager>(m_AssetManager);
    }

    std::shared_ptr<EditorAssetManager> Project::GetEditorAssetManager() const {
        return std::static_pointer_cast<EditorAssetManager>(m_AssetManager);
    }

    void Project::ReloadScriptEngine() {
        auto& scriptEngine = ScriptEngine::GetMutable();
        scriptEngine.Shutdown();
        scriptEngine.Initialize(this);
        scriptEngine.LoadProjectAssembly();
    }

    void Project::SetActive(const Ref<Project>& project) {
        if (s_ActiveProject)
            ScriptEngine::GetMutable().Shutdown();

        s_ActiveProject = project;
        if (s_ActiveProject)
            ScriptEngine::GetMutable().Initialize(s_ActiveProject);
    }

    Ref<Project> Project::New() {
        SetActive(CreateRef<Project>());
        Application::dispatchEvent(ProjectChangedEvent(s_ActiveProject));
        return s_ActiveProject;
    }

    Ref<Project> Project::Load(const std::filesystem::path& path) {
        Ref<Project> project = CreateRef<Project>();

        ProjectSerializer serializer(project);
        if (serializer.Deserialize(path)) {
            project->m_ProjectDirectory = path.parent_path();
            SetActive(project);

            std::shared_ptr<EditorAssetManager> editorAssetManager = std::make_shared<EditorAssetManager>();
            s_ActiveProject->m_AssetManager = editorAssetManager;
            editorAssetManager->DeserializeAssetRegistry();

            Application::dispatchEvent(ProjectChangedEvent(s_ActiveProject));
            return s_ActiveProject;
        }

        return nullptr;
    }

    bool Project::SaveActive(const std::filesystem::path& path) {
        ProjectSerializer serializer(s_ActiveProject);
        if (serializer.Serialize(path)) {
            s_ActiveProject->m_ProjectDirectory = path.parent_path();

            Application::dispatchEvent(ProjectChangedEvent(s_ActiveProject));
            return true;
        }

        return false;
    }

    ProjectChangedEvent::ProjectChangedEvent(const Ref<Project>& project)
        : m_Project(project) {
    }

    /************************************************************
     * 
     *
     */

    const ProjectConfig& ProjectUtils::GenerateNewProject(const std::filesystem::path& fullpathToSProjFile) {
        const auto& path = fullpathToSProjFile;

        Ref<Project> project = Project::New();
        std::string projectName = std::filesystem::path(path).filename().replace_extension().string();
        std::filesystem::path projectPath = std::filesystem::path(path).parent_path() / projectName;

        // Create project root directory
        FileSystem::NewDirectory(projectPath);

        project->GetConfig().Name = projectName;
        project->GetConfig().StartScene = "main.shadoscene";

        project->GetConfig().AssetDirectory = projectPath / "Assets";
        project->GetConfig().ScriptModulePath = projectPath / "bin" / "Release-windows-x86_64" / projectName / (
            projectName + ".dll");

        // Create folders
        FileSystem::NewDirectory(project->GetConfig().AssetDirectory);
        //FileSystem::NewDirectory(project->GetConfig().ScriptModulePath.parent_path());
        FileSystem::NewDirectory(projectPath / "src");

        Project::SetActive(project);
        Project::SaveActive(projectPath / (projectName + ".sproj"));

        // Copy premake file
        constexpr std::string_view PREMAKE_BIN_DIR = "premake/premake5.exe";
        SHADO_CORE_ASSERT(std::filesystem::exists(PREMAKE_BIN_DIR), "Premake binary not found");
        std::filesystem::copy(PREMAKE_BIN_DIR, projectPath / "premake5.exe");

        // Generate premake bat file
        std::ofstream batFile(projectPath / "GenerateSolution.bat");
        batFile << "call " + (projectPath / "premake5.exe").string() + " vs2022\nPAUSE";
        batFile.close();

        // Generate premake5.lua
        std::string solutionItems_TODO_REMOVE = std::string(SHADO_OPENGL_SOLUTION_DIR_TODO_REMOVE) + "/";
        SHADO_CORE_ASSERT(std::filesystem::exists(solutionItems_TODO_REMOVE), "Solution items not found");
        {
            std::ofstream luaFile(projectPath / "premake5.lua");
            luaFile << std::string("local RootDir = \"" + solutionItems_TODO_REMOVE + "\"\n") +
                "include(RootDir .. \"premake/solution_items.lua\")\nworkspace \"" + projectName + "\""
                R"(
	architecture "x64"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

group "Shado"
	include(RootDir .. "Shado-script-core")
group ""
)" +
                "project \"" + projectName + "\"\n" +
                R"(
	kind "SharedLib"
	language "C#"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	configurations
	{
		"Release"
	}
	
	links
	{
		"Shado-script-core"
	}

	files
	{
		"src/**.cs",
	}

	postbuildcommands
	{
		--("{COPY} \"$(ProjectDir)bin/Release-windows-x86_64/%{prj.name}/%{prj.name}.dll\" \"$(ProjectDir)Assets/%{prj.name}.dll\""),
	}
)";
            luaFile.close();
        }

        // Generate a sample C# class
        {
            std::ofstream sampleClass(projectPath / "src" / "SampleClass.cs");
            sampleClass << R"(
using System;
using System.Collections.Generic;
using Shado;
using Shado.Editor;

namespace )" + projectName + R"( {
	public class SampleClass : Entity {
		void OnCreate() {
		}

		void OnUpdate(float ts) {
		}

		void OnDestroy() {
		}

		void OnEvent(ref Event e) {
		}

		void OnDraw() {
		}

		void OnCollision2DEnter(Collision2DInfo info, Entity other) {
		}

		void OnCollision2DLeave(Collision2DInfo info, Entity other) {
		}
	}

	[EditorTargetType(typeof(SampleClass))]
	public class SampleClassEditor : Editor {
		protected override void OnEditorDraw() {
		}
	}
}
)";
            sampleClass.close();
        }
        return project->GetConfig();
    }
}
