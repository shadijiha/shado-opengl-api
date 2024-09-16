workspace "shado-opengl-api"
	architecture "x64"
	startproject "shado-editor"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

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
IncludeDir["msdfgen"] = "%{wks.location}/shado-opengl-api/vendor/msdf-atlas-gen/msdfgen"
IncludeDir["msdf_atlas_gen"] = "%{wks.location}/shado-opengl-api/vendor/msdf-atlas-gen/msdf-atlas-gen"

group "Dependancies"
	include "shado-opengl-api/vendor/Coral/Coral.Managed"
	include "shado-opengl-api/vendor/Coral/Coral.Native"
	include "shado-opengl-api/vendor/GLFW"
	include "shado-opengl-api/vendor/GLEW"
	include "shado-opengl-api/vendor/imgui"
	include "shado-opengl-api/vendor/yaml-cpp"
	include "shado-opengl-api/vendor/box2d"
	include "shado-opengl-api/vendor/msdf-atlas-gen"
group ""

include "shado-opengl-api"
include "sandbox"
include "shado-editor"
include "Shado-script-core"