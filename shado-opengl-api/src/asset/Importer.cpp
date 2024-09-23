#include "Importer.h"

#include <functional>
#include <map>

#include "stb_image.h"
#include "debug/Debug.h"
#include "debug/Profile.h"
#include "project/Project.h"
#include "renderer/Texture2D.h"
#include "renderer/Shader.h"
#include "util/Buffer.h"
#include "asset/AssetManager.h"

namespace Shado {
    using AssetImportFunction = std::function<Ref<Asset>(AssetHandle, const AssetMetadata&)>;

    static std::map<AssetType, AssetImportFunction> s_AssetImportFunctions = {
        {AssetType::Texture2D, TextureImporter::ImportTexture2D},
        {AssetType::Shader, ShaderImporter::ImportShader},
    };

    Ref<Asset> AssetImporter::ImportAsset(AssetHandle handle, const AssetMetadata& metadata) {
        if (s_AssetImportFunctions.find(metadata.Type) == s_AssetImportFunctions.end()) {
            SHADO_CORE_ERROR("No importer available for asset type: {}", (uint16_t)metadata.Type);
            return nullptr;
        }
        return s_AssetImportFunctions.at(metadata.Type)(handle, metadata);
    }


    Ref<Texture2D> TextureImporter::ImportTexture2D(AssetHandle handle, const AssetMetadata& metadata) {
        SHADO_PROFILE_FUNCTION();
        return LoadTexture2D(Project::GetProjectDirectory() / metadata.FilePath);
    }

    Ref<Texture2D> TextureImporter::LoadTexture2D(const std::filesystem::path& path) {
        SHADO_PROFILE_FUNCTION();

        int width, height, channels;
        stbi_set_flip_vertically_on_load(1);
        Buffer data;

        {
            SHADO_PROFILE_SCOPE("stbi_load - TextureImporter::ImportTexture2D");
            std::string pathStr = path.string();
            data.Data = stbi_load(pathStr.c_str(), &width, &height, &channels, 0);
        }

        if (data.Data == nullptr) {
            SHADO_PROFILE_SCOPE("TextureImporter::ImportTexture2D - Could not load texture from filepath: {}",
                                path.string());
            return nullptr;
        }


        // TODO: think about this
        data.Size = width * height * channels;

        Texture2DSpecification spec;
        spec.width = width;
        spec.height = height;
        switch (channels) {
        case 3:
            spec.format = Texture2DChannelFormat::RGB8;
            spec.dataFormat = Texture2DDataFormat::RGB;
            break;
        case 4:
            spec.format = Texture2DChannelFormat::RGBA8;
            spec.dataFormat = Texture2DDataFormat::RGBA;
            break;
        }

        Ref<Texture2D> texture = CreateRef<Texture2D>(spec, data);
        data.Release();
        return texture;
    }

    Ref<Shader> ShaderImporter::ImportShader(AssetHandle handle, const AssetMetadata& metadata) {
        SHADO_PROFILE_FUNCTION();
        return LoadShader(Project::GetProjectDirectory() / metadata.FilePath);
    }

    Ref<Shader> ShaderImporter::LoadShader(const std::filesystem::path& path) {
        // Read file from filesystem
        std::string source;
        {
            std::ifstream file(path);
            if (!file.is_open()) {
                SHADO_CORE_ERROR("Could not open shader file: {}", path.string());
                return nullptr;
            }

            std::stringstream ss;
            ss << file.rdbuf();
            source = ss.str();
        }
        return CreateRef<Shader>(source);
    }
}
