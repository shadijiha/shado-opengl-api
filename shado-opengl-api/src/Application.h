#pragma once
#include <string>
#include <vector>
#include "Renderer2D.h"
#include "Events/Event.h"
#include "Window.h"
#include "ui/ImguiScene.h"

namespace Shado {

	class Application {
	public:
		Application(unsigned int width, unsigned int height, const std::string& title);
		Application();
		~Application();

		static Application& get() { return *singleton; }
		static void destroy();

		static void close();

		void run();
		void submit(Layer* scene);
		void onEvent(Event& e);

		Window& getWindow() { return *window; }

	private:
		ScopedPtr<Window> window;		// TODO: This might be a bad idea, might want to revert to std::unique_ptr
		ImguiLayer* uiScene;
		float m_LastFrameTime = 0.0f;	// Time took to render last frame	
		
		bool m_Running = true;

		std::vector<Layer*> layers;

		static Application* singleton;
	};
}
