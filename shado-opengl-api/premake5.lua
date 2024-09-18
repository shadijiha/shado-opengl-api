
project "shado-opengl-api"
    kind "StaticLib"
    language "C++"
    staticruntime "off"
    cppdialect "C++20"
    dependson "Coral.Managed"

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

    files
    {
        "src/**.h",
        "src/**.cpp",
        "src/**.hpp",
        "vendor/entt/include/**.hpp",
        "vendor/ImGuizmo/ImGuizmo.h",
        "vendor/ImGuizmo/ImGuizmo.cpp"
    }

    includedirs
    {
        "src",
        "%{IncludeDir.GLFW}",
        "%{IncludeDir.GLEW}",
        "%{IncludeDir.imgui}",
        "%{IncludeDir.glm}",
        "%{IncludeDir.spdlog}",
        "%{IncludeDir.entt}",
        "%{IncludeDir.box2d}",
        "%{IncludeDir.yaml_cpp}",
        "%{IncludeDir.ImGuizmo}",
        "%{IncludeDir.filewatch}",
        "%{IncludeDir.msdfgen}",
        "%{IncludeDir.msdf_atlas_gen}",
        "%{IncludeDir.Coral}",
    }

    links
    {
        "GLFW",
        "GLEW",
        "ImGui",
        "gdi32.lib",
        "opengl32.lib",
        "box2d",
        "yaml-cpp",
        "shcore.lib",
        "msdf-atlas-gen",
        "Coral.Native",
    }

    filter "system:windows"
        staticruntime "Off"
        systemversion "latest"

        defines
        {
            "SHADO_PLATFORM_WINDOWS", "GLEW_STATIC", "SHADO_ENABLE_ASSERTS"
        }


    defines 
    {
        "SHADO_OPENGL_SOLUTION_DIR_TODO_REMOVE=\"" .. path.getabsolute("%{wks.location}") .. "\""
    }

    filter "configurations:Debug"
        defines {"SHADO_DEBUG", "SHADO_PROFILE"}
        symbols "On"

        postbuildcommands
        {
            '{COPYFILE} "%{wks.location}/shado-opengl-api/vendor/Coral/Coral.Managed/Coral.Managed.runtimeconfig.json" "%{wks.location}/shado-editor/DotNet/Coral.Managed.runtimeconfig.json"',
		    '{COPYFILE} "%{wks.location}/shado-opengl-api/vendor/Coral/Build/Debug/Coral.Managed.dll" "%{wks.location}/shado-editor/DotNet/Coral.Managed.dll"',
		    '{COPYFILE} "%{wks.location}/shado-opengl-api/vendor/Coral/Build/Debug/Coral.Managed.pdb" "%{wks.location}/shado-editor/DotNet/Coral.Managed.pdb"',
		    '{COPYFILE} "%{wks.location}/shado-opengl-api/vendor/Coral/Build/Debug/Coral.Managed.deps.json" "%{wks.location}/shado-editor/DotNet/Coral.Managed.deps.json"',
        }

    filter "configurations:Release"
        defines "SHADO_RELEASE"
        optimize "On"

        postbuildcommands {
            '{COPY} "%{wks.location}/shado-opengl-api/vendor/Coral/Coral.Managed/Coral.Managed.runtimeconfig.json" "%{wks.location}/shado-editor/DotNet/Coral.Managed.runtimeconfig.json"',
		    '{COPY} "%{wks.location}/shado-opengl-api/vendor/Coral/Build/Release/Coral.Managed.dll" "%{wks.location}/shado-editor/DotNet/Coral.Managed.dll"',
		    '{COPY} "%{wks.location}/shado-opengl-api/vendor/Coral/Build/Release/Coral.Managed.pdb" "%{wks.location}/shado-editor/DotNet/Coral.Managed.pdb"',
		    '{COPY} "%{wks.location}/shado-opengl-api/vendor/Coral/Build/Release/Coral.Managed.deps.json" "%{wks.location}/shado-editor/DotNet/Coral.Managed.deps.json"',
        }

    filter "configurations:Dist"
        defines "SHADO_DIST"
        optimize "Full"

        postbuildcommands {
            '{COPY} "%{wks.location}/shado-opengl-api/vendor/Coral/Coral.Managed/Coral.Managed.runtimeconfig.json" "%{wks.location}/shado-editor/DotNet/Coral.Managed.runtimeconfig.json"',
		    '{COPY} "%{wks.location}/shado-opengl-api/vendor/Coral/Build/Dist/Coral.Managed.dll" "%{wks.location}/shado-editor/DotNet/Coral.Managed.dll"',
		    '{COPY} "%{wks.location}/shado-opengl-api/vendor/Coral/Build/Dist/Coral.Managed.pdb" "%{wks.location}/shado-editor/DotNet/Coral.Managed.pdb"',
		    '{COPY} "%{wks.location}/shado-opengl-api/vendor/Coral/Build/Dist/Coral.Managed.deps.json" "%{wks.location}/shado-editor/DotNet/Coral.Managed.deps.json"',
        }

