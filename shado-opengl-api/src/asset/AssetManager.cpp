#include "AssetManager.h"

#include <fstream>
#include <yaml-cpp/yaml.h>

#include "Importer.h"
#include "debug/Profile.h"

namespace Shado {
    static AssetType FilepathToAssetType(const std::filesystem::path& filepath) {
        auto extension = filepath.extension();
        if (extension == ".scene")
            return AssetType::Scene;
        if (extension == ".png" || extension == ".jpg")
            return AssetType::Texture2D;
        if (extension == ".shader" || extension == ".glsl")
            return AssetType::Shader;
        return AssetType::None;
    }

    /**
     * @param fullpath File path including project dir
     * @return 
     */
    static uint64_t GetFileLastModified(const std::filesystem::path& fullpath) {
        SHADO_CORE_ASSERT(std::filesystem::exists(fullpath), "File does not exist {}", fullpath.string());
        
        auto ftime = std::filesystem::last_write_time(fullpath);
        auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
            ftime - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now()
        );

        return static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
            sctp.time_since_epoch()
        ).count());
    }

    YAML::Emitter& operator<<(YAML::Emitter& out, const std::string_view& v) {
        out << std::string(v.data(), v.size());
        return out;
    }

    bool EditorAssetManager::IsAssetHandleValid(AssetHandle handle) const {
        return handle != 0 && m_AssetRegistry.find(handle) != m_AssetRegistry.end();
    }

    AssetHandle EditorAssetManager::ImportAsset(const std::filesystem::path& filepath, bool serializeToRegistry) {
        AssetHandle handle; // generate new handle
        AssetMetadata metadata;
        metadata.FilePath = filepath;
        metadata.Type = FilepathToAssetType(filepath);
        metadata.DateModified = GetFileLastModified(Project::GetProjectDirectory() / filepath);
        Ref<Asset> asset = AssetImporter::ImportAsset(handle, metadata);
        asset->Handle = handle;
        if (asset) {
            m_LoadedAssets[handle] = asset;
            m_AssetRegistry[handle] = metadata;

            if (serializeToRegistry)
                SerializeAssetRegistry();

            SHADO_CORE_TRACE("Imported {} => {} and {} serialized registry",
                filepath.string(),
                asset->Handle,
                serializeToRegistry ? "" : "DID NOT");
        }

        return handle;
    }

    AssetHandle EditorAssetManager::GetHandleFromPath(const std::filesystem::path& filepath) {
        return std::ranges::find_if(m_AssetRegistry,
                                    [&filepath](const auto& pair) { return pair.second.FilePath == filepath; })->first;
    }

    std::filesystem::path EditorAssetManager::GetPathFromHandle(AssetHandle handle) {
        return m_AssetRegistry.at(handle).FilePath;
    }

    bool EditorAssetManager::IsAssetLoaded(AssetHandle handle) const {
        return m_LoadedAssets.contains(handle);
    }

    bool EditorAssetManager::IsPathInRegistry(const std::filesystem::path& path) const {
        SHADO_PROFILE_FUNCTION();

        auto assetIt = std::ranges::find_if(m_AssetRegistry,
                                    [&path](const auto& pair) {
                                        return pair.second.FilePath == path;
                                    });
        if (assetIt == m_AssetRegistry.end())
            return false;

        uint64_t accualDateModified = GetFileLastModified(path.is_absolute() ? path :  Project::GetProjectDirectory() / path);
        bool result = assetIt->second.DateModified == accualDateModified;
        if (!result) {
            SHADO_CORE_WARN("Triggering reload of {} because Last modified dates do not match (registered: {}, actual: {})", path, assetIt->second.DateModified, accualDateModified);
        }
        return result;
    }

    const AssetMetadata& EditorAssetManager::GetMetadata(AssetHandle handle) const {
        static AssetMetadata s_NullMetadata;
        auto it = m_AssetRegistry.find(handle);
        if (it == m_AssetRegistry.end())
            return s_NullMetadata;
        return it->second;
    }

    Ref<Asset> EditorAssetManager::GetAsset(AssetHandle handle) {
        // 1. check if handle is valid
        if (!IsAssetHandleValid(handle)) {
            SHADO_CORE_ASSERT(false, "Invalid AssetHandle {}", handle);
            return nullptr;
        }
            
        // 2. check if asset needs load (and if so, load)
        Ref<Asset> asset;
        if (IsAssetLoaded(handle)) {
            asset = m_LoadedAssets.at(handle);
        }
        else {
            // load asset
            const AssetMetadata& metadata = GetMetadata(handle);
            asset = AssetImporter::ImportAsset(handle, metadata);
            if (!asset) {
                // import failed
                SHADO_CORE_ERROR("EditorAssetManager::GetAsset - asset import failed for {}!", handle);
            }
            else {
                m_LoadedAssets[handle] = asset;
                m_AssetRegistry[handle] = metadata;
            }
        }
        // 3. return asset
        return asset;
    }

    void EditorAssetManager::SerializeAssetRegistry() {
        SHADO_PROFILE_FUNCTION();
        
        auto path = Project::GetAssetRegistryPath();
        YAML::Emitter out;

        {
            out << YAML::BeginMap; // Root

            // Write metadata
            out << YAML::Key << "Version" << YAML::Value << SHADO_ASSET_MANAGER_VERSION;
            out << YAML::Key << "CreationDate" << YAML::Value << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

            // Write assets data
            out << YAML::Key << "AssetRegistry" << YAML::Value;
            out << YAML::BeginSeq;
            for (const auto& [handle, metadata] : m_AssetRegistry) {
                out << YAML::BeginMap;
                out << YAML::Key << "Handle" << YAML::Value << handle;

                auto filepathStr = metadata.FilePath.is_absolute()
                                       ? Project::GetActive()->GetRelativePath(metadata.FilePath)
                                       : metadata.FilePath;
                out << YAML::Key << "FilePath" << YAML::Value << filepathStr.generic_string();
                out << YAML::Key << "Type" << YAML::Value << AssetTypeToString(metadata.Type);
                out << YAML::Key << "DateModified" << YAML::Value << metadata.DateModified;
                out << YAML::EndMap;
            }
            out << YAML::EndSeq;
            out << YAML::EndMap; // Root
        }
        std::ofstream fout(path);
        fout << out.c_str();
    }

    bool EditorAssetManager::DeserializeAssetRegistry() {
        SHADO_PROFILE_FUNCTION();
        
        auto path = Project::GetAssetRegistryPath();
        if (!std::filesystem::exists(path)) {
            SHADO_CORE_WARN("Asset registry file does not exist: {0}", path);
            return false;
        }

        YAML::Node data;
        try {
            data = YAML::LoadFile(path.string());
        }
        catch (const YAML::BadFile& e) {
            SHADO_CORE_WARN("Failed to load project file '{0}'\n     {1}", path, e.what());
            return false;
        }
        catch (const YAML::ParserException& e) {
            SHADO_CORE_ERROR("Failed to load project file '{0}'\n     {1}", path, e.what());
            return false;
        }

        auto version = data["Version"].as<std::string>();
        SHADO_CORE_ASSERT(version == SHADO_ASSET_MANAGER_VERSION, "Unexpected Asset registry file version {}", version);
        
        auto rootNode = data["AssetRegistry"];
        if (!rootNode)
            return false;
        for (const auto& node : rootNode) {
            AssetHandle handle = node["Handle"].as<uint64_t>();
            auto& metadata = m_AssetRegistry[handle];
            metadata.FilePath = Project::GetActive()->GetProjectDirectory() / node["FilePath"].as<std::string>();
            metadata.Type = AssetTypeFromString(node["Type"].as<std::string>());

            if (node["DateModified"]) {
                metadata.DateModified = node["DateModified"].as<uint64_t>();
            }
        }
        return true;
    }
}
