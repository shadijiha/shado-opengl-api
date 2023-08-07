#pragma once

#include "Buffer.h"

namespace Shado {
	class VertexArray : public RefCounted {
	public:
		VertexArray();
		~VertexArray();

		virtual void bind() const;
		virtual void unBind() const;

		virtual void addVertexBuffer(const Ref<VertexBuffer>& vertexBuffer);
		virtual void setIndexBuffer(const Ref<IndexBuffer>& vertexBuffer);

		virtual const std::vector<Ref<VertexBuffer>>& getVertexBuffers() const { return m_VertexBuffers; };
		virtual const Ref<IndexBuffer>& getIndexBuffers() const { return m_IndexBuffer; };

		static Ref<VertexArray> create();

	private:
		uint32_t m_RendererID;
		uint32_t m_VertexBufferIndex = 0;
		std::vector<Ref<VertexBuffer>> m_VertexBuffers;
		Ref<IndexBuffer> m_IndexBuffer;
	};
}
