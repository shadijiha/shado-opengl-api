#pragma once
#include <filesystem>

#include "Texture2D.h"
#include "util/Memory.h"

namespace msdfgen {
    class FreetypeHandle;
    class FontHandle;
}

namespace msdf_atlas {
    class FontGeometry;
    class GlyphGeometry;
}

namespace Shado {
    struct MSDFData {
        msdfgen::FreetypeHandle* m_FontTypeHandle;
        msdfgen::FontHandle* m_FontHandle;
        msdf_atlas::FontGeometry* m_FontGeometry;
        std::vector<msdf_atlas::GlyphGeometry> m_Glyphs;
    };

    class Font : public RefCounted {
    public:
        Font(const std::filesystem::path& path);
        ~Font();

        Ref<Texture2D> getAtlasTexture() const { return m_AtlasTexture; }
        const MSDFData& getData() const { return *m_Data; }
        const std::filesystem::path& getPath() const { return m_Path; }

    private:
        ScopedRef<MSDFData> m_Data;
        Ref<Texture2D> m_AtlasTexture;
        std::filesystem::path m_Path;
    };
}
