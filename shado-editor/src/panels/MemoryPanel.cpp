#include "MemoryPanel.h"
#include "imgui.h"
#include "util/Memory.h"

namespace Shado {
	// Utility function to format storage size in a human-readable format
	static std::string FormatStorageSize(float sizeInBytes) {
		const char* units[] = { "B", "KB", "MB", "GB", "TB" };
		int unitIndex = 0;

		while (sizeInBytes >= 1024.0f && unitIndex < sizeof(units) / sizeof(units[0]) - 1) {
			sizeInBytes /= 1024.0f;
			unitIndex++;
		}

		char formattedSize[32];
		snprintf(formattedSize, sizeof(formattedSize), "%.2f %s", sizeInBytes, units[unitIndex]);

		return formattedSize;
	}

	MemoryPanel::MemoryPanel()
	{
	}

	void MemoryPanel::onImGuiRender()
	{
		ImGui::Begin("Memory");
		ImGui::Text("Total allocated: %s", FormatStorageSize((float)Memory::GetTotalAllocated()).c_str());
		ImGui::Text("Total alive: %s", FormatStorageSize((float)Memory::GetTotalAlive()).c_str());

		//ImGui::Text("Total allocated: %d", Memory::GetTotalAllocated());
		//ImGui::Text("Total alive: %d", Memory::GetTotalAlive());
		// Inside the ImGui window
		if (!Memory::GetMemoryHistory().empty()) {
			ImGui::Text("Heap Memory Usage Over Time");

			// Convert memory history data to ImGui-compatible arrays
			std::vector<float> timeData, memoryData;
			for (const auto& entry : Memory::GetMemoryHistory()) {
				timeData.push_back(entry.first);
				memoryData.push_back(static_cast<float>(entry.second));
			}

			// Display a graph using ImGui's PlotLines function
			ImGui::PlotHistogram("Heap Memory Usage", timeData.data(), timeData.size(),
				0, NULL, 0.0f, FLT_MAX, ImVec2(0, 200));
		}
		
		if (ImGui::BeginTable("Memory Breakdown", 2))
		{

			// Header
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("sector");
			ImGui::TableSetColumnIndex(1);
			ImGui::Text("total allocated");
			
			auto& temp = Memory::GetMemoryLabels();
			for (const auto& entry : temp)
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("%s", entry.first);
				ImGui::TableSetColumnIndex(1);
				ImGui::Text("%zu", entry.second);
			}
			ImGui::EndTable();
		}
		
		ImGui::End();
	}
}