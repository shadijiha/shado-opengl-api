workspace "shado-opengl-api"
	startproject "shado-editor"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

	-- Architecture is platform dependent. Apple Silicon is arm64, while
	-- Windows/Linux desktop builds target x86_64.
	filter "system:windows"
		architecture "x64"
		-- Use a valid modern Windows SDK for EVERY project in the workspace.
		-- Some vendored projects (e.g. Coral.Native) don't set this themselves;
		-- without it, projects that include <Windows.h> fail to compile with
		-- "undeclared identifier" errors in WinBase.h (stale/8.1 SDK fallback).
		systemversion "latest"
	filter "system:linux"
		architecture "x64"
	filter "system:macosx"
		architecture "ARM64"
	filter {}

-- When generating Makefiles (gmake2) we build the native C/C++ code only.
-- The C# managed assemblies (Coral.Managed, Shado-script-core) are portable
-- .NET assemblies that are compiled separately with `dotnet build` and loaded
-- at runtime by the scripting host, so we exclude them from the native
-- workspace to avoid the (flaky) gmake2 C# toolchain. This mirrors Coral's own
-- cross-platform build scripts.
BUILD_CSHARP = not (_ACTION == "gmake2" or _ACTION == "gmake")

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- Include deirectories relative to root folder
IncludeDir = {}
IncludeDir["GLFW"] = "%{wks.location}/shado-opengl-api/vendor/GLFW/include"
IncludeDir["GLEW"] = "%{wks.location}/shado-opengl-api/vendor/GLEW/include"
IncludeDir["imgui"] = "%{wks.location}/shado-opengl-api/vendor/imgui"
IncludeDir["glm"] = "%{wks.location}/shado-opengl-api/vendor/glm"
IncludeDir["spdlog"] = "%{wks.location}/shado-opengl-api/vendor/spdlog/include"
IncludeDir["yaml_cpp"] = "%{wks.location}/shado-opengl-api/vendor/yaml-cpp/include"
IncludeDir["box2d"] = "%{wks.location}/shado-opengl-api/vendor/box2d/include"
IncludeDir["entt"] = "%{wks.location}/shado-opengl-api/vendor/entt/include"
IncludeDir["ImGuizmo"] = "%{wks.location}/shado-opengl-api/vendor/ImGuizmo"
IncludeDir["filewatch"] = "%{wks.location}/shado-opengl-api/vendor/filewatch"
IncludeDir["Coral"] = "%{wks.location}/shado-opengl-api/vendor/Coral/Coral.Native/include"
IncludeDir["CoralSrc"] = "%{wks.location}/shado-opengl-api/vendor/Coral/Coral.Native/Source"
IncludeDir["msdfgen"] = "%{wks.location}/shado-opengl-api/vendor/msdf-atlas-gen/msdfgen"
IncludeDir["msdf_atlas_gen"] = "%{wks.location}/shado-opengl-api/vendor/msdf-atlas-gen/msdf-atlas-gen"
IncludeDir["shado_bin_serialization"] = "%{wks.location}/shado-opengl-api/vendor/shado-bin-serialization/shado-bin-serialization/src"

group "Dependancies"
	if BUILD_CSHARP then
		include "shado-opengl-api/vendor/Coral/Coral.Managed"
	end
	include "shado-opengl-api/vendor/Coral/Coral.Native"
	include "shado-opengl-api/vendor/GLFW"
	include "shado-opengl-api/vendor/GLEW"
	include "shado-opengl-api/vendor/imgui"
	include "shado-opengl-api/vendor/yaml-cpp"
	include "shado-opengl-api/vendor/box2d"
	include "shado-opengl-api/vendor/msdf-atlas-gen"
	include "shado-opengl-api/vendor/shado-bin-serialization/shado-bin-serialization"
group ""

include "shado-opengl-api"
include "sandbox"
include "shado-editor"

if BUILD_CSHARP then
	include "Shado-script-core"
end
