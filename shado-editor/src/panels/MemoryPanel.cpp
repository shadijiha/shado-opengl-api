#include "MemoryPanel.h"

#include <algorithm>

#include "imgui.h"
#include "ui/UI.h"
#include "util/Memory.h"


namespace Shado {
	/**
	 * Utility function to format storage size in a human-readable format
	 * @param sizeInBytes  
	 * @return 
	 */
	static std::string FormatStorageSize(float sizeInBytes) {
		constexpr const char* units[] = { "B", "KB", "MB", "GB", "TB" };
		int unitIndex = 0;

		while (sizeInBytes >= 1024.0f && unitIndex < std::size(units) - 1) {
			sizeInBytes /= 1024.0f;
			unitIndex++;
		}

		return std::format("{:.2f} {}", sizeInBytes, units[unitIndex]);
	}

	void MemoryPanel::onImGuiRender() const
	{
		ImGui::Begin("Memory");
		ImGui::Text("Total allocated: %s", FormatStorageSize(static_cast<float>(Memory::GetTotalAllocated())).c_str());
		ImGui::Text("Total alive: %s", FormatStorageSize(static_cast<float>(Memory::GetTotalAlive())).c_str());

		// Inside the ImGui window
		if (!Memory::GetMemoryHistory().empty()) {
			// Convert memory history data to ImGui-compatible arrays
			auto& memoryHistory = Memory::GetMemoryHistory();

			// Extract memory values into a uint64_t buffer
			std::vector<float> memoryValues;
			memoryValues.reserve(memoryHistory.size());
			for (const auto& [timestamp, mem] : memoryHistory) {
				memoryValues.push_back(static_cast<float>(mem));
			}

			// Display a graph using ImGui's PlotLines function
			ImGui::PlotLines(
				"Heap Total Alive Memory Usage (MB)",
				memoryValues.data(),
				static_cast<int>(memoryValues.size()),
				0,
				nullptr,
				0.0f,
				*std::ranges::max_element(memoryValues),
				ImVec2(0, 80)
			);
		}

		auto memoryLabels = Memory::GetMemoryLabels();
		static bool detailedBreakdown = true;
		UI::Checkbox("Detailed", detailedBreakdown);
		
		if (detailedBreakdown) {
			// Aggregate all labels that start with "Engine -"
			uint64_t totalEngineMemory = 0;
			for (const auto& [label, mem] : memoryLabels) {
				if (label.starts_with("Engine -")) {
					totalEngineMemory += mem;
				}
			}

			std::erase_if(memoryLabels, [](const auto& entry) {
				return entry.first.starts_with("Engine -");
			});

			memoryLabels["Engine"] = totalEngineMemory;
		}
		
		UI::Table(
			"Memory Breakdown",
			memoryLabels.begin(),
			memoryLabels.end(),
			{
				{"Sector", [](const auto& entry, int i) {
					UI::Text("%s", entry.first.c_str());
				}},
				{"Total allocated", [](const auto& entry, int i) {
					UI::Text("%s", FormatStorageSize(static_cast<float>(entry.second)).c_str());
				}}
			}
		);
		
		ImGui::End();
	}
}