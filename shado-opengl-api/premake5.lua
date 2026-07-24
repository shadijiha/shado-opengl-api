project "shado-opengl-api"
    kind "StaticLib"
    language "C++"
    staticruntime "off"
    cppdialect "C++20"

    if BUILD_CSHARP then
        dependson "Coral.Managed"
    end

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
        "%{wks.location}/shado-editor/src",
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
        "%{IncludeDir.shado_bin_serialization}"
    }

    -- Sibling (project) libraries. These are the same on every platform.
    links
    {
        "GLFW",
        "GLEW",
        "ImGui",
        "box2d",
        "yaml-cpp",
        "msdf-atlas-gen",
        "Coral.Native",
        "shado-bin-serialization"
    }

    defines
    {
        "GLEW_STATIC",
        "SHADO_ENABLE_ASSERTS",
        -- The bundled fmt in this spdlog fork uses consteval in a way that
        -- newer clang/gcc reject under C++20. Disable fmt's consteval checks.
        "FMT_CONSTEVAL=",
        "SHADO_OPENGL_SOLUTION_DIR_TODO_REMOVE=\"" .. path.getabsolute("%{wks.location}") .. "\""
    }

    filter "system:windows"
        staticruntime "Off"
        systemversion "latest"

        defines
        {
            "SHADO_PLATFORM_WINDOWS"
        }

        links
        {
            "gdi32.lib",
            "opengl32.lib",
            "shcore.lib"
        }

        -- On Windows the C# managed assemblies are built by the Visual Studio
        -- solution (Coral.Managed -> vendor/Coral/Build/<config>/, Shado-script-core
        -- -> shado-editor/ScriptCore/). Deploy them into shado-editor/DotNet so the
        -- editor's post-build step then copies them next to the executable and the
        -- scripting host can load them. (On macOS/Linux this deployment is handled
        -- by scripts/unix_compile_run_editor.sh instead.)
        postbuildcommands
        {
            '{MKDIR} "%{wks.location}/shado-editor/DotNet"',
            '{COPYFILE} "%{wks.location}/shado-opengl-api/vendor/Coral/Build/%{cfg.buildcfg}/Coral.Managed.dll" "%{wks.location}/shado-editor/DotNet/Coral.Managed.dll"',
            '{COPYFILE} "%{wks.location}/shado-opengl-api/vendor/Coral/Build/%{cfg.buildcfg}/Coral.Managed.runtimeconfig.json" "%{wks.location}/shado-editor/DotNet/Coral.Managed.runtimeconfig.json"',
            '{COPYFILE} "%{wks.location}/shado-opengl-api/vendor/Coral/Build/%{cfg.buildcfg}/Coral.Managed.deps.json" "%{wks.location}/shado-editor/DotNet/Coral.Managed.deps.json"',
        }

    filter "system:macosx"
        defines
        {
            "SHADO_PLATFORM_MACOS",
            "GL_SILENCE_DEPRECATION"
        }

        -- Objective-C++ implementations (file dialogs, etc.)
        files
        {
            "src/**.mm"
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
        pic "On"
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

    filter "configurations:Debug"
        defines {"SHADO_DEBUG", "SHADO_PROFILE"}
        symbols "On"

    filter "configurations:Release"
        defines "SHADO_RELEASE"
        optimize "On"

    filter "configurations:Dist"
        defines "SHADO_DIST"
        optimize "Full"

    -- NOTE: /Zc:preprocessor (MSVC's conforming preprocessor) breaks
    -- <windows.h>/winbase.h on Windows SDKs older than 10.0.22621 — every
    -- WINBASEAPI/WINAPI declaration fails to parse, producing a storm of
    -- "identifier not found" / "undeclared identifier" errors for all Win32
    -- functions. The engine only needs the traditional preprocessor's
    -- `, ##__VA_ARGS__` comma-elision (for the log/assert macros), which the
    -- default (traditional) MSVC preprocessor supports, so explicitly keep the
    -- conforming preprocessor OFF.
    filter "action:vs*"
        buildoptions { "/Zc:preprocessor-" }
