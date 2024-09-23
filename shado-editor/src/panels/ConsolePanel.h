#pragma once
#include "asset/Importer.h"
#include "renderer/Texture2D.h"

namespace Shado {
    class ConsolePanel {
    public:
        ConsolePanel();

        void onImGuiRender();

    private:
        Ref<Texture2D> detectMessageType(const std::string& message) const;

    private:
        Ref<Texture2D> m_ErrorIcon = TextureImporter::LoadTexture2D("resources/icons/error.png");
        Ref<Texture2D> m_WarnIcon = TextureImporter::LoadTexture2D("resources/icons/warn.png");
        Ref<Texture2D> m_InfoIcon = TextureImporter::LoadTexture2D("resources/icons/info.png");
    };
}
