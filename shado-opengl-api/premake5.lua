
project "shado-opengl-api"
    kind "StaticLib"
    language "C++"
    staticruntime "off"

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
        "%{IncludeDir.mono}",
        "%{IncludeDir.filewatch}"
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
        "%{wks.location}/mono/mono-2.0-sgen.lib",
        --"mono/mono-2.0-sgen.dll",
        "Shado-script-core",
        "cs-sandbox"
    }

    filter "system:windows"
        cppdialect "C++17"
        staticruntime "Off"
        systemversion "latest"

        defines
        {
            "SHADO_PLATFORM_WINDOWS", "GLEW_STATIC", "SHADO_ENABLE_ASSERTS"
        }

        postbuildcommands
        {
            ("{COPY} ../mono/mono-2.0-sgen.dll ../bin/" ..outputdir .. "/shado-editor"),
            ("{COPY} ../mono/mono-2.0-sgen.dll ../bin/" ..outputdir .. "/sandbox"),
        }

    filter "configurations:Debug"
        defines {"SHADO_DEBUG", "SHADO_PROFILE"}
        symbols "On"

    filter "configurations:Release"
        defines "SHADO_RELEASE"
        optimize "On"

    filter "configurations:Dist"
        defines "SHADO_DIST"
        optimize "Full"

