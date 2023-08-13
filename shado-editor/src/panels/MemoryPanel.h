#pragma once
#ifndef MEMORY_H
#define MEMORY_H
#include <filesystem>
#include "renderer/Texture2D.h"
#include "util/Util.h"
#include <unordered_map>

namespace Shado {
	class MemoryPanel {
	public:
		MemoryPanel();
		~MemoryPanel() = default;

		void onImGuiRender();
	};
}
#endif // !MEMORY_H