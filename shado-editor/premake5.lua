
project "shado-editor"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    debuggertype "NativeWithManagedCore"

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
        "src",
        "%{IncludeDir.GLFW}", -- For some reason I need this and cherno doesn't
        "%{IncludeDir.GLEW}", -- For some reason I need this and cherno doesn't
        "%{IncludeDir.imgui}",
        "%{IncludeDir.glm}",
        "%{IncludeDir.spdlog}",
        "%{IncludeDir.entt}",
        "%{IncludeDir.box2d}",
        "%{wks.location}/shado-opengl-api/src",
        "%{wks.location}/shado-opengl-api/vendor",
        "%{IncludeDir.Coral}",
        "%{IncludeDir.CoralSrc}",
        "%{IncludeDir.filewatch}",
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

    postbuildcommands
    {
        ("{COPY} ./assets                       %{wks.location}/bin/" .. outputdir .. "/%{prj.name}/assets"),
        ("{COPY} ./resources                    %{wks.location}/bin/" .. outputdir .. "/%{prj.name}/resources"),
        ("{COPY} ../mono/lib                    %{wks.location}/bin/" .. outputdir .. "/%{prj.name}/mono/lib"),
        ("{COPY} ../mono/mono-2.0-sgen.dll      %{wks.location}/bin/" ..outputdir .. "/shado-editor"),
        ("{COPY} ./imgui.ini                    %{wks.location}/bin/" ..outputdir .. "/shado-editor/imgui.ini"),
        ("{COPY} ../premake                     %{wks.location}/bin/" ..outputdir .. "/shado-editor/premake"),
        ("{COPY} ./DotNet                     %{wks.location}/bin/" ..outputdir .. "/shado-editor/DotNet"),
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

