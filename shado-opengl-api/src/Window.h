#pragma once

#include "renderer/opengl.h"
#include <string>

#include "Events/Event.h"
#include "Events/ApplicationEvent.h"

namespace Shado {

	enum class WindowMode {
		FULLSCREEN, WINDOWED, BORDERLESS_WINDOWED
	};
	
	class Window {
	public:
		Window(uint32_t width, uint32_t height, const std::string& title = "Shado OpenGL Engine", WindowMode mode = WindowMode::WINDOWED);
		Window();
		~Window();

		void onUpdate();

		void setTitle(const std::string& title);
		void setVSync(bool enabled = true);
		void resize(uint32_t width, uint32_t height);
		void setMode(WindowMode mode);	// TODO: Set full screen mode etc
		void setOpacity(float opacity = 1.0f);
		void setResizable(bool resizable = true);

		// Events
		void onEvent(Event& e);
		
		uint32_t getWidth() const;
		uint32_t getHeight() const;
		uint32_t getPosX() const;
		uint32_t getPosY() const;
		float getAspectRatio() const { return (float)getWidth() / (float)getHeight(); }
		const std::string& getTitle() const { return m_Data.title; }
		WindowMode getMode() const { return m_Mode; }
		bool isVSync() const { return vsync; }
		
		GLFWwindow* getNativeWindow() const { return native_window; }	

	private:
		using EventCallbackFn = std::function<void(Event&)>;
		struct WindowData
		{
			std::string title;
			unsigned int width, height;
			bool VSync;

			EventCallbackFn eventCallback;
		};

		void listenToEvents();
		bool onWindowClose(WindowCloseEvent& e);
		bool onWindowResize(WindowResizeEvent& e);

		void makeFullScreen();
		void makeWindowed();
		void makeBorderless();

		void updateViewport();
		
	private:
		GLFWwindow* native_window;
		WindowData m_Data;
		WindowMode m_Mode;
		GLFWmonitor* monitor;

		std::pair<int, int> m_Position;
		std::pair<int, int> m_Size;

		bool vsync = true;
	};
	
}
