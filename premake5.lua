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

project "shado-opengl-api"
	location "shado-opengl-api"
	kind "StaticLib"
	language "C++"
	staticruntime "off"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/src/**.hpp",
		"%{prj.name}/vendor/entt/include/**.hpp",

		"%{prj.name}/vendor/ImGuizmo/ImGuizmo.h",
		"%{prj.name}/vendor/ImGuizmo/ImGuizmo.cpp"
	}

	includedirs
	{
		"%{prj.name}/src",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.GLEW}",
		"%{IncludeDir.imgui}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.box2d}",
		"%{IncludeDir.yaml_cpp}",
		"%{IncludeDir.ImGuizmo}",
		"%{IncludeDir.mono}"
	}

	links
	{
		"GLFW",
		"GLEW",
		"ImGui",
		"gdi32.lib",
		"opengl32.lib",
		"box2d",
		"yaml-cpp",
		"shcore.lib",
		"mono/mono-2.0-sgen.lib",
		--"mono/mono-2.0-sgen.dll",
		"cs-script"
	}

	filter "system:windows"
		cppdialect "C++17"
		staticruntime "Off"
		systemversion "latest"

		defines
		{
			"SHADO_PLATFORM_WINDOWS", "GLEW_STATIC", "SHADO_ENABLE_ASSERTS"
		}
	
		postbuildcommands
		{
			("{COPY} ../mono/mono-2.0-sgen.dll ../bin/" ..outputdir .. "/shado-editor"),
			("{COPY} ../mono/mono-2.0-sgen.dll ../bin/" ..outputdir .. "/sandbox"),
		}
	
	filter "configurations:Debug"
		defines {"SHADO_DEBUG", "SHADO_PROFILE"}
		symbols "On"
	
	filter "configurations:Release"
		defines "SHADO_RELEASE"
		optimize "On"

	filter "configurations:Dist"
		defines "SHADO_DIST"
		optimize "Full"

project "sandbox"
	location "sandbox"
	kind "ConsoleApp"
	language "C++"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/src/**.hpp",
		"%{prj.name}/vendor/entt/include/**.hpp"
	}

	includedirs
	{
		"%{IncludeDir.GLFW}", -- For some reason I need this and cherno doesn't
		"%{IncludeDir.GLEW}", -- For some reason I need this and cherno doesn't
		"%{IncludeDir.imgui}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.box2d}",
		"%{IncludeDir.mono}",	-- TODO : remove this
		"shado-opengl-api/src",
		"shado-opengl-api/vendor"
	}

	links
	{
		"shado-opengl-api",
	}

	filter "system:windows"
	cppdialect "C++17"
	staticruntime "Off"
	systemversion "latest"

	defines
	{
		"SHADO_PLATFORM_WINDOWS"
	}

	filter "configurations:Debug"
		defines "SHADO_DEBUG"
		symbols "On"

	filter "configurations:Release"
		defines "SHADO_RELEASE"
		optimize "On"

	filter "configurations:Dist"
		defines "SHADO_DIST"
		optimize "Full"

project "shado-editor"
	location "shado-editor"
	kind "ConsoleApp"
	language "C++"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/src/**.hpp"
	}

	includedirs
	{
		"%{IncludeDir.GLFW}", -- For some reason I need this and cherno doesn't
		"%{IncludeDir.GLEW}", -- For some reason I need this and cherno doesn't
		"%{IncludeDir.imgui}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.box2d}",
		--"%{IncludeDir.mono}",	-- TODO: remove this
		"shado-opengl-api/src",
		"shado-opengl-api/vendor"
	}

	links
	{
		"shado-opengl-api",
		"cs-script"
	}

	filter "system:windows"
		cppdialect "C++17"
		staticruntime "Off"
		systemversion "latest"

	defines
	{
		"SHADO_PLATFORM_WINDOWS"
	}

	filter "configurations:Debug"
		defines "SHADO_DEBUG"
		symbols "On"

	filter "configurations:Release"
		defines "SHADO_RELEASE"
		optimize "On"

	filter "configurations:Dist"
		defines "SHADO_DIST"
		optimize "Full"


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

