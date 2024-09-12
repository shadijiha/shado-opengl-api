#include "SceneUtils.h"
#include <commdlg.h>
#include "Application.h"
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <shtypes.h>
#include <GLFW/glfw3native.h>
#include <shlobj_core.h>

#include "Project/Project.h"

namespace Shado {

	std::string FileDialogs::openFile(const char* filter) {
		OPENFILENAMEA ofn;
		CHAR szFile[260] = { 0 };
		CHAR currentDir[256] = { 0 };
		
		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = glfwGetWin32Window(Application::get().getWindow().getNativeWindow());
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);

		if (Project::GetActive()) {
			std::strcpy(currentDir, Project::GetActive()->GetProjectDirectory().string().c_str());
		} else if (GetCurrentDirectoryA(256, currentDir))
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
		CHAR szFile[260] = { 0 };
		CHAR currentDir[256] = { 0 };
		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = glfwGetWin32Window(Application::get().getWindow().getNativeWindow());
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		
		if (Project::GetActive()) {
			std::strcpy(currentDir, Project::GetActive()->GetProjectDirectory().string().c_str());
		} else if (GetCurrentDirectoryA(256, currentDir))
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
		
		BROWSEINFO brwinfo = { 0 };
		brwinfo.lpszTitle = L"Select Your Source Directory";
		brwinfo.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE | BIF_EDITBOX | BIF_USENEWUI;
		LPITEMIDLIST pitemidl = SHBrowseForFolder(&brwinfo);

		if (pitemidl == 0)
			return "";
		
		// get the full path of the folder
		TCHAR path[MAX_PATH];
		std::wstring result;
		if (SHGetPathFromIDList(pitemidl, path))
		{
			result = path;
		}

		IMalloc* pMalloc = 0;
		if (SUCCEEDED(SHGetMalloc(&pMalloc)))
		{
			pMalloc->Free(pitemidl);
			pMalloc->Release();
		}

		return std::string(result.begin(), result.end());
	}

	/**
	 *
	 * Dialog class
	 */
	void Dialog::alert(const std::string& message, const std::string& alertTitle , DialogIcon icon) {
		std::wstring WMessage = std::wstring(message.begin(), message.end());
		std::wstring WTitle = std::wstring(alertTitle.begin(), alertTitle.end());
		HWND handler = glfwGetWin32Window(Application::get().getWindow().getNativeWindow());
		MessageBox(
			handler,
			WMessage.c_str(),
			WTitle.c_str(),
			(UINT)icon
		);
	}

	void Dialog::openPathInExplorer(const std::filesystem::path& path) {
		ShellExecuteA(NULL, "open", path.string().c_str(), NULL, NULL, SW_SHOWDEFAULT);
	}
}

