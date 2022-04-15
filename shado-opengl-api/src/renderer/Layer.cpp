#include "Layer.h"

#include "debug/Debug.h"

namespace Shado {

	Layer::Layer(const std::string& name)
		: m_Name(name), m_Id(rand() % UINT64_MAX)
	{
	}
}
