#include "UniformBuffer.h"

#include "GL/glew.h"

namespace Shado {

	UniformBuffer::UniformBuffer(uint32_t size, uint32_t binding) {
#if defined(SHADO_PLATFORM_MACOS)
		// macOS OpenGL 4.1 has no DSA; use the classic bind-based path.
		glGenBuffers(1, &m_RendererID);
		glBindBuffer(GL_UNIFORM_BUFFER, m_RendererID);
		glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
#else
		glCreateBuffers(1, &m_RendererID);
		glNamedBufferData(m_RendererID, size, nullptr, GL_DYNAMIC_DRAW); // TODO: investigate usage hint
#endif
		glBindBufferBase(GL_UNIFORM_BUFFER, binding, m_RendererID);
	}

	UniformBuffer::~UniformBuffer() {
		glDeleteBuffers(1, &m_RendererID);
	}

	void UniformBuffer::setData(const void* data, uint32_t size, uint32_t offset) {
#if defined(SHADO_PLATFORM_MACOS)
		glBindBuffer(GL_UNIFORM_BUFFER, m_RendererID);
		glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data);
#else
		glNamedBufferSubData(m_RendererID, offset, size, data);
#endif
	}

	Ref<Shado::UniformBuffer> UniformBuffer::create(uint32_t size, uint32_t binding) {
		return CreateRef<UniformBuffer>(size, binding);
	}

}
