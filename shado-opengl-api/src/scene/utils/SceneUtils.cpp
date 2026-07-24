#include "SceneUtils.h"

// The macOS implementation lives in SceneUtils.mm (Objective-C++). This
// translation unit provides the Windows and Linux implementations.

#if defined(SHADO_PLATFORM_WINDOWS)

// <Windows.h> must be included before the Windows sub-headers below
// (commdlg.h, shtypes.h, shlobj_core.h, glfw3native.h with GLFW_EXPOSE_NATIVE_WIN32),
// which rely on its types. Previously this ordering was provided by
// SceneUtils.h; keep it explicit here now that the header is platform-neutral.
#include <Windows.h>
#include <winuser.h>
#include <commdlg.h>
#include "Application.h"
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <shtypes.h>
#include <GLFW/glfw3native.h>
#include <shlobj_core.h>

#include "Project/Project.h"
#include "asset/AssetManager.h" // <--- This is needed DO NOT REMOVE

namespace Shado {
    static UINT ToNativeIcon(Dialog::DialogIcon icon) {
        switch (icon) {
        case Dialog::DialogIcon::EXCLAMATION: return MB_ICONEXCLAMATION;
        case Dialog::DialogIcon::WARNING:     return MB_ICONWARNING;
        case Dialog::DialogIcon::INFORMATION: return MB_ICONINFORMATION;
        case Dialog::DialogIcon::ASTERISK:    return MB_ICONASTERISK;
        case Dialog::DialogIcon::QUESTION:    return MB_ICONQUESTION;
        case Dialog::DialogIcon::STOP:        return MB_ICONSTOP;
        case Dialog::DialogIcon::ERROR_ICON:  return MB_ICONERROR;
        case Dialog::DialogIcon::HAND:        return MB_ICONHAND;
        }
        return MB_ICONWARNING;
    }

    std::string FileDialogs::openFile(const char* filter) {
        OPENFILENAMEA ofn;
        CHAR szFile[260] = {0};
        CHAR currentDir[256] = {0};

        ZeroMemory(&ofn, sizeof(OPENFILENAME));
        ofn.lStructSize = sizeof(OPENFILENAME);
        ofn.hwndOwner = glfwGetWin32Window(Application::get().getWindow().getNativeWindow());
        ofn.lpstrFile = szFile;
        ofn.nMaxFile = sizeof(szFile);

        if (Project::GetActive()) {
            std::strcpy(currentDir, Project::GetActive()->GetProjectDirectory().string().c_str());
        }
        else if (GetCurrentDirectoryA(256, currentDir))
            ofn.lpstrInitialDir = currentDir;

        ofn.lpstrFilter = filter;
        ofn.nFilterIndex = 1;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

        if (GetOpenFileNameA(&ofn) == TRUE)
            return ofn.lpstrFile;

        return std::string();
    }

    std::string FileDialogs::saveFile(const char* filter) {
        OPENFILENAMEA ofn;
        CHAR szFile[260] = {0};
        CHAR currentDir[256] = {0};
        ZeroMemory(&ofn, sizeof(OPENFILENAME));
        ofn.lStructSize = sizeof(OPENFILENAME);
        ofn.hwndOwner = glfwGetWin32Window(Application::get().getWindow().getNativeWindow());
        ofn.lpstrFile = szFile;
        ofn.nMaxFile = sizeof(szFile);

        if (Project::GetActive()) {
            std::strcpy(currentDir, Project::GetActive()->GetProjectDirectory().string().c_str());
        }
        else if (GetCurrentDirectoryA(256, currentDir))
            ofn.lpstrInitialDir = currentDir;

        ofn.lpstrFilter = filter;
        ofn.nFilterIndex = 1;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;

        // Sets the default extension by extracting it from the filter
        ofn.lpstrDefExt = strchr(filter, '\0') + 1;

        if (GetSaveFileNameA(&ofn) == TRUE)
            return ofn.lpstrFile;

        return std::string();
    }

    std::string FileDialogs::chooseFolder() {
        BROWSEINFO brwinfo = {0};
        brwinfo.lpszTitle = L"Select Your Source Directory";
        brwinfo.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE | BIF_EDITBOX | BIF_USENEWUI;
        LPITEMIDLIST pitemidl = SHBrowseForFolder(&brwinfo);

        if (pitemidl == 0)
            return "";

        // get the full path of the folder
        TCHAR path[MAX_PATH];
        std::wstring result;
        if (SHGetPathFromIDList(pitemidl, path)) {
            result = path;
        }

        IMalloc* pMalloc = 0;
        if (SUCCEEDED(SHGetMalloc(&pMalloc))) {
            pMalloc->Free(pitemidl);
            pMalloc->Release();
        }

        return std::string(result.begin(), result.end());
    }

    void Dialog::alert(const std::string& message, const std::string& alertTitle, DialogIcon icon) {
        std::wstring WMessage = std::wstring(message.begin(), message.end());
        std::wstring WTitle = std::wstring(alertTitle.begin(), alertTitle.end());
        HWND handler = glfwGetWin32Window(Application::get().getWindow().getNativeWindow());
        MessageBox(
            handler,
            WMessage.c_str(),
            WTitle.c_str(),
            ToNativeIcon(icon)
        );
    }

    void Dialog::openPathInExplorer(const std::filesystem::path& path) {
        ShellExecuteA(NULL, "open", path.string().c_str(), NULL, NULL, SW_SHOWDEFAULT);
    }
}

#elif defined(SHADO_PLATFORM_LINUX)

#include <array>
#include <cstdio>
#include <memory>
#include "debug/Debug.h"

namespace Shado {
    // Linux implementation shells out to a portable dialog helper (zenity is
    // available on most desktop environments). If none is present the calls
    // degrade gracefully to empty results / no-ops.
    static std::string RunCommand(const std::string& cmd) {
        std::array<char, 512> buffer{};
        std::string result;
        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
        if (!pipe)
            return "";
        while (fgets(buffer.data(), (int)buffer.size(), pipe.get()) != nullptr)
            result += buffer.data();
        // Strip trailing newline
        while (!result.empty() && (result.back() == '\n' || result.back() == '\r'))
            result.pop_back();
        return result;
    }

    std::string FileDialogs::openFile(const char* filter) {
        (void)filter;
        return RunCommand("zenity --file-selection 2>/dev/null");
    }

    std::string FileDialogs::saveFile(const char* filter) {
        (void)filter;
        return RunCommand("zenity --file-selection --save --confirm-overwrite 2>/dev/null");
    }

    std::string FileDialogs::chooseFolder() {
        return RunCommand("zenity --file-selection --directory 2>/dev/null");
    }

    void Dialog::alert(const std::string& message, const std::string& alertTitle, DialogIcon icon) {
        (void)icon;
        std::string cmd = "zenity --warning --title=\"" + alertTitle +
            "\" --text=\"" + message + "\" 2>/dev/null";
        if (system(cmd.c_str()) != 0)
            SHADO_CORE_WARN("[Dialog] {}: {}", alertTitle, message);
    }

    void Dialog::openPathInExplorer(const std::filesystem::path& path) {
        std::string cmd = "xdg-open \"" + path.string() + "\" 2>/dev/null &";
        (void)system(cmd.c_str());
    }
}

#endif
