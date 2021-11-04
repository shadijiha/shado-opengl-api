workspace "shado-opengl-api"
	architecture "x64"
	startproject "sandbox"

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
IncludeDir["lua"] = "shado-opengl-api/vendor/lua/include"
IncludeDir["box2d"] = "shado-opengl-api/vendor/box2d/include"

LibDirs = {}
LibDirs["lua"] = "vendor/lua/lua54.lib"

include "shado-opengl-api/vendor/GLFW"
include "shado-opengl-api/vendor/GLEW"
include "shado-opengl-api/vendor/imgui"
--include "shado-opengl-api/vendor/luabind"
include "shado-opengl-api/vendor/box2d"

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
		"%{prj.name}/src/**.hpp"
	}

	includedirs
	{
		"%{prj.name}/src",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.GLEW}",
		"%{IncludeDir.imgui}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.lua}",
		"%{IncludeDir.box2d}"
	}

	links
	{
		"GLFW",
		"GLEW",
		"ImGui",
		"gdi32.lib",
		"opengl32.lib",
		"box2d",
		"%{LibDirs.lua}"
	}

	filter "system:windows"
		cppdialect "C++17"
		staticruntime "Off"
		systemversion "latest"

		defines
		{
			"SHADO_PLATFORM_WINDOWS", "GLEW_STATIC"
		}
	
		--postbuildcommands
		--{
		--	("{COPY} %{cfg.buildtarget.relpath} ../bin/" ..outputdir .. "/sandbox"),
		--}
	
	filter "configurations:Debug"
		defines "SHADO_DEBUG"
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
		"%{prj.name}/src/**.hpp"
	}

	includedirs
	{
		"%{IncludeDir.GLFW}", -- For some reason I need this and cherno doesn't
		"%{IncludeDir.GLEW}", -- For some reason I need this and cherno doesn't
		"%{IncludeDir.imgui}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.lua}",
		"%{IncludeDir.box2d}",
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
