#include "ContentBrowserPanel.h"
#include <imgui.h>
#include <filesystem>

#include "debug/Profile.h"
#include "renderer/Texture2D.h"
#include "scene/utils/SceneUtils.h"
#include "util/Util.h"
#include "project/Project.h"

namespace Shado {
	static bool isImage(const std::filesystem::path& path);

	static Ref<Texture2D> getTextureCached(std::unordered_map<std::string, Ref<Texture2D>>& cache, std::filesystem::path path) {
		std::string str = path.string();
		if (cache.find(str) != cache.end()) {
			return cache[str];
		}

		Ref<Texture2D> texture = CreateRef<Texture2D>(path.string());
		cache[str] = texture;
		return texture;
	}

	ContentBrowserPanel::ContentBrowserPanel()
	{
		auto project = Project::GetActive();
		if (project) {
			m_CurrentDirectory = project->GetProjectDirectory();
			setDirectory(m_CurrentDirectory);
		}

		m_DirectoryIcon = CreateRef<Texture2D>("resources/icons/DirectoryIcon.png");
		m_FileIcon = CreateRef<Texture2D>("resources/icons/FileIcon.png");
		m_SceneIcon = CreateRef<Texture2D>("resources/icons/scene.png");
	}

	ContentBrowserPanel::ContentBrowserPanel(const std::filesystem::path& path)
		: m_CurrentDirectory(path)
	{
		m_DirectoryIcon = CreateRef<Texture2D>("resources/icons/DirectoryIcon.png");
		m_FileIcon = CreateRef<Texture2D>("resources/icons/FileIcon.png");
		m_SceneIcon = CreateRef<Texture2D>("resources/icons/scene.png");

		setDirectory(m_CurrentDirectory);
	}

	void ContentBrowserPanel::onImGuiRender() {
		SHADO_PROFILE_FUNCTION();

		ImGui::Begin("Content Browser");

		// If m_CurrentDirectory is not defined then display "Open a project to browse
		if (!Project::GetActive() || m_CurrentDirectory.empty()) {
			ImGui::Text("Open a project to browse");
			ImGui::End();
			return;
		}

		// Recheck the filesystem every 200 tick
		if (tick++ % 500 == 0) {
			setDirectory(m_CurrentDirectory);
			tick = 1;
		}

		{
			const auto& projectPath = Project::GetActive()->GetProjectDirectory();
			if (m_CurrentDirectory != projectPath)
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
				auto relativePath = std::filesystem::relative(path, projectPath);
				std::string filenameString = relativePath.filename().string();

				ImGui::PushID(filenameString.c_str());

				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));

				uint32_t rendererId = getThumbnail(directoryEntry);
				ImGui::ImageButton((ImTextureID)rendererId, { thumbnailSize, thumbnailSize }, { 0, 1 }, { 1, 0 });

				// Context menu to create 
				if (ImGui::BeginPopupContextItem()) {
					if (ImGui::MenuItem("Open..."))
						Dialog::openPathInExplorer(path.string());
					if (ImGui::MenuItem("Open in Explorer"))
						Dialog::openPathInExplorer(directoryEntry.is_directory() ? path.string() : path.parent_path().string());

					ImGui::EndPopup();
				}

				if (ImGui::BeginDragDropSource())
				{
					const wchar_t* itemPath = relativePath.c_str();
					ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", itemPath, (wcslen(itemPath) + 1) * sizeof(wchar_t));

					// Drag and drop preview
					if (isImage(path)) {
						Ref<Texture2D> text = getTextureCached(imagesThumbnails, path);
						ImGui::Image((void*)text->getRendererID(), { thumbnailSize / 2, thumbnailSize / 2 }, { 0, 1 }, { 1, 0 });
					} else {
						ImGui::Text(path.string().c_str());
					}

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
	}

	void ContentBrowserPanel::setDirectory(const std::filesystem::path& path) {
		SHADO_PROFILE_FUNCTION();

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
		if (isImage(path)) {
			icon = getTextureCached(imagesThumbnails, path);
		} else if (path.extension() == ".shadoscene") {
			icon = m_SceneIcon;
		}
		return icon->getRendererID();
	}

	static bool isImage(const std::filesystem::path& path) {
		return path.extension() == ".jpg" || path.extension() == ".png";
	}
}
