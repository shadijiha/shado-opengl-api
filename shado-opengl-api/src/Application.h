#pragma once
#include <string>
#include <vector>
#include "renderer/Renderer2D.h"
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

		void SubmitToMainThread(const std::function<void()>& function);

		Window& getWindow() { return *window; }
		ImguiLayer* getUILayer() { return uiScene; }
	private:
		void Init();
	private:
		ScopedPtr<Window> window;		// TODO: This might be a bad idea, might want to revert to std::unique_ptr
		ImguiLayer* uiScene;
		float m_LastFrameTime = 0.0f;	// Time took to render last frame	
		
		bool m_Running = true;
		bool m_minimized = false;

		std::vector<Layer*> layers;

		std::vector<std::function<void()>> m_MainThreadQueue;
		std::mutex m_MainThreadQueueMutex;

		static Application* singleton;
	};
}
