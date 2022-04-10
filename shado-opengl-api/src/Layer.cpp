#include "Layer.h"

#include "Debug.h"

namespace Shado {

	Layer::Layer(const std::string& name)
		: m_Name(name), m_Id(rand() % UINT64_MAX)
	{
	}
}
