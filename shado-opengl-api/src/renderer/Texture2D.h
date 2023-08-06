#pragma once
#include <string>
#include <unordered_map>

#include "util/Util.h"

namespace Shado {
	
	class Texture2D {
	public:
		/**
		 * Use the create function instead of Texture2D for caching
		 */
		Texture2D(uint32_t width, uint32_t height);
		Texture2D(const std::string& path);
		~Texture2D();

		static Ref<Texture2D> create(const std::string& path);
		static inline Ref<Texture2D> create(const std::filesystem::path& path) {
			return create(path.string());
		}

		void setData(void* data, uint32_t size);

		void bind(uint32_t slot = 0) const;
		void unbind() const;

		bool isLoaded() const { return m_IsLoaded; }

		int getWidth() const { return m_Width; }
		int getHeight() const { return m_Height; }
		uint32_t getRendererID() const { return m_RendererID; }
		std::string getFilePath() const { return m_FilePath; }
		int getDataFormat() const { return m_DataFormat; }
		int getInternalFormat() const { return m_InternalFormat; }

		bool operator==(const Texture2D& other) const
		{
			return m_RendererID == ((Texture2D&)other).m_RendererID;
		}

	private:
		uint32_t m_RendererID;
		uint32_t m_Width, m_Height;

		unsigned int m_InternalFormat;
		unsigned int m_DataFormat;

		std::string m_FilePath;
		bool m_IsLoaded = false;

		// Cache texture
		struct TextureInfo {
			Ref<Texture2D> texture;
			uint32_t refCount = 0;
		};

		inline static std::unordered_map<std::string, TextureInfo> s_cache;
	};

}
