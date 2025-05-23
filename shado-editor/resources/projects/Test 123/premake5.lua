local RootDir = "../../../../"	-- TODO: <-- Change to ENV variable
include(RootDir .. "premake/solution_items.lua")
include (path.join(RootDir, "shado-opengl-api", "vendor", "Coral", "Premake", "CSExtensions.lua"))

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
	include(RootDir .. "shado-opengl-api/vendor/Coral/Coral.Managed")
 	include(RootDir .. "Shado-script-core")
group ""

project "Test123"
	kind "SharedLib"
	language "C#"
	dotnetframework "net9.0"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	propertytags {
        { "AppendTargetFrameworkToOutputPath", "false" },
        { "Nullable", "enable" },
    }

	configurations
	{
		"Release"
	}
	
	links
	{
		RootDir .. "shado-editor/ScriptCore/Shado-script-core.dll",
		RootDir .. "shado-editor/DotNet/Coral.Managed.dll",
		
	}

	files
	{
		"src/**.cs",
	}

	postbuildcommands
	{
		--("{COPY} \"$(ProjectDir)bin/Release-windows-x86_64/%{prj.name}/%{prj.name}.dll\" \"$(ProjectDir)Assets/%{prj.name}.dll\""),
	}

