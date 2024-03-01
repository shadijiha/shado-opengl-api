#pragma once
#include <glm/vec2.hpp>

#include "debug/Debug.h"
#include "util/Util.h"

namespace Shado {

	enum class FramebufferTextureFormat
	{
		None = 0,

		// Color
		RGBA8,
		RED_INTEGER,

		// Depth/stencil
		DEPTH24STENCIL8,

		// Defaults
		Depth = DEPTH24STENCIL8
	};

	struct FramebufferTextureSpecification
	{
		FramebufferTextureSpecification() = default;
		FramebufferTextureSpecification(FramebufferTextureFormat format)
			: TextureFormat(format) {}

		FramebufferTextureFormat TextureFormat = FramebufferTextureFormat::None;
		// TODO: filtering/wrap
	};

	struct FramebufferAttachmentSpecification
	{
		FramebufferAttachmentSpecification() = default;
		FramebufferAttachmentSpecification(std::initializer_list<FramebufferTextureSpecification> attachments)
			: Attachments(attachments) {}

		std::vector<FramebufferTextureSpecification> Attachments;
	};

	struct FramebufferSpecification
	{
		uint32_t Width = 0, Height = 0;
		FramebufferAttachmentSpecification Attachments;
		uint32_t Samples = 1;

		bool SwapChainTarget = false;
	};

	class Framebuffer : public RefCounted {
	public:
		Framebuffer(const FramebufferSpecification& spec);
		virtual ~Framebuffer();

		void invalidate();

		void bind();
		void unbind();

		void resize(uint32_t width, uint32_t height);
		int readPixel(uint32_t attachmentIndex, int x, int y);

		void clearAttachment(uint32_t attachmentIndex, int value);

		const FramebufferSpecification& getSpecification() const { return m_Specification; }
		uint32_t getColorAttachmentRendererID(uint32_t index = 0) const {
			SHADO_CORE_ASSERT(index < m_ColorAttachments.size(), "Framebuffer index is creater than Colour attachment size");
			return m_ColorAttachments[index];
		}

		static Ref<Framebuffer> create(const FramebufferSpecification& spec) { return CreateRef<Framebuffer>(spec); }

	private:
		uint32_t m_RendererID = 0;
		FramebufferSpecification m_Specification;

		std::vector<FramebufferTextureSpecification> m_ColorAttachmentSpecifications;
		FramebufferTextureSpecification m_DepthAttachmentSpecification = FramebufferTextureFormat::None;

		std::vector<uint32_t> m_ColorAttachments;
		uint32_t m_DepthAttachment = 0;
	};
}
