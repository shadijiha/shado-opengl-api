#include "Texture2D.h"
#include "debug/Profile.h"
#include "GL/glew.h"
#include <GLFW/glfw3.h>

namespace Shado {
    Texture2D::Texture2D(uint32_t width, uint32_t height)
        : m_Width(width), m_Height(height) {
        SHADO_PROFILE_FUNCTION();

        m_InternalFormat = GL_RGBA8;
        m_DataFormat = GL_RGBA;

        glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
        glTextureStorage2D(m_RendererID, 1, m_InternalFormat, m_Width, m_Height);

        glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }

    static std::mutex s_mutex;

    Texture2D::Texture2D(Texture2DSpecification specs, Buffer data) {
        SHADO_PROFILE_FUNCTION();

        // For some reason glCreateTextures is crashing on std::async so make this thread safe
        std::lock_guard<std::mutex> lock(s_mutex);

        m_InternalFormat = (uint32_t)specs.format;
        m_DataFormat = (uint32_t)specs.dataFormat;
        m_Width = specs.width;
        m_Height = specs.height;

        glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
        SHADO_CORE_ASSERT(m_RendererID > 0 && m_RendererID < UINT32_MAX, "RendererId doesn't seem right!");
        SHADO_CORE_ASSERT(glIsTexture(m_RendererID) == GL_TRUE, "{} is NOT a texture!", m_RendererID);
        
        glTextureStorage2D(m_RendererID, 1, m_InternalFormat, m_Width, m_Height);
        
        glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);

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
        glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, data.Data);
    }

    void Texture2D::bind(uint32_t slot) const {
        glBindTextureUnit(slot, m_RendererID);
    }

    void Texture2D::unbind() const {
        //glCall(glBindTexture(GL_TEXTURE_2D, 0));
    }

    bool Texture2D::operator==(const Texture2D& other) const {
        return m_RendererID == ((Texture2D&)other).m_RendererID;
    }
}
