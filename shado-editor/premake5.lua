project "shado-editor"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"

    -- NativeWithManagedCore is a Visual Studio only debugger type.
    filter "action:vs*"
        debuggertype "NativeWithManagedCore"
    filter {}

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

    -- gmake (unlike Visual Studio) does not transitively link a static
    -- library's own dependencies, so the final executable must link the full
    -- closure of static libs explicitly.
    links
    {
        "shado-opengl-api",
        "GLFW",
        "GLEW",
        "ImGui",
        "Box2D",
        "yaml-cpp",
        "msdf-atlas-gen",
        "msdfgen",
        "freetype",
        "Coral.Native",
        "shado-bin-serialization",
    }

    defines
    {
        "FMT_CONSTEVAL="
    }

    -- Ensure the C# assemblies are built before the editor's post-build deploy
    -- (Visual Studio path). On gmake they're excluded and built via dotnet.
    if BUILD_CSHARP == nil or BUILD_CSHARP then
        dependson { "Coral.Managed", "Shado-script-core" }
    end

    filter "system:windows"
        staticruntime "Off"
        systemversion "latest"

        defines
        {
            "SHADO_PLATFORM_WINDOWS"
        }

        -- Stage Shado-script-core.dll (built to shado-editor/ScriptCore) into
        -- shado-editor/DotNet so the common DotNet->bin copy below deploys it
        -- next to the executable. Coral.Managed is staged there by the engine's
        -- own post-build step.
        postbuildcommands
        {
            '{MKDIR} "%{wks.location}/shado-editor/DotNet"',
            '{COPYFILE} "%{wks.location}/shado-editor/ScriptCore/Shado-script-core.dll" "%{wks.location}/shado-editor/DotNet/Shado-script-core.dll"',
        }

    filter "system:macosx"
        defines
        {
            "SHADO_PLATFORM_MACOS",
            "GL_SILENCE_DEPRECATION"
        }

        links
        {
            "OpenGL.framework",
            "Cocoa.framework",
            "IOKit.framework",
            "CoreVideo.framework",
            "CoreFoundation.framework",
            "AppKit.framework",
            "z"
        }

    filter "system:linux"
        defines
        {
            "SHADO_PLATFORM_LINUX"
        }

        links
        {
            "GL",
            "X11",
            "pthread",
            "dl",
            "m",
            "z"
        }

    filter {}

    postbuildcommands
    {
        ("{COPYDIR} %{wks.location}/shado-editor/assets       %{wks.location}/bin/" .. outputdir .. "/%{prj.name}/assets"),
        ("{COPYDIR} %{wks.location}/shado-editor/resources    %{wks.location}/bin/" .. outputdir .. "/%{prj.name}/resources"),
        ("{COPYDIR} %{wks.location}/shado-editor/DotNet       %{wks.location}/bin/" .. outputdir .. "/%{prj.name}/DotNet"),
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
