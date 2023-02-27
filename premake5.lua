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
IncludeDir["GLFW"] = "%{wks.location}/shado-opengl-api/vendor/GLFW/include"
IncludeDir["GLEW"] = "%{wks.location}/shado-opengl-api/vendor/GLEW/include"
IncludeDir["imgui"] = "%{wks.location}/shado-opengl-api/vendor/imgui"
IncludeDir["glm"] = "%{wks.location}/shado-opengl-api/vendor/glm"
IncludeDir["spdlog"] = "%{wks.location}/shado-opengl-api/vendor/spdlog/include"
IncludeDir["yaml_cpp"] = "%{wks.location}/shado-opengl-api/vendor/yaml-cpp/include"
IncludeDir["box2d"] = "%{wks.location}/shado-opengl-api/vendor/box2d/include"
IncludeDir["entt"] = "%{wks.location}/shado-opengl-api/vendor/entt/include"
IncludeDir["ImGuizmo"] = "%{wks.location}/shado-opengl-api/vendor/ImGuizmo"
IncludeDir["filewatch"] = "%{wks.location}/shado-opengl-api/vendor/filewatch"
IncludeDir["mono"] = "%{wks.location}/mono/include/mono-2.0"

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

project "Shado-script-core"
	location "Script projs/Shado-script-core"
	kind "SharedLib"
	language "C#"
	clr "Unsafe"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	configurations
	{
		"Release"
	}

	files
	{
		"Script projs/%{prj.name}/src/**.cs",
	}

	postbuildcommands
	{
		("{COPY} \"%{wks.location}bin/Release-windows-x86_64/%{prj.name}/%{prj.name}.dll\" \"%{wks.location}shado-editor/resources/Scripts/Shado-script-core.dll\""),
	}

project "cs-sandbox"
	location "Script projs/cs-sandbox"
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
		"Script projs/%{prj.name}/src/**.cs",
	}

	postbuildcommands
	{
		("{COPY} \"%{wks.location}bin/Release-windows-x86_64/%{prj.name}/%{prj.name}.dll\" \"%{wks.location}shado-editor/resources/projects/Test 123/Assets/%{prj.name}.dll\""),
	}

