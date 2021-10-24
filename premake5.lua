workspace "shado-opengl-api"
	architecture "x64"

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

include "shado-opengl-api/vendor/GLFW"
include "shado-opengl-api/vendor/GLEW"

project "shado-opengl-api"
	location "shado-opengl-api"
	kind "StaticLib"
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
		"%{prj.name}",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.GLEW}"
	}

	links
	{
		"GLFW",
		"GLEW",
		"opengl32.lib"
	}

	filter "system:windows"
		cppdialect "C++17"
		staticruntime "On"
		systemversion "latest"

		defines
		{
			"SHADO_PLATFORM_WINDOWS"
		}
	
		postbuildcommands
		{
			("{COPY} %{cfg.buildtarget.relpath} ../bin/" ..outputdir .. "/sandbox")
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
		"shado-opengl-api/src"
	}

	links
	{
		"shado-opengl-api"
	}

	filter "system:windows"
	cppdialect "C++17"
	staticruntime "On"
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
