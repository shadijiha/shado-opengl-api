#pragma once
#include <string>
#include "Events/Event.h"
#include "util/Util.h"

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

	class Scene {
	public:
		Scene(const std::string& name);
		virtual ~Scene();

		/// <summary>
		/// Called when the scene is set as active
		/// </summary>
		virtual void onMount()		{}
		
		/// <summary>
		/// Called when the scene is unmounted (is inactive)
		/// </summary>
		virtual void onUnMount()	{}

		void pushLayer(Layer* layer);
		
		const std::vector<Layer*>& getLayers()	const;
		const std::string& getName()			const { return name; }
		
	protected:
		std::vector<Layer*> m_Layers;
		std::string name;
	};
}
