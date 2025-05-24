#include "UI.h"

#include <filesystem>

#include "asset/AssetManager.h"
#include "debug/Debug.h"
#include "Events/input.h"
#include "project/Project.h"
#include "renderer/Texture2D.h"
#include "scene/utils/SceneUtils.h"

namespace Shado {
    void UI::TreeNode(int id, const std::string& label, std::function<void()> ui, ImGuiTreeNodeFlags flags) {
        bool open = ImGui::TreeNodeEx((void*)(intptr_t)id, flags, label.c_str());
        if (open) {
            ui();
            ImGui::TreePop();
        }
    }

    void UI::DropDown(const std::string& label,
                      std::initializer_list<std::pair<std::string, std::function<void()>>> options,
                      std::string& currentType) {
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
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {0, 10});

        float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
        ImVec2 buttonSize = {lineHeight + 3.0f, lineHeight};

        ImGui::PushStyleColor(ImGuiCol_Button, {0.8f, 0.1f, 0.15f, 1.0f});
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, {0.9f, 0.2f, 0.2f, 1.0f});
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, {0.8f, 0.1f, 0.15f, 1.0f});
        ImGui::PushFont(boldFont);
        if (ImGui::Button(("X##" + label).c_str(), buttonSize))
            values.x = resetValue;
        ImGui::PopFont();
        ImGui::PopStyleColor(3);


        ImGui::SameLine();
        bool resultX = ImGui::DragFloat(("##X" + label).c_str(), &values.x, 0.1f, 0, 0, "%.2f");
        ImGui::PopItemWidth();
        ImGui::SameLine();


        ImGui::PushStyleColor(ImGuiCol_Button, {0.2f, 0.7f, 0.2f, 1.0f});
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, {0.3f, 0.8f, 0.3f, 1.0f});
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, {0.2f, 0.7f, 0.2f, 1.0f});
        ImGui::PushFont(boldFont);
        if (ImGui::Button(("Y##" + label).c_str(), buttonSize))
            values.y = resetValue;
        ImGui::PopFont();
        ImGui::PopStyleColor(3);


        ImGui::SameLine();
        bool resultY = ImGui::DragFloat(("##Y" + label).c_str(), &values.y, 0.1f, 0, 0, "%.2f");
        ImGui::PopItemWidth();
        ImGui::SameLine();


        ImGui::PushStyleColor(ImGuiCol_Button, {0.1f, 0.25f, 0.8f, 1.0f});
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, {0.2f, 0.35f, 0.9f, 1.0f});
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, {0.1f, 0.25f, 0.8f, 1.0f});
        ImGui::PushFont(boldFont);
        if (ImGui::Button(("Z##" + label).c_str(), buttonSize))
            values.z = resetValue;
        ImGui::PopFont();
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        bool resultZ = ImGui::DragFloat(("##Z" + label).c_str(), &values.z, 0.1f, 0, 0, "%.2f");
        ImGui::PopItemWidth();

        ImGui::PopStyleVar();

        ImGui::Columns(1);

        ImGui::PopID();

        return resultX || resultY || resultZ;
    }

    bool UI::Checkbox(const std::string& label, bool& data, float columnWidth) {
        ImGui::PushID(label.c_str());

        ImGui::Columns(2);
        ImGui::SetColumnWidth(0, columnWidth);
        ImGui::Text(label.c_str());
        ImGui::NextColumn();

        ImGui::PushMultiItemsWidths(1, ImGui::CalcItemWidth());
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {0, 10});

        bool modified = ImGui::Checkbox("##Vea5", &data);

        ImGui::PopItemWidth();
        ImGui::PopStyleVar();
        ImGui::Columns(1);
        ImGui::PopID();

        return modified;
    }

    bool UI::ColorControl(const std::string& label, glm::vec4& values, float columnWidth) {
        ImGui::PushID(label.c_str());

        ImGui::Columns(2);
        ImGui::SetColumnWidth(0, columnWidth);
        ImGui::Text(label.c_str());
        ImGui::NextColumn();

        ImGui::PushMultiItemsWidths(1, ImGui::CalcItemWidth());
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {0, 10});

        bool modified = ImGui::ColorEdit4("##eafefa4", (float*)&values);

        ImGui::PopItemWidth();
        ImGui::PopStyleVar();
        ImGui::Columns(1);
        ImGui::PopID();

        return modified;
    }

    void UI::InputTextWithChooseFile(const std::string& label, const std::string& text,
                                     const std::vector<std::string>& dragAndDropExtensions, int id,
                                     std::function<void(std::string)> onChange, UI::FileChooserType chooseType) {
        bool filepathHasChanged = false;

        std::string filepath = text;
        if (InputTextControl(label, filepath) && Shado::Input::isKeyPressed(Shado::KeyCode::Enter)) {
            filepathHasChanged = true;
        }

        // Double click opens the file
        if (ImGui::BeginPopupContextItem()) {
            if (ImGui::MenuItem("Open...")) {
                std::filesystem::path path = std::filesystem::path(filepath).is_absolute()
                                                 ? filepath
                                                 : Project::GetProjectDirectory() / filepath;
                Dialog::openPathInExplorer(path);
            }
            ImGui::EndPopup();
        }

        // For drag and drop
        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM")) {
                auto pathRelativeToProject = (const wchar_t*)payload->Data;
                std::filesystem::path dataPath = pathRelativeToProject;

                bool acceptable = dragAndDropExtensions.empty();
                for (const auto& ext : dragAndDropExtensions) {
                    if (ext == dataPath.extension()) {
                        acceptable = true;
                        break;
                    }
                }

                if (acceptable) {
                    filepath = dataPath.string();
                    onChange((Project::GetProjectDirectory() / dataPath).string());
                } else
                    SHADO_CORE_WARN("Invalid drag and drop file extension {0}", dataPath.filename());
            }

            ImGui::EndDragDropTarget();
        }

        // File choose
        ImGui::PushID(id);
        ImGui::SameLine();
        if (ImGui::Button("...", {24, 24})) {
            std::string buffer;
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

            switch (chooseType) {
            case UI::FileChooserType::Open:
                filepath = Shado::FileDialogs::openFile(filter.c_str());
                break;
            case UI::FileChooserType::Save:
                filepath = Shado::FileDialogs::saveFile(filter.c_str());
                break;
            case UI::FileChooserType::Folder:
                filepath = Shado::FileDialogs::chooseFolder();
                break;
            default:
                SHADO_CORE_ERROR("Unknown File dialog type ", (int)chooseType);
                break;
            }

            if (!filepath.empty()) {
                // Check if the path is inside the project directory
                if (filepath.find(Project::GetProjectDirectory().string()) == std::string::npos) {
                    SHADO_CORE_WARN("File {} is not inside the project directory", filepath);
                    std::filesystem::path copyToPath = Project::GetAssetDirectory() / std::filesystem::path(filepath).
                        filename();
                    std::filesystem::copy_file(filepath, copyToPath);
                    filepath = copyToPath.string();
                } else
                    filepath = std::filesystem::relative(filepath, Project::GetProjectDirectory()).string();

                filepathHasChanged = true;
            }
        }
        ImGui::PopID();

        if (filepathHasChanged && !filepath.empty())
            onChange(filepath);
    }

    void UI::TextureControl(AssetHandle& assetHandle) {
        // =========== Texture
        std::string texturePath = assetHandle ? AssetManager::GetPathFromHandle(assetHandle).string() : "No Texture";

        InputTextWithChooseFile("Texture", texturePath, {".jpg", ".png"}, typeid(Texture2D).hash_code(),
                                [&](std::string path) {
                                    assetHandle = Project::GetActive()->GetEditorAssetManager()->ImportAsset(path);
                                }
        );

        // Image
        if (assetHandle) {
            Ref<Texture2D> texture = AssetManager::GetAsset<Texture2D>(assetHandle);
            ImGui::Image((void*)texture->getRendererID(), {60, 60}, ImVec2(0, 1), ImVec2(1, 0));
        }

        ImGui::Separator();
    }

    bool UI::InputTextControl(const std::string& tag, std::string& value, ImGuiInputTextFlags flags,
                              float columnWidth) {
        ImGui::PushID(tag.c_str());

        ImGui::Columns(2);
        ImGui::SetColumnWidth(0, columnWidth);
        ImGui::Text("%s", tag.c_str());
        ImGui::NextColumn();

        ImGui::PushMultiItemsWidths(1, ImGui::CalcItemWidth());
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {0, 10});

        char buffer[512] = {};
        if (strcpy_s<512>(buffer, value.c_str()) != 0) {
            value = "<Error strcpy_s>";
            return false;
        }

        bool modified = false;
        if (ImGui::InputText("##textControl", buffer, sizeof(buffer), flags)) {
            value = std::string(buffer);
            modified = true;
        }

        ImGui::PopItemWidth();
        ImGui::PopStyleVar();
        ImGui::Columns(1);
        ImGui::PopID();

        return modified;
    }

    bool UI::ButtonControl(const std::string& value, const glm::vec2& size) {
        return ImGui::Button(value.c_str(), {size.x, size.y});
    }

    void UI::NewLine() {
        ImGui::NewLine();
    }

    void UI::SameLine(float offsetFromStart, float spacing) {
        ImGui::SameLine(offsetFromStart, spacing);
    }
}
