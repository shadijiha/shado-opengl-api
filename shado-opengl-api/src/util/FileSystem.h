#pragma once

#include "Buffer.h"
#include <filesystem>

namespace Shado {
    class FileSystem {
    public:
        // TODO: move to FileSystem class
        static Buffer ReadFileBinary(const std::filesystem::path& filepath);

        static std::filesystem::path
        NewDirectory(const std::filesystem::path& path, const std::string& folderName = "");

        static void DeleteFile(const std::filesystem::path& path);
    };
}
