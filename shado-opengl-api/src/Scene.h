#pragma once
#include <string>
#include "Events/Event.h"
#include "util/Util.h"

namespace Shado {
	class Scene {

	public:
		Scene(const std::string& name, int zIndex);
		Scene(const std::string& name);
		virtual ~Scene() {}

		virtual void onInit() = 0;
		virtual void onUpdate(TimeStep dt) = 0;
		virtual void onDraw() = 0;
		virtual void onImGuiRender() = 0;
		virtual void onDestroy() = 0;
		virtual void onEvent(Event& event) = 0;

		std::string			getName()	const { return m_Name; }
		unsigned long long	getId()		const { return m_Id; }
		int					getZIndex()	const { return zIndex; }

	private:
		std::string m_Name;
		unsigned long long m_Id;
		int zIndex;
	};


}
