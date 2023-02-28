local RootDir = "../../../../"
include(RootDir .. "premake/solution_items.lua")

workspace "Test123"
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

project "Test123"
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

