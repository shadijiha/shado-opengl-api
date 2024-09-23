#pragma once
#include "Asset.h"
#include "util/Memory.h"

namespace Shado {
    class Shader;
    class Texture2D;

    class AssetImporter {
    public:
        static Ref<Asset> ImportAsset(AssetHandle handle, const AssetMetadata& metadata);
    };

    class TextureImporter {
    public:
        // AssetMetadata filepath is relative to project asset directory
        static Ref<Texture2D> ImportTexture2D(AssetHandle handle, const AssetMetadata& metadata);
        // Reads file directly from filesystem
        // (i.e. path has to be relative / absolute to working directory)
        static Ref<Texture2D> LoadTexture2D(const std::filesystem::path& path);
    };

    class ShaderImporter {
    public:
        static Ref<Shader> ImportShader(AssetHandle handle, const AssetMetadata& metadata);
        static Ref<Shader> LoadShader(const std::filesystem::path& path);
    };
}
