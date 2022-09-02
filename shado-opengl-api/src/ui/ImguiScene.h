#pragma once
#include "renderer/Layer.h"
#include "Events/KeyEvent.h"
#include <imgui.h>	// keep this here

namespace Shado {
	
	class ImguiLayer : public Layer
	{
	public:
		ImguiLayer(bool showDemo = false);
		~ImguiLayer();

		void onInit() override;
		void onDestroy() override;
		void onUpdate(TimeStep dt) override;
		void onDraw() override {}
		void onEvent(Event& event) override;
		void onImGuiRender() override;

		void begin();
		void end();

		void setBlockEvents(bool b) { m_BlockEvents = b; }
		bool isBlockingEvents() const { return m_BlockEvents; }

		enum FontIndex {
			BOLD = 0, REGULAR = 1
		};

	private:
		void setDarkThemeColors();
	private:
		float m_Time;
		bool m_ShowDemo;
		bool m_BlockEvents = true;
	};
}
