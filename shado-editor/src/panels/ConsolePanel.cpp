#include "ConsolePanel.h"

#include <string>
#include <vector>

#include "imgui.h"
#include "debug/Debug.h"


namespace Shado	{
	ConsolePanel::ConsolePanel() {
	}

	void ConsolePanel::onImGuiRender() {

		std::vector<std::string> messages = Log::getMessages();

		char buffer[512];
		sprintf(buffer, "Console (%d)###Console", messages.size());
		ImGui::Begin(buffer);

		if(ImGui::Button("Clear")) {
			Log::clearMessages();
		}

		ImGui::BeginChild("Scrolling");
		ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, { 20.0f , 20.0f });
		//ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, { 10.0, 10.0 });
		for (const auto& message : messages) {
			ImGui::Image((ImTextureID)detectMessageType(message)->getRendererID(), {25.0f, 25.0}, { 0, 1 }, { 1, 0 });
			ImGui::SameLine();
			ImGui::Selectable(message.c_str());
			ImGui::Separator();
		}
		ImGui::PopStyleVar(1);
		ImGui::EndChild();	

		ImGui::End();
	}

	Ref<Texture2D> ConsolePanel::detectMessageType(const std::string& message) const {
		if (message.find("error") != std::string::npos || message.find("Error") != std::string::npos) {
			return m_ErrorIcon;
		}
		if (message.find("warning") != std::string::npos || message.find("warn") != std::string::npos) {
			return m_WarnIcon;
		}
		return m_InfoIcon;
	}
}


