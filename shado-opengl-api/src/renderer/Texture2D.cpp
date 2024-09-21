#include "Texture2D.h"
#include "debug/Profile.h"
#include "stb_image.h"
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

    Texture2D::Texture2D(const std::string& path)
        : m_RendererID(0), m_Width(0), m_Height(0), m_FilePath(path) {
        // If texture is in cache,
        // Then just copy it
        //if (s_cache.find(m_FilePath) != s_cache.end()) {
        //	auto& data = s_cache[path];
        //	data.refCount++;
        //	m_RendererID = data.texture->m_RendererID;
        //	m_Width = data.texture->m_Width;
        //	m_Height = data.texture->m_Height;
        //	m_DataFormat = data.texture->m_DataFormat;
        //	m_InternalFormat = data.texture->m_InternalFormat;
        //	m_IsLoaded = true;
        //	return;
        //}

        SHADO_PROFILE_FUNCTION();

        int width, height, channels;
        stbi_set_flip_vertically_on_load(1);

        stbi_uc* data = nullptr;
        {
            data = stbi_load(path.c_str(), &width, &height, &channels, 0);
        }

        if (data) {
            m_IsLoaded = true;
            m_Width = width;
            m_Height = height;

            GLenum internalFormat = 0, dataFormat = 0;
            if (channels == 4) {
                internalFormat = GL_RGBA8;
                dataFormat = GL_RGBA;
            }
            else if (channels == 3) {
                internalFormat = GL_RGB8;
                dataFormat = GL_RGB;
            }

            m_InternalFormat = internalFormat;
            m_DataFormat = dataFormat;

            SHADO_CORE_ASSERT(internalFormat & dataFormat, "Format not supported!");

            glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
            glTextureStorage2D(m_RendererID, 1, internalFormat, m_Width, m_Height);

            glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);

            // IF the width, height are not divisible by 4
            // Then use glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            if (m_Width % 4 != 0 || m_Height % 4 != 0)
                glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            else
                glPixelStorei(GL_UNPACK_ALIGNMENT, 4); // TODO: IF crash remove this line

            glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, dataFormat, GL_UNSIGNED_BYTE, data);

            stbi_image_free(data);
        }
    }

    static std::mutex s_mutex;

    Texture2D::Texture2D(Texture2DSpecification specs) {
        SHADO_PROFILE_FUNCTION();

        // For some reason glCreateTextures is crashing on std::async so make this thread safe
        std::lock_guard<std::mutex> lock(s_mutex);

        m_InternalFormat = (uint32_t)specs.format;
        m_DataFormat = (uint32_t)specs.dataFormat;
        m_Width = specs.width;
        m_Height = specs.height;

        glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
        glTextureStorage2D(m_RendererID, 1, m_InternalFormat, m_Width, m_Height);

        glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }

    Texture2D::~Texture2D() {
        // Check if texture is referenced
        //if (s_cache.find(m_FilePath) != s_cache.end()) {
        //	auto& data = s_cache[m_FilePath];

        //	// If the texture is no longer referenced
        //	// Then deleted from GL and from cache
        //	if (data.refCount - 1 <= 0) {
        //		glDeleteTextures(1, &m_RendererID);
        //		s_cache.erase(m_FilePath);
        //	} else {
        //		// Otherwise just decrement the ref count
        //		data.refCount -= 1;
        //		return;
        //	}
        //}
        //else {
        glDeleteTextures(1, &m_RendererID);
        //}		
    }

    Texture2D* Texture2D::create(const std::string& path) {
        //if (cache.find(path) != cache.end())
        //	return cache[path].texture;

        // Add to cache
        Texture2D* text = new Texture2D(path);
        //cache[path] = { text, 1 };

        return text;
    }

    void Texture2D::setData(void* data, uint32_t size) {
        uint32_t bpp = m_DataFormat == GL_RGBA ? 4 : 3;
        SHADO_CORE_ASSERT(size == m_Width * m_Height * bpp, "Data must be entire texture!");
        glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, data);
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
