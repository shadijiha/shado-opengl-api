#pragma once
#include <string>

#include "box2d/b2_world.h"
#include "Events/Event.h"
#include "util/Util.h"
#include "Entity.h"

namespace Shado {
	
	class Layer {
	public:
		Layer(const std::string& name = "Layer");
		virtual ~Layer() = default;

		virtual void onInit()				{}
		virtual void onUpdate(TimeStep dt)	{}
		virtual void onDraw()				{}
		virtual void onImGuiRender()		{}
		virtual void onDestroy()			{}
		virtual void onEvent(Event& event)	{}

		std::string			getName()	const { return m_Name; }
		unsigned long long	getId()		const { return m_Id; }

	private:
		std::string m_Name;
		unsigned long long m_Id;
	};

}
