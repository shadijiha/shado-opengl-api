include "../shado-opengl-api/vendor/Coral/Premake/CSExtensions.lua"

project "Shado-script-core"
	filter "not system:windows"
		kind "StaticLib"
	filter "system:windows"
		kind "SharedLib"
		language "C#"
		dotnetframework "net8.0"
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
