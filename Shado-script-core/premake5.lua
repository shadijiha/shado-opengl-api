include "../shado-opengl-api/vendor/Coral/Premake/CSExtensions.lua"

project "Shado-script-core"
	kind "SharedLib"
	language "C#"
	clr "Unsafe"
	dotnetframework "net8.0"

	targetdir ("../shado-editor/ScriptCore")
	objdir ("../shado-editor/ScriptCore/Intermediates")

	configurations
	{
		"Release"
	}

	propertytags {
		{ "AppendTargetFrameworkToOutputPath", "false" },
		{ "Nullable", "enable" },
	}

	links
	{
		"Coral.Managed"
	}

	files
	{
		"src/**.cs",
	}
