#pragma once
#include <functional>
#include <string>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include "Application.h"
#include "imgui_internal.h"

namespace Shado {
	class UI {
	public:
		enum class FileChooserType {
			Open = 0, Save, Folder
		};

		template<typename T>
		static void TreeNode(const std::string& label, std::function<void()> ui);

		static void DropDown(const std::string& label, std::initializer_list<std::pair<std::string, std::function<void()>>> options, std::string& currentType);

		static bool Vec3Control(const std::string& label, glm::vec3& values, float resetValue = 0.0f, float columnWidth = 100.0f);

		template<typename T>
		static bool Vec1Control(const std::string& label, T& value, T resetValue = 0.0f, float columnWidth = 100.0f);

		static bool ColorControl(const std::string& label, glm::vec4& values, float columnWidth = 100.0f);

		static void InputTextWithChooseFile(
			const std::string& label, const std::string& text, const std::vector<std::string>& dragAndDropExtensions, int id,
			std::function<void(std::string)> onChange,
			FileChooserType type = FileChooserType::Open
		);

		static void TextureControl(Ref<Shado::Texture2D>& texture);

		static bool InputTextControl(const std::string& tag, std::string& value);

		static bool ButtonControl(const std::string& value, const glm::vec2& size = { 0, 0 });

	};

	template <typename T>
	void UI::TreeNode(const std::string& label, std::function<void()> ui) {
		const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap
			| ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_FramePadding
			| ImGuiTreeNodeFlags_AllowItemOverlap;

		// Olaf settings
		bool open = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), treeNodeFlags, label.c_str());
		if (open) {
			ui();
			ImGui::TreePop();
		}
		ImGui::NewLine();
	}

	template <typename T>
	bool UI::Vec1Control(const std::string& label, T& value, T resetValue, float columnWidth) {
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];

		ImGui::PushID(label.c_str());

		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, columnWidth);
		ImGui::Text(label.c_str());
		ImGui::NextColumn();

		ImGui::PushMultiItemsWidths(1, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0, 0 });

		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

		auto color = ImVec4{ 57.0f / 255, 3.0f / 255, 252.0f / 255, 1.0f };
		ImGui::PushStyleColor(ImGuiCol_Button, color);
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { color.x + 0.1f, color.y + 0.1f, color.z + 0.05f, color.w });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, color);
		ImGui::PushFont(boldFont);
		if (ImGui::Button("-", buttonSize))
			value = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);


		ImGui::SameLine();
		bool result = false;

		if (typeid(T) == typeid(float))
			result = ImGui::DragFloat("##V", (float*)&value, 0.01f, 0, 0, "%.2f");
		else if (typeid(T) == typeid(int))
			result = ImGui::DragInt("##V", (int*)&value);

		ImGui::PopItemWidth();

		ImGui::PopStyleVar();

		ImGui::Columns(1);

		ImGui::PopID();

		return result;
	}

	struct ScopedStyleColor
	{
		ScopedStyleColor() = default;

		ScopedStyleColor(ImGuiCol idx, ImVec4 color, bool predicate = true)
			: m_Set(predicate)
		{
			if (predicate)
				ImGui::PushStyleColor(idx, color);
		}

		ScopedStyleColor(ImGuiCol idx, ImU32 color, bool predicate = true)
			: m_Set(predicate)
		{
			if (predicate)
				ImGui::PushStyleColor(idx, color);
		}

		~ScopedStyleColor()
		{
			if (m_Set)
				ImGui::PopStyleColor();
		}
	private:
		bool m_Set = false;
	};

}