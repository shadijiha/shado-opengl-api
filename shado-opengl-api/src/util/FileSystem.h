#pragma once

#include "Buffer.h"
#include <filesystem>

namespace Shado {

	class FileSystem
	{
	public:
		// TODO: move to FileSystem class
		static Buffer ReadFileBinary(const std::filesystem::path& filepath);
	};

}