
project "shado-editor"
    kind "ConsoleApp"
    language "C++"

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

    files
    {
        "src/**.h",
        "src/**.cpp",
        "src/**.hpp"
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
        --"%{IncludeDir.mono}",	-- TODO: remove this
        "%{wks.location}/shado-opengl-api/src",
        "%{wks.location}/shado-opengl-api/vendor"
    }

    links
    {
        "shado-opengl-api",
        "Shado-script-core",
        "cs-sandbox",
    }

    filter "system:windows"
        cppdialect "C++17"
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

