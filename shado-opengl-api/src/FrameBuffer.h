#pragma once
#include <glm/vec2.hpp>

#include "util/Util.h"

namespace Shado {

	struct FrameBufferSpecification {
		uint32_t width, height;
		//FrameBufferFormat format;
		uint32_t samples = 1;
		bool swapChainTarget = false;
	};

	class FrameBuffer {
	public:
		FrameBuffer(const FrameBufferSpecification& spec);
		virtual ~FrameBuffer();

		void resize(uint32_t width, uint32_t height);
		void invalidate();
		void bind();
		void unbind();

		const FrameBufferSpecification& getSpecification() const { return m_Specification; }
		uint32_t getColorAttachmentRendererID() const { return  m_ColorAttachment; }

		static Ref<FrameBuffer> create(const FrameBufferSpecification& spec);

	private:
		uint32_t m_RendererID = 0;
		uint32_t m_ColorAttachment = 0;
		uint32_t m_DepthAttachment = 0;
		FrameBufferSpecification m_Specification;
	};
}
