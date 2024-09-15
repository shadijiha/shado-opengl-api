include "../shado-opengl-api/vendor/Coral/Premake/CSExtensions.lua"

project "Shado-script-core"
	kind "SharedLib"
	language "C#"
	clr "Unsafe"

	targetdir ("../shado-editor/ScriptCore")
	objdir ("../shado-editor/ScriptCore/Intermediates")

	configurations
	{
		"Release"
	}

	files
	{
		"src/**.cs",
	}
