#pragma once
#include <filesystem>

#include "renderer/Texture2D.h"
#include "util/Util.h"

namespace Shado {
	class ContentBrowserPanel {
	public:
		ContentBrowserPanel();
		ContentBrowserPanel(const std::filesystem::path& path);

		void onImGuiRender();

	private:
		std::filesystem::path m_CurrentDirectory;

		Ref<Texture2D> m_DirectoryIcon;
		Ref<Texture2D> m_FileIcon;
	};
}
