#include "Font.h"

#undef INFINITE
#include <msdf-atlas-gen.h>

#include "Texture2D.h"
#include "debug/Debug.h"
#include "project/Project.h"

namespace Shado {
    template <typename T, typename S, int N, msdf_atlas::GeneratorFunction<S, N> GenFunc>
    static Ref<Texture2D> CreateAndCacheAtlas(const std::string& fontName, float fontSize,
                                              const std::vector<msdf_atlas::GlyphGeometry>& glyphs,
                                              const msdf_atlas::FontGeometry& fontGeometry, uint32_t width,
                                              uint32_t height) {
        msdf_atlas::GeneratorAttributes attributes;
        attributes.config.overlapSupport = true;
        attributes.scanlinePass = true;

        msdf_atlas::ImmediateAtlasGenerator<S, N, GenFunc, msdf_atlas::BitmapAtlasStorage<T, N>> generator(
            width, height);
        generator.setAttributes(attributes);
        generator.setThreadCount(8);
        generator.generate(glyphs.data(), (int)glyphs.size());

        msdfgen::BitmapConstRef<T, N> bitmap = (msdfgen::BitmapConstRef<T, N>)generator.atlasStorage();

        Texture2DSpecification spec;
        spec.width = bitmap.width;
        spec.height = bitmap.height;
        spec.format = Texture2DChannelFormat::RGB8;
        spec.dataFormat = Texture2DDataFormat::RGB;
        spec.generateMips = false;

        Ref<Texture2D> texture = CreateRef<Texture2D>(spec);
        texture->setData(Buffer((void*)bitmap.pixels, bitmap.width * bitmap.height * 3));
        return texture;
    }

    Font::Font(const std::filesystem::path& path)
        : m_Data(CreateScoped<MSDFData>()), m_Path(path) {
        using namespace msdfgen;

        FreetypeHandle* ft = initializeFreetype();
        if (!ft) {
            SHADO_CORE_ERROR("Failed to initialize freetype");
            return;
        }
        m_Data->m_FontTypeHandle = ft;

        // TODO: When we have an asset manager, we should load the font from memory buffer
        FontHandle* font = loadFont(ft, (Project::GetProjectDirectory() / path).string().c_str());
        if (!font) {
            SHADO_CORE_ERROR("Failed to load font from path: {0}", path.string());
            return;
        }
        m_Data->m_FontHandle = font;

        // From imgui_draw.cpp
        static constexpr uint32_t charsetRanges[] = {
            0x0020, 0x00FF, // Basic Latin + Latin-1 Supplement
            0,
        };
        msdf_atlas::Charset charset;
        for (std::size_t range = 0; range < std::size(charsetRanges); range += 2) {
            for (uint32_t i = charsetRanges[range]; i <= charsetRanges[range + 1]; i++) {
                charset.add(i);
            }
        }

        double fontScale = 1.0;
        m_Data->m_FontGeometry = snew(msdf_atlas::FontGeometry) msdf_atlas::FontGeometry(&m_Data->m_Glyphs);
        int glyphsLoaded = m_Data->m_FontGeometry->loadCharset(font, fontScale, charset);
        SHADO_CORE_INFO("Loaded {} glyphs (out of {})", glyphsLoaded, charset.size());

        double emSize = 40.0;
        msdf_atlas::TightAtlasPacker atlasPacker;
        atlasPacker.setPixelRange(2.0);
        atlasPacker.setMiterLimit(1.0);
        atlasPacker.setPadding(0);
        atlasPacker.setScale(emSize);
        int remaining = atlasPacker.pack(m_Data->m_Glyphs.data(), m_Data->m_Glyphs.size());
        if (remaining != 0) {
            SHADO_CORE_ERROR("Failed to pack {} glyphs (remaining should be 0)", remaining);
        }

        int width, height;
        atlasPacker.getDimensions(width, height);
        emSize = atlasPacker.getScale();

        constexpr float DEFAULT_ANGLE_THRESHOLD = 3.0;
        constexpr uint64_t LCG_MULTIPLIER = 6364136223846793005ull;
        constexpr uint64_t LCG_INCREMENT = 1442695040888963407ull;
        constexpr uint64_t THREAD_COUNT = 8;

        uint64_t coloringSeed = 0;
        bool expensiveColoring = false;
        if (expensiveColoring) {
            msdf_atlas::Workload([&glyphs = m_Data->m_Glyphs, &coloringSeed](int i, int threadNo) -> bool {
                uint64_t glyphSeed = (LCG_MULTIPLIER * (coloringSeed ^ i) + LCG_INCREMENT) * !!coloringSeed;
                glyphs[i].edgeColoring(msdfgen::edgeColoringInkTrap, DEFAULT_ANGLE_THRESHOLD, glyphSeed);
                return true;
            }, m_Data->m_Glyphs.size()).finish(THREAD_COUNT);
        }
        else {
            unsigned long long glyphSeed = coloringSeed;
            for (msdf_atlas::GlyphGeometry& glyph : m_Data->m_Glyphs) {
                glyphSeed *= LCG_MULTIPLIER;
                glyph.edgeColoring(msdfgen::edgeColoringInkTrap, DEFAULT_ANGLE_THRESHOLD, glyphSeed);
            }
        }

        m_AtlasTexture = CreateAndCacheAtlas<uint8_t, float, 3, msdf_atlas::msdfGenerator>(
            path.stem().string(), emSize, m_Data->m_Glyphs, *m_Data->m_FontGeometry, width, height);
    }

    Font::~Font() {
        destroyFont(m_Data->m_FontHandle);
        deinitializeFreetype(m_Data->m_FontTypeHandle);

        delete m_Data->m_FontGeometry;
    }
}
