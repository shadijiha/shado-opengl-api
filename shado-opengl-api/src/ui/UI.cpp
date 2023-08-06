#include "UI.h"

#include <filesystem>

#include "debug/Debug.h"
#include "Events/input.h"
#include "scene/utils/SceneUtils.h"

namespace Shado {
	void UI::DropDown(const std::string& label, std::initializer_list<std::pair<std::string, std::function<void()>>> options, std::string& currentType) {
		if (ImGui::BeginCombo(label.c_str(), currentType.c_str())) {

			for (const auto& option : options) {
				bool isSelected = currentType == option.first;

				if (ImGui::Selectable(option.first.c_str(), isSelected)) {
					currentType = option.first;

					// Change camera type
					option.second();
				}

				if (isSelected)
					ImGui::SetItemDefaultFocus();
			}

			ImGui::EndCombo();
		}
	}

	// Draw a vector with colourful x, y and z
	bool UI::Vec3Control(const std::string& label, glm::vec3& values, float resetValue, float columnWidth) {
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];

		ImGui::PushID(label.c_str());

		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, columnWidth);
		ImGui::Text(label.c_str());
		ImGui::NextColumn();

		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0, 0 });

		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

		ImGui::PushStyleColor(ImGuiCol_Button, { 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.9f, 0.2f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("X", buttonSize))
			values.x = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);


		ImGui::SameLine();
		bool resultX = ImGui::DragFloat("##X", &values.x, 0.1f, 0, 0, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();


		ImGui::PushStyleColor(ImGuiCol_Button, { 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.3f, 0.8f, 0.3f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Y", buttonSize))
			values.y = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);


		ImGui::SameLine();
		bool resultY = ImGui::DragFloat("##Y", &values.y, 0.1f, 0, 0, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();


		ImGui::PushStyleColor(ImGuiCol_Button, { 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.2f, 0.35f, 0.9f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Z", buttonSize))
			values.z = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		bool resultZ = ImGui::DragFloat("##Z", &values.z, 0.1f, 0, 0, "%.2f");
		ImGui::PopItemWidth();

		ImGui::PopStyleVar();

		ImGui::Columns(1);

		ImGui::PopID();

		return  resultX || resultY || resultZ;
	}

	bool UI::ColorControl(const std::string& label, glm::vec4& values, float columnWidth) {
		return ImGui::ColorEdit4(label.c_str(), (float*)&values);
	}

	void UI::InputTextWithChooseFile(const std::string& label, const std::string& text,
		const std::vector<std::string>& dragAndDropExtensions, int id, std::function<void(std::string)> onChange, UI::FileChooserType chooseType) {
		bool textureChanged = false;

		std::string texturePath = text;
		if (InputTextControl(label, texturePath) && Shado::Input::isKeyPressed(Shado::KeyCode::Enter)) {
			textureChanged = true;
		}
		//ImGui::InputText(label.c_str(), (char*)text.c_str(), text.length(), ImGuiInputTextFlags_ReadOnly);


		// For drag and drop
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
			{
				const wchar_t* path = (const wchar_t*)payload->Data;
				std::filesystem::path dataPath = std::filesystem::path("assets") / path;

				bool acceptable = dragAndDropExtensions.empty();
				for (const auto& ext : dragAndDropExtensions) {
					if (ext == dataPath.extension()) {
						acceptable = true;
						break;
					}
				}

				if (acceptable)
					onChange(dataPath.string());
				else
					SHADO_CORE_WARN("Invalid drag and drop file extension {0}", dataPath.filename());
			}

			ImGui::EndDragDropTarget();
		}

		// File choose
		ImGui::PushID(id);
		ImGui::SameLine();
		if (ImGui::Button("...", { 24, 24 })) {

			std::string buffer = "";
			int count = 0;
			for (const auto& ext : dragAndDropExtensions) {
				buffer += "*" + ext;

				if (count != dragAndDropExtensions.size() - 1)
					buffer += ";";
				count++;
			}

			// Need to do this because we have \0 in string body
			std::string filter = "Files (";
			filter += std::string((buffer + ")\0").c_str(), buffer.length() + 2);
			filter += std::string((buffer + "\0").c_str(), buffer.length() + 1);

			switch (chooseType)
			{
				case UI::FileChooserType::Open:
					texturePath = Shado::FileDialogs::openFile(filter.c_str());
					break;
				case UI::FileChooserType::Save:
					texturePath = Shado::FileDialogs::saveFile(filter.c_str());
					break;
				case UI::FileChooserType::Folder:
					texturePath = Shado::FileDialogs::chooseFolder();
					break;
				default:
					SHADO_CORE_ERROR("Unknown File dialog type ", (int)chooseType);
					break;
			}

			textureChanged = true;
		}
		ImGui::PopID();


		if (textureChanged && !texturePath.empty())
			onChange(texturePath);

	}

	void UI::TextureControl(Ref<Shado::Texture2D>& texture) {

		// =========== Texture
		std::string texturePath = texture ? texture->getFilePath().c_str() : "No Texture";

		InputTextWithChooseFile("Texture", texturePath, { ".jpg", ".png" }, typeid(texture).hash_code(),
			[&](std::string path) {
				Ref<Shado::Texture2D> text = CreateRef<Shado::Texture2D>(path);
				if (text->isLoaded())
					texture = text;
				else
					SHADO_CORE_WARN("Could not load texture %s", path.c_str());
				SHADO_CORE_INFO("Loaded texture %s", path.c_str());
			}
		);

		// Image
		if (texture) {
			ImGui::Image((void*)texture->getRendererID(), { 60, 60 }, ImVec2(0, 1), ImVec2(1, 0));
		}

		ImGui::Separator();
	}

	bool UI::InputTextControl(const std::string& tag, std::string& value) {
		char buffer[512];
		memset(buffer, 0, sizeof(buffer));
		strcpy_s(buffer, sizeof(buffer), value.c_str());
		if (ImGui::InputText(tag.c_str(), buffer, sizeof(buffer))) {
			value = std::string(buffer);
			return true;
		}
		return false;
	}

	bool UI::ButtonControl(const std::string& value, const glm::vec2& size) {
		return ImGui::Button(value.c_str(), { size.x, size.y });
	}
}
