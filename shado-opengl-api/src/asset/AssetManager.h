#pragma once
#include <map>

#include "Asset.h"
#include "project/Project.h"
#include "util/Memory.h"

#define SHADO_ASSET_MANAGER_VERSION "0.0.1"

namespace Shado {
    using AssetMap = std::map<AssetHandle, Ref<Asset>>;

    class AssetManagerBase : public RefCounted {
    public:
        virtual ~AssetManagerBase() = default;
        virtual Ref<Asset> GetAsset(AssetHandle handle) = 0;
        virtual AssetHandle GetHandleFromPath(const std::filesystem::path& filepath) = 0;
        virtual std::filesystem::path GetPathFromHandle(AssetHandle handle) = 0;

        virtual bool IsAssetHandleValid(AssetHandle handle) const = 0;
        virtual bool IsAssetLoaded(AssetHandle handle) const = 0;
        virtual bool IsPathInRegistry(const std::filesystem::path& path) const = 0;
    };

    /**
     *
     * 
     */
    class AssetManager {
    public:
        template <typename T>
        static Ref<T> GetAsset(AssetHandle handle) {
            Ref<Asset> asset = Project::GetActive()->GetAssetManager()->GetAsset(handle);
            return asset;
        }

        static AssetHandle GetHandleFromPath(const std::filesystem::path& filepath) {
            auto handle = Project::GetActive()->GetAssetManager()->GetHandleFromPath(filepath);
            return handle;
        }

        static std::filesystem::path GetPathFromHandle(AssetHandle handle) {
            return Project::GetActive()->GetAssetManager()->GetPathFromHandle(handle);
        }
    };


    /**
     *
     * 
     */
    using AssetRegistry = std::map<AssetHandle, AssetMetadata>;

    class EditorAssetManager : public AssetManagerBase {
    public:
        virtual Ref<Asset> GetAsset(AssetHandle handle) override;
        AssetHandle GetHandleFromPath(const std::filesystem::path& filepath) override;
        std::filesystem::path GetPathFromHandle(AssetHandle handle) override;

        virtual bool IsAssetHandleValid(AssetHandle handle) const override;
        virtual bool IsAssetLoaded(AssetHandle handle) const override;

        /**
         * Checks if provided filepath is in the asset registry AND if its date modified matches the registry
         * @param path 
         * @return 
         */
        virtual bool IsPathInRegistry(const std::filesystem::path& path) const override;

        AssetHandle ImportAsset(const std::filesystem::path& filepath, bool serializeToRegistry = true);


        void SerializeAssetRegistry();
        bool DeserializeAssetRegistry();

        const AssetRegistry& GetAssetRegistry() const { return m_AssetRegistry; }
        const AssetMetadata& GetMetadata(AssetHandle handle) const;

    private:
        AssetRegistry m_AssetRegistry;
        AssetMap m_LoadedAssets;
        // TODO: memory-only assets
    };

    class RuntimeAssetManager : public AssetManagerBase {
    public:
    };
}
