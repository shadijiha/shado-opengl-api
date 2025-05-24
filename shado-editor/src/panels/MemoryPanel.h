#pragma once
#ifndef MEMORY_H
#define MEMORY_H
#include <filesystem>
#include "renderer/Texture2D.h"
#include "util/TimeStep.h"
#include <unordered_map>

namespace Shado {
	class MemoryPanel {
	public:
		MemoryPanel() = default;
		~MemoryPanel() = default;

		void onImGuiRender() const;
	};
}
#endif // !MEMORY_H