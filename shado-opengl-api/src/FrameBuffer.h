#pragma once
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

		void invalidate();
		void bind();
		void unbind();

		const FrameBufferSpecification& getSpecification() const { return m_Specification; }
		uint32_t getColorAttachmentRendererID() const { return  m_ColorAttachment; }

		static Ref<FrameBuffer> create(const FrameBufferSpecification& spec);

	private:
		uint32_t m_RendererID;
		uint32_t m_ColorAttachment;
		uint32_t m_DepthAttachment;
		FrameBufferSpecification m_Specification;
	};
}
