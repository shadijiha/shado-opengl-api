#include "ContentBrowserPanel.h";
#include <imgui.h>
#include <filesystem>

#include "renderer/Texture2D.h"
#include "util/Util.h"

namespace Shado {

	// Change when projects are added
	extern const std::filesystem::path g_AssetsPath = "assets";


	ContentBrowserPanel::ContentBrowserPanel()
		: ContentBrowserPanel(g_AssetsPath)
	{
	}

	ContentBrowserPanel::ContentBrowserPanel(const std::filesystem::path& path)
		: m_CurrentDirectory(path)
	{
		m_DirectoryIcon = CreateRef<Texture2D>("resources/icons/DirectoryIcon.png");
		m_FileIcon = CreateRef<Texture2D>("resources/icons/FileIcon.png");

		setDirectory(m_CurrentDirectory);
	}

	void ContentBrowserPanel::onImGuiRender() {

		// Recheck the filesystem every 200 tick
		if (tick++ % 500 == 0)
			setDirectory(m_CurrentDirectory);

		ImGui::Begin("Content Browser");

		if (m_CurrentDirectory != std::filesystem::path(g_AssetsPath))
		{
			if (ImGui::Button("<-"))
			{
				m_CurrentDirectory = m_CurrentDirectory.parent_path();
				setDirectory(m_CurrentDirectory);
			}
		}

		static float padding = 25.0f;
		static float thumbnailSize = 100.0f;
		float cellSize = thumbnailSize + padding;

		float panelWidth = ImGui::GetContentRegionAvail().x;
		int columnCount = (int)(panelWidth / cellSize);
		if (columnCount < 1)
			columnCount = 1;

		ImGui::Columns(columnCount, 0, false);

		bool changeDir = false;
		for (auto& directoryEntry : directories)
		{
			const auto& path = directoryEntry.path();
			auto relativePath = std::filesystem::relative(path, g_AssetsPath);
			std::string filenameString = relativePath.filename().string();

			ImGui::PushID(filenameString.c_str());
			
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));

			uint32_t rendererId = getThumbnail(directoryEntry);
			ImGui::ImageButton((ImTextureID)rendererId, { thumbnailSize, thumbnailSize }, { 0, 1 }, { 1, 0 });
			

			if (ImGui::BeginDragDropSource())
			{
				const wchar_t* itemPath = relativePath.c_str();
				ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", itemPath, (wcslen(itemPath) + 1) * sizeof(wchar_t));
				ImGui::EndDragDropSource();
			}

			ImGui::PopStyleColor();
			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				if (directoryEntry.is_directory()) {
					m_CurrentDirectory /= path.filename();
					changeDir = true;
					
				}				

			}
			ImGui::TextWrapped(filenameString.c_str());

			ImGui::NextColumn();

			ImGui::PopID();
		}

		ImGui::Columns(1);

		//ImGui::SliderFloat("Thumbnail Size", &thumbnailSize, 16, 512);
		//ImGui::SliderFloat("Padding", &padding, 0, 32);

		// TODO: status bar
		ImGui::End();

		if (changeDir)
			setDirectory(m_CurrentDirectory);
	}

	void ContentBrowserPanel::setDirectory(const std::filesystem::path& path) {
		directories.clear();
		for (auto& directoryEntry : std::filesystem::directory_iterator(path)) {
			directories.push_back(directoryEntry);
		}

		using P = std::filesystem::directory_entry;
		std::sort(directories.begin(), directories.end(), [](const P& a, const P& b) {
			if (a.is_directory() && !b.is_directory()) return true;
			if (!a.is_directory() && b.is_directory()) return false;
			return a.path().filename() < b.path().filename();
		});
	}

	uint32_t ContentBrowserPanel::getThumbnail(const std::filesystem::directory_entry& directoryEntry) {
		Ref<Texture2D> icon = directoryEntry.is_directory() ? m_DirectoryIcon : m_FileIcon;

		// If it is an image
		const auto path = directoryEntry.path();
		if (path.extension() == ".jpg" || path.extension() == ".png") {
			if (imagesThumbnails.find(path.string()) == imagesThumbnails.end()) {
				imagesThumbnails[path.string()] = CreateRef<Texture2D>(path.string());
			}
			icon = imagesThumbnails[path.string()];

		}
		return icon->getRendererID();
	}
}
