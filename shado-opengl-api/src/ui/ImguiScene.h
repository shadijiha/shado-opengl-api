#pragma once
#include "Layer.h"
#include "Events/KeyEvent.h"
#include <imgui.h>	// keep this here

namespace Shado {

	class ImguiLayer : public Scene
	{
	public:
		ImguiLayer(bool showDemo = false);
		~ImguiLayer();

		void onInit() override;
		void onDestroy() override;
		void onUpdate(TimeStep dt) override;
		void onDraw() override {}
		void onEvent(Event& event) override {}
		void onImGuiRender() override;

		void begin();
		void end();
	private:
		float m_Time;
		bool m_ShowDemo;
	};
}
