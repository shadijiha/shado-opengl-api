#pragma once
#include <lua.h>
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
		void submit(Scene* scene);
		void onEvent(Event& e);

		void setActiveScene(Scene* scene);
		void setActiveScene(const std::string& name);

		Window& getWindow()								{ return *window; }
		const std::vector<Scene*>& getScenes()	const	{ return allScenes; }
		const Scene& getActiveScene()			const	{ return *m_activeScene; }

	private:
		ScopedPtr<Window> window;		// TODO: This might be a bad idea, might want to revert to std::unique_ptr
		ImguiLayer* uiScene;
		float m_LastFrameTime = 0.0f;	// Time took to render last frame	
		
		bool m_Running = true;

		std::vector<Scene*> allScenes;
		Scene* m_activeScene = nullptr;

		static Application* singleton;
	};
}
