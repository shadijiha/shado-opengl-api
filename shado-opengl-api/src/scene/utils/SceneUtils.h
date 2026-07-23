#pragma once
#include <filesystem>
#include <string>

namespace Shado {

	class FileDialogs {
	public:
		// filter uses the Win32 double-null-terminated format on Windows, e.g.
		// "Shado Project (*.sproj)\0*.sproj\0". On other platforms the filter is
		// used on a best-effort basis (or ignored).
		static std::string openFile(const char* filter);
		static std::string saveFile(const char* filter);

		static std::string chooseFolder();
	};

	class Dialog {
	public:
		// Platform-neutral icon identifiers. Each platform implementation maps
		// these onto its native dialog icon constants.
		enum class DialogIcon {
			EXCLAMATION,
			WARNING,
			INFORMATION,
			ASTERISK,
			QUESTION,
			STOP,
			ERROR_ICON,
			HAND
		};

		static void alert(const std::string& message, const std::string& alertTitle = "Alert", DialogIcon icon = DialogIcon::WARNING);

		static void openPathInExplorer(const std::filesystem::path& path);
	};
}
