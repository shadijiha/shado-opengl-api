#pragma once

#include "Buffer.h"

namespace Shado {
	class VertexArray {
	public:
		VertexArray();
		~VertexArray();

		virtual void bind() const;
		virtual void unBind() const;

		virtual void addVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer);
		virtual void setIndexBuffer(const std::shared_ptr<IndexBuffer>& vertexBuffer);

		virtual const std::vector<std::shared_ptr<VertexBuffer>>& getVertexBuffers() const { return m_VertexBuffers; };
		virtual const std::shared_ptr<IndexBuffer>& getIndexBuffers() const { return m_IndexBuffer; };

		static std::shared_ptr<VertexArray> create();

	private:
		uint32_t m_RendererID;
		uint32_t m_VertexBufferIndex = 0;
		std::vector<std::shared_ptr<VertexBuffer>> m_VertexBuffers;
		std::shared_ptr<IndexBuffer> m_IndexBuffer;
	};
}
