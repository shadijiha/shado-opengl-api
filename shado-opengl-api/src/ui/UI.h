#pragma once
#include <functional>
#include <map>
#include <string>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <iterator>

#include "Application.h"
#include "imgui_internal.h"

namespace Shado {
    class UI {
    public:
        enum class FileChooserType {
            Open = 0, Save, Folder
        };

        static void TreeNode(int id, const std::string& label, std::function<void()> ui,
                             ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen |
                                 ImGuiTreeNodeFlags_AllowItemOverlap
                                 | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth |
                                 ImGuiTreeNodeFlags_FramePadding
                                 | ImGuiTreeNodeFlags_AllowItemOverlap);

        static void DropDown(const std::string& label,
                             std::initializer_list<std::pair<std::string, std::function<void()>>> options,
                             std::string& currentType);

        static bool Vec3Control(const std::string& label, glm::vec3& values, float resetValue = 0.0f,
                                float columnWidth = 100.0f);

        template <typename T> requires std::is_same_v<T, float> || std::is_same_v<T, int>
        static bool Vec1Control(const std::string& label, T& value, T resetValue = 0.0f, float columnWidth = 100.0f,
                                ImGuiSliderFlags flags = 0);

        static bool Checkbox(const std::string& label, bool& data, float columnWidth = 100.0f);

        static bool ColorControl(const std::string& label, glm::vec4& values, float columnWidth = 100.0f);

        static void InputTextWithChooseFile(
            const std::string& label, const std::string& text, const std::vector<std::string>& dragAndDropExtensions,
            int id,
            std::function<void(std::string)> onChange,
            FileChooserType type = FileChooserType::Open
        );

        static void TextureControl(Ref<Shado::Texture2D>& texture);

        static bool InputTextControl(const std::string& tag, std::string& value, ImGuiInputTextFlags flags = 0,
                                     float columnWidth = 100.0f);

        static bool ButtonControl(const std::string& value, const glm::vec2& size = {0, 0});

        template <typename... Args>
        static void Text(const std::string& format, Args... args) {
            ImGui::Text(format.c_str(), args...);
        }

        static void NewLine();

        static void SameLine(float offsetFromStart = 0, float spacing = -1);

        template <typename Iterator>
        static bool Table(const std::string& label,
                          Iterator begin,
                          Iterator end,
                          const std::map<std::string, std::function<void(decltype(*begin)&, int)>>& columnHeaderUi,
                          ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg);
    };

    template <typename T> requires std::is_same_v<T, float> || std::is_same_v<T, int>
    bool UI::Vec1Control(const std::string& label, T& value, T resetValue, float columnWidth, ImGuiSliderFlags flags) {
        ImGuiIO& io = ImGui::GetIO();
        auto boldFont = io.Fonts->Fonts[0];

        ImGui::PushID(label.c_str());

        ImGui::Columns(2);
        ImGui::SetColumnWidth(0, columnWidth);
        ImGui::Text(label.c_str());
        ImGui::NextColumn();

        ImGui::PushMultiItemsWidths(1, ImGui::CalcItemWidth());
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {0, 10});

        float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
        ImVec2 buttonSize = {lineHeight + 3.0f, lineHeight};

        auto color = ImVec4{0.3f, 0.3f, 0.3f, 1.0f};
        ImGui::PushStyleColor(ImGuiCol_Button, color);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, {color.x + 0.1f, color.y + 0.1f, color.z + 0.1f, color.w});
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, color);
        ImGui::PushFont(boldFont);

        bool modified = false;
        if (ImGui::Button("-", buttonSize)) {
            value = resetValue;
            modified = true;
        }
        ImGui::PopFont();
        ImGui::PopStyleColor(3);

        ImGui::SameLine();

        if constexpr (std::is_same_v<T, float>)
            modified = modified ||
                ImGui::DragFloat(("##V" + label).c_str(), (float*)&value, 0.01f, 0, 0, "%.2f", flags);
        else if constexpr (std::is_same_v<T, int>)
            modified = modified || ImGui::DragInt(("##V" + label).c_str(), (int*)&value, 1, 0, 0, "%d", flags);
        else {
            static_assert(false, "Vec1Control only supports float and int types");
        }

        ImGui::PopItemWidth();
        ImGui::PopStyleVar();
        ImGui::Columns(1);
        ImGui::PopID();

        return modified;
    }

    template <typename Iterator>
    bool UI::Table(const std::string& label,
                   Iterator begin,
                   Iterator end,
                   const std::map<std::string, std::function<void(decltype(*begin)&, int)>>& columnHeaderUi,
                   ImGuiTableFlags flags) {
        if (ImGui::BeginTable(label.c_str(), columnHeaderUi.size(), flags)) {
            for (const auto& [header, ui] : columnHeaderUi) {
                ImGui::TableSetupColumn(header.c_str());
            }
            ImGui::TableHeadersRow();

            for (auto it = begin; it != end; ++it) {
                ImGui::TableNextRow();

                auto& item = *it;
                int i = 0;
                for (const auto& [header, ui] : columnHeaderUi) {
                    ImGui::TableSetColumnIndex(i);
                    ui(item, i);
                    i++;
                }
            }


            ImGui::EndTable();
            return true;
        }
        return false;
    }

    struct ScopedStyleColor {
        ScopedStyleColor() = default;

        ScopedStyleColor(ImGuiCol idx, ImVec4 color, bool predicate = true)
            : m_Set(predicate) {
            if (predicate)
                ImGui::PushStyleColor(idx, color);
        }

        ScopedStyleColor(ImGuiCol idx, ImU32 color, bool predicate = true)
            : m_Set(predicate) {
            if (predicate)
                ImGui::PushStyleColor(idx, color);
        }

        ~ScopedStyleColor() {
            if (m_Set)
                ImGui::PopStyleColor();
        }

    private:
        bool m_Set = false;
    };
}
