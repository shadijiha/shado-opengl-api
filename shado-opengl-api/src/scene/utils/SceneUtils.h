#pragma once
#include <string>
#include <Windows.h>
#include <winuser.h>

namespace Shado {

	class FileDialogs {
	public:
		static std::string openFile(const char* filter);
		static std::string saveFile(const char* filter);

	};

	class Dialog {
	public:
		enum class DialogIcon {
			EXCLAMATION = MB_ICONEXCLAMATION,
			WARNING = MB_ICONWARNING,
			INFORMATION = MB_ICONINFORMATION,
			ASTERISK = MB_ICONASTERISK,
			QUESTION = MB_ICONQUESTION,
			STOP = MB_ICONSTOP,
			ERROR_ICON = MB_ICONERROR,
			HAND = MB_ICONHAND
		};

		static void alert(const std::string& message, const std::string& alertTitle = "Alert", DialogIcon icon = DialogIcon::WARNING);

		static void openPathInExplorer(const std::string& path);
	};
}