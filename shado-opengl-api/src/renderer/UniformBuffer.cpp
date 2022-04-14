#include "UniformBuffer.h"

#include "GL/glew.h"

namespace Shado {

	UniformBuffer::UniformBuffer(uint32_t size, uint32_t binding) {
		glCreateBuffers(1, &m_RendererID);
		glNamedBufferData(m_RendererID, size, nullptr, GL_DYNAMIC_DRAW); // TODO: investigate usage hint
		glBindBufferBase(GL_UNIFORM_BUFFER, binding, m_RendererID);
	}

	UniformBuffer::~UniformBuffer() {
		glDeleteBuffers(1, &m_RendererID);
	}

	void UniformBuffer::setData(const void* data, uint32_t size, uint32_t offset) {
		glNamedBufferSubData(m_RendererID, offset, size, data);
	}

	Ref<Shado::UniformBuffer> UniformBuffer::create(uint32_t size, uint32_t binding) {
		return CreateRef<UniformBuffer>(size, binding);
	}

}
