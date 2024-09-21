#include <fstream>
#include "FileSystem.h"

#include "debug/Debug.h"

namespace Shado {
    Buffer FileSystem::ReadFileBinary(const std::filesystem::path& filepath) {
        std::ifstream stream(filepath, std::ios::binary | std::ios::ate);

        if (!stream) {
            // Failed to open the file
            return {};
        }


        std::streampos end = stream.tellg();
        stream.seekg(0, std::ios::beg);
        uint64_t size = end - stream.tellg();

        if (size == 0) {
            // File is empty
            return {};
        }

        Buffer buffer;
        buffer.Allocate(size);
        stream.read(buffer.As<char>(), size);
        stream.close();
        return buffer;
    }

    std::filesystem::path FileSystem::NewDirectory(const std::filesystem::path& path, const std::string& folderName) {
        std::filesystem::path folderPath = folderName.empty() ? path : path / folderName;
        if (!std::filesystem::exists(folderPath)) {
            std::filesystem::create_directory(folderPath);
        }
        else {
            SHADO_CORE_ERROR("Folder already exists: {0}", folderPath.string());
        }
        return folderPath;
    }

    void FileSystem::DeleteFile(const std::filesystem::path& path) {
        if (std::filesystem::exists(path)) {
            std::filesystem::remove(path);
        }
    }
}
