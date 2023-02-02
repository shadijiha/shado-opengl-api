workspace "shado-opengl-api"
	architecture "x64"
	startproject "shado-editor"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- Include deirectories relative to root folder
IncludeDir = {}
IncludeDir["GLFW"] = "shado-opengl-api/vendor/GLFW/include"
IncludeDir["GLEW"] = "shado-opengl-api/vendor/GLEW/include"
IncludeDir["imgui"] = "shado-opengl-api/vendor/imgui"
IncludeDir["glm"] = "shado-opengl-api/vendor/glm"
IncludeDir["spdlog"] = "shado-opengl-api/vendor/spdlog/include"
IncludeDir["yaml_cpp"] = "shado-opengl-api/vendor/yaml-cpp/include"
IncludeDir["box2d"] = "shado-opengl-api/vendor/box2d/include"
IncludeDir["entt"] = "shado-opengl-api/vendor/entt/include"
IncludeDir["ImGuizmo"] = "shado-opengl-api/vendor/ImGuizmo"
IncludeDir["mono"] = "mono/include/mono-2.0"

group "Dependancies"
	include "shado-opengl-api/vendor/GLFW"
	include "shado-opengl-api/vendor/GLEW"
	include "shado-opengl-api/vendor/imgui"
	include "shado-opengl-api/vendor/yaml-cpp"
	include "shado-opengl-api/vendor/box2d"
group ""

include "shado-opengl-api"
include "sandbox"
include "shado-editor"

project "cs-script"
	location "cs-script"
	kind "SharedLib"
	language "C#"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.cs",
	}

