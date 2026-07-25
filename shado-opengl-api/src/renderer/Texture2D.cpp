#include "Texture2D.h"
#include "debug/Profile.h"
#include "GL/glew.h"
#include <GLFW/glfw3.h>

// Windows/Linux use OpenGL 4.5 Direct State Access
// (glCreateTextures/glTextureStorage2D/glTextureParameteri/glBindTextureUnit).
// Apple caps OpenGL at 4.1, so on macOS we use the classic bind-then-modify
// path behind SHADO_PLATFORM_MACOS guards. The Windows/Linux path is unchanged.

namespace Shado {
    Texture2D::Texture2D(uint32_t width, uint32_t height)
        : m_Width(width), m_Height(height) {
        SHADO_PROFILE_FUNCTION();

        m_InternalFormat = GL_RGBA8;
        m_DataFormat = GL_RGBA;

#if defined(SHADO_PLATFORM_MACOS)
        glGenTextures(1, &m_RendererID);
        glBindTexture(GL_TEXTURE_2D, m_RendererID);
        glTexImage2D(GL_TEXTURE_2D, 0, m_InternalFormat, m_Width, m_Height, 0,
                     m_DataFormat, GL_UNSIGNED_BYTE, nullptr);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
#else
        glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
        glTextureStorage2D(m_RendererID, 1, m_InternalFormat, m_Width, m_Height);

        glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);
#endif
    }

    static std::mutex s_mutex;

    Texture2D::Texture2D(Texture2DSpecification specs, Buffer data) {
        SHADO_PROFILE_FUNCTION();

        // For some reason texture creation is crashing on std::async so make this thread safe
        std::lock_guard<std::mutex> lock(s_mutex);

        m_InternalFormat = (uint32_t)specs.format;
        m_DataFormat = (uint32_t)specs.dataFormat;
        m_Width = specs.width;
        m_Height = specs.height;

#if defined(SHADO_PLATFORM_MACOS)
        glGenTextures(1, &m_RendererID);
        SHADO_CORE_ASSERT(m_RendererID > 0 && m_RendererID < UINT32_MAX, "RendererId doesn't seem right!");
        glBindTexture(GL_TEXTURE_2D, m_RendererID);
        SHADO_CORE_ASSERT(glIsTexture(m_RendererID) == GL_TRUE, "{} is NOT a texture!", m_RendererID);

        glTexImage2D(GL_TEXTURE_2D, 0, m_InternalFormat, m_Width, m_Height, 0,
                     m_DataFormat, GL_UNSIGNED_BYTE, nullptr);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
#else
        glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
        SHADO_CORE_ASSERT(m_RendererID > 0 && m_RendererID < UINT32_MAX, "RendererId doesn't seem right!");
        SHADO_CORE_ASSERT(glIsTexture(m_RendererID) == GL_TRUE, "{} is NOT a texture!", m_RendererID);

        glTextureStorage2D(m_RendererID, 1, m_InternalFormat, m_Width, m_Height);

        glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);
#endif

        if (data) {
            setData(data);
        }
    }

    Texture2D::~Texture2D() {
        glDeleteTextures(1, &m_RendererID);
    }

    void Texture2D::setData(Buffer data) {
        SHADO_PROFILE_FUNCTION();

        uint32_t bpp = m_DataFormat == GL_RGBA ? 4 : 3;
        SHADO_CORE_ASSERT(data.Size == m_Width * m_Height * bpp, "Data must be entire texture!");
#if defined(SHADO_PLATFORM_MACOS)
        glBindTexture(GL_TEXTURE_2D, m_RendererID);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, data.Data);
#else
        glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, data.Data);
#endif
    }

    void Texture2D::bind(uint32_t slot) const {
#if defined(SHADO_PLATFORM_MACOS)
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D, m_RendererID);
#else
        glBindTextureUnit(slot, m_RendererID);
#endif
    }

    void Texture2D::unbind() const {
        //glCall(glBindTexture(GL_TEXTURE_2D, 0));
    }

    bool Texture2D::operator==(const Texture2D& other) const {
        return m_RendererID == ((Texture2D&)other).m_RendererID;
    }
}
