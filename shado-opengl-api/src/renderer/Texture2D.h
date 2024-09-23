#pragma once
#include <string>
#include <unordered_map>
#include <filesystem>

#include "asset/Asset.h"
#include "util/Buffer.h"
#include "util/Memory.h"


namespace Shado {
    enum class Texture2DChannelFormat : uint32_t {
        RGBA8 = 0x8058,
        RGB8 = 0x8051,
        RGBA16 = 0x805B
    };

    enum class Texture2DDataFormat : uint32_t {
        RGB = 0x1907,
        RGBA = 0x1908
    };

    struct Texture2DSpecification {
        uint32_t width = 1, height = 1;
        Texture2DChannelFormat format = Texture2DChannelFormat::RGBA8;
        Texture2DDataFormat dataFormat = Texture2DDataFormat::RGBA;
        bool generateMips = true;
    };

    class Texture2D : public Asset {
    public:
        /**
         * Use the create function instead of Texture2D for caching
         */
        Texture2D(uint32_t width, uint32_t height);
        Texture2D(Texture2DSpecification specs, Buffer data = Buffer());
        ~Texture2D();

        void setData(Buffer data);

        void bind(uint32_t slot = 0) const;
        void unbind() const;

        bool isLoaded() const { return m_IsLoaded; }

        int getWidth() const { return m_Width; }
        int getHeight() const { return m_Height; }
        uint32_t getRendererID() const { return m_RendererID; }
        int getDataFormat() const { return m_DataFormat; }
        int getInternalFormat() const { return m_InternalFormat; }

        bool operator==(const Texture2D& other) const;

        static AssetType GetStaticType() { return AssetType::Texture2D; }
        AssetType GetType() const override { return GetStaticType(); }

    private:
        uint32_t m_RendererID;
        uint32_t m_Width, m_Height;

        unsigned int m_InternalFormat;
        unsigned int m_DataFormat;

        bool m_IsLoaded = false;

        // Cache texture
        struct TextureInfo {
            Ref<Texture2D> texture;
            uint32_t refCount = 0;
        };

        inline static std::unordered_map<std::string, TextureInfo> s_cache;
    };
}
