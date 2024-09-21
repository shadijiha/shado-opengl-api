#pragma once
#include <filesystem>
#include <optional>

#include "renderer/Texture2D.h"
#include "util/TimeStep.h"
#include <unordered_map>

namespace Shado {
    class Framebuffer;
}

namespace Shado {
    class ContentBrowserPanel {
    public:
        ContentBrowserPanel();
        ContentBrowserPanel(std::optional<std::filesystem::path> path);

        void onImGuiRender();

    private:
        struct ThumbnailMetadata {
            uint32_t rendererId;
            float aspectRatio;
        };

        void setDirectory(const std::filesystem::path& path);
        ThumbnailMetadata getThumbnail(const std::filesystem::directory_entry&);
        Ref<Framebuffer> generateSceneThumbnail(const std::filesystem::path& path, uint32_t width);

    private:
        float m_ThumbnailSize = 100.0f;
        std::optional<std::filesystem::path> m_CurrentDirectory;

        Ref<Texture2D> m_DirectoryIcon;
        Ref<Texture2D> m_FileIcon;
        Ref<Texture2D> m_SceneIcon;
        Ref<Texture2D> m_PrefabIcon;
        Ref<Texture2D> m_CSIcon;
        Ref<Texture2D> m_SlnIcon;
        std::unordered_map<std::string, Ref<Framebuffer>> m_ScenesThumbnails;

        std::vector<std::filesystem::directory_entry> directories;
        uint32_t tick = 1;

        std::unordered_map<std::string, Ref<Texture2D>> imagesThumbnails;
    };
}
