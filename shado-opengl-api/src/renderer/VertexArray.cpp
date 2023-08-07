#include "VertexArray.h"
#include <memory>
#include "Buffer.h"
#include "Renderer2D.h"
#include <GL/glew.h>

namespace Shado {
	Ref<VertexArray> VertexArray::create() {
		return CreateRef<VertexArray>();
	}

	static GLenum toOpenGLType(ShaderDataType type) {
		switch (type) {
		case ShaderDataType::Float:		return GL_FLOAT;
		case ShaderDataType::Float2:	return GL_FLOAT;
		case ShaderDataType::Float3:	return GL_FLOAT;
		case ShaderDataType::Float4:	return GL_FLOAT;
		case ShaderDataType::Mat3:		return GL_FLOAT;
		case ShaderDataType::Mat4:		return GL_FLOAT;
		case ShaderDataType::Int:		return GL_INT;
		case ShaderDataType::Int2:		return GL_INT;
		case ShaderDataType::Int3:		return GL_INT;
		case ShaderDataType::Int4:		return GL_INT;
		case ShaderDataType::Bool:		return GL_BOOL;
		}
		SHADO_CORE_ASSERT(false, "Unknown ShaderDataType");
		return 0;
	}

	VertexArray::VertexArray() {
		glCreateVertexArrays(1, &m_RendererID);
	}

	VertexArray::~VertexArray() {
		glDeleteVertexArrays(1, &m_RendererID);
	}

	void VertexArray::bind() const {
		glBindVertexArray(m_RendererID);
		//m_IndexBuffers->bind();
	}

	void VertexArray::unBind() const {
		glBindVertexArray(0);
	}

	void VertexArray::addVertexBuffer(const Ref<VertexBuffer>& vertexBuffer) {

		SHADO_CORE_ASSERT(vertexBuffer->getLayout().getElements().size(), "Vertex buffer has no layout!");

		glBindVertexArray(m_RendererID);
		vertexBuffer->bind();

		const auto& layout = vertexBuffer->getLayout();
		for (const auto& element : layout)
		{
			switch (element.Type)
			{
			case ShaderDataType::Float:
			case ShaderDataType::Float2:
			case ShaderDataType::Float3:
			case ShaderDataType::Float4:
			{
				glEnableVertexAttribArray(m_VertexBufferIndex);
				glVertexAttribPointer(m_VertexBufferIndex,
					element.getComponentCount(),
					toOpenGLType(element.Type),
					element.Normalized ? GL_TRUE : GL_FALSE,
					layout.getStride(),
					(const void*)element.Offset);
				m_VertexBufferIndex++;
				break;
			}
			case ShaderDataType::Int:
			case ShaderDataType::Int2:
			case ShaderDataType::Int3:
			case ShaderDataType::Int4:
			case ShaderDataType::Bool: {
				glEnableVertexAttribArray(m_VertexBufferIndex);
				glVertexAttribIPointer(m_VertexBufferIndex,
					element.getComponentCount(),
					toOpenGLType(element.Type),
					layout.getStride(),
					(const void*)element.Offset);
				m_VertexBufferIndex++;
				break;
			}
			case ShaderDataType::Mat3:
			case ShaderDataType::Mat4:
			{
				uint8_t count = element.getComponentCount();
				for (uint8_t i = 0; i < count; i++)
				{
					glEnableVertexAttribArray(m_VertexBufferIndex);
					glVertexAttribPointer(m_VertexBufferIndex,
						count,
						toOpenGLType(element.Type),
						element.Normalized ? GL_TRUE : GL_FALSE,
						layout.getStride(),
						(const void*)(element.Offset + sizeof(float) * count * i));
					glVertexAttribDivisor(m_VertexBufferIndex, 1);
					m_VertexBufferIndex++;
				}
				break;
			}
			default:
				SHADO_CORE_ASSERT(false, "Unknown ShaderDataType!");
			}
		}

		m_VertexBuffers.push_back(vertexBuffer);
	}

	void VertexArray::setIndexBuffer(const Ref<IndexBuffer>& indexBuffer) {
		glBindVertexArray(m_RendererID);
		indexBuffer->bind();

		m_IndexBuffer = indexBuffer;
	}
}