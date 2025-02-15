project "sandbox"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"src/**.h",
		"src/**.cpp",
		"src/**.hpp",
		"vendor/entt/include/**.hpp"
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
        "%{IncludeDir.Coral}",
        "%{IncludeDir.CoralSrc}",
		"%{wks.location}/shado-opengl-api/src",
		"%{wks.location}/shado-opengl-api/vendor",
		"%{wks.location}/shado-editor/src",
	}

	links
	{
		"shado-opengl-api",
	}

	filter "system:windows"
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
