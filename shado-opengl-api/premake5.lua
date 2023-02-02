
project "shado-opengl-api"
    location "shado-opengl-api"
    kind "StaticLib"
    language "C++"
    staticruntime "off"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    files
    {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp",
        "%{prj.name}/src/**.hpp",
        "%{prj.name}/vendor/entt/include/**.hpp",

        "%{prj.name}/vendor/ImGuizmo/ImGuizmo.h",
        "%{prj.name}/vendor/ImGuizmo/ImGuizmo.cpp"
    }

    includedirs
    {
        "%{prj.name}/src",
        "%{IncludeDir.GLFW}",
        "%{IncludeDir.GLEW}",
        "%{IncludeDir.imgui}",
        "%{IncludeDir.glm}",
        "%{IncludeDir.spdlog}",
        "%{IncludeDir.entt}",
        "%{IncludeDir.box2d}",
        "%{IncludeDir.yaml_cpp}",
        "%{IncludeDir.ImGuizmo}",
        "%{IncludeDir.mono}"
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
        "mono/mono-2.0-sgen.lib",
        --"mono/mono-2.0-sgen.dll",
        "cs-script"
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

