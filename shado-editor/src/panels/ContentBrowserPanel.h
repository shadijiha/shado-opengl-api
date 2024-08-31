#pragma once
#include <filesystem>
#include <optional>

#include "renderer/Texture2D.h"
#include "util/Util.h"
#include <unordered_map>

namespace Shado {
	class ContentBrowserPanel {
	public:
		ContentBrowserPanel();
		ContentBrowserPanel(std::optional<std::filesystem::path> path);

		void onImGuiRender();

	private:
		void setDirectory(const std::filesystem::path& path);
		uint32_t getThumbnail(const std::filesystem::directory_entry&);
		
	private:
		std::optional<std::filesystem::path> m_CurrentDirectory;

		Ref<Texture2D> m_DirectoryIcon;
		Ref<Texture2D> m_FileIcon;
		Ref<Texture2D> m_SceneIcon;
		Ref<Texture2D> m_PrefabIcon;

		std::vector<std::filesystem::directory_entry> directories;
		uint32_t tick = 1;

		std::unordered_map<std::string, Ref<Texture2D>> imagesThumbnails;
	};

}
