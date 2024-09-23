#pragma once
#include <filesystem>

#include "ui/UUID.h"
#include "util/Memory.h"

namespace Shado {
    using AssetHandle = UUID;

    enum class AssetType : uint16_t {
        None = 0,
        Scene,
        Texture2D,
        Shader,
    };

    std::string_view AssetTypeToString(AssetType type);
    AssetType AssetTypeFromString(std::string_view assetType);

    class Asset : public RefCounted {
    public:
        virtual ~Asset() = default;

        AssetHandle Handle; // Generate handle
        virtual AssetType GetType() const = 0;
    };

    struct AssetMetadata {
        AssetType Type = AssetType::None;
        std::filesystem::path FilePath;
        operator bool() const { return Type != AssetType::None; }
    };
}
