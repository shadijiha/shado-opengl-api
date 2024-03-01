project "Shado-script-core"
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
		"src/**.cs",
	}
