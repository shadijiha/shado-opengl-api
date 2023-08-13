#pragma once
#include "util/Util.h"

namespace Shado {

	class UniformBuffer : public RefCounted
	{
	public:
		UniformBuffer(uint32_t size, uint32_t binding);
		virtual ~UniformBuffer();
		virtual void setData(const void* data, uint32_t size, uint32_t offset = 0);

		static Ref<UniformBuffer> create(uint32_t size, uint32_t binding);
	private:
		uint32_t m_RendererID = 0;
	};
}
