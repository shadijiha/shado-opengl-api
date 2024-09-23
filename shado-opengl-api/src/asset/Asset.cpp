#include "Asset.h"

namespace Shado {
    std::string_view AssetTypeToString(AssetType type) {
        switch (type) {
        case AssetType::None: return "AssetType::None";
        case AssetType::Scene: return "AssetType::Scene";
        case AssetType::Texture2D: return "AssetType::Texture2D";
        case AssetType::Shader: return "AssetType::Shader";
        }
        return "AssetType::<Invalid>";
    }

    AssetType AssetTypeFromString(std::string_view assetType) {
        if (assetType == "AssetType::None") return AssetType::None;
        if (assetType == "AssetType::Scene") return AssetType::Scene;
        if (assetType == "AssetType::Texture2D") return AssetType::Texture2D;
        if (assetType == "AssetType::Shader") return AssetType::Shader;
        return AssetType::None;
    }
}
