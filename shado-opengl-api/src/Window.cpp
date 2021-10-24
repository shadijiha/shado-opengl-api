#include "Window.h"


#include "Application.h"
#include "Debug.h"
#include "Events/ApplicationEvent.h"
#include "Events/KeyEvent.h"
#include "Events/MouseEvent.h"
#include "GL/glew.h"

#define BIND_EVENT_FN(x) std::bind(&Window::x, this, std::placeholders::_1)

namespace Shado {

	Window::Window(uint32_t width, uint32_t height, const std::string& title, WindowMode mode)
		: m_Mode(mode)
	{
		/* Initialize the library */
		if (!glfwInit())
			SHADO_CORE_ASSERT(false, "Failed to initialize GLFW!");

		m_Data.title = title;
		m_Data.width = width;
		m_Data.height = height;

		/* Create a windowed mode window and its OpenGL context */
		native_window = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);
		if (!native_window)
		{
			glfwTerminate();
			SHADO_CORE_ASSERT(false, "Failed to create window");
		}

		/* Make the window's context current */
		glfwMakeContextCurrent(native_window);

		if (glewInit() != GLEW_OK) {
			glfwTerminate();
			SHADO_CORE_ASSERT(false, "Failed to create GLEW context");
		}

		glfwSetWindowUserPointer(native_window, &m_Data);
		listenToEvents();

		glfwGetWindowSize(native_window, (int*)&m_Data.width, (int*)&m_Data.height);
		glfwGetWindowPos(native_window, &m_Position.first, &m_Position.second);

		monitor = glfwGetPrimaryMonitor();
	}

	Window::Window()
		: Window(1280, 720)
	{
	}

	Window::~Window() {
		glfwDestroyWindow(native_window);
	}

	void Window::onUpdate() {
		glfwSwapBuffers(native_window);
		glfwPollEvents();
	}

	void Window::setTitle(const std::string& title) {
		if (title == m_Data.title)
			return;

		m_Data.title = title;
		glfwSetWindowTitle(native_window, title.c_str());
	}

	void Window::setVSync(bool enabled) {
		if (enabled)
			glfwSwapInterval(1);
		else
			glfwSwapInterval(0);

		m_Data.VSync = enabled;
	}

	void Window::resize(uint32_t width, uint32_t height) {
		glfwSetWindowSize(native_window, width, height);
		updateViewport();

		WindowResizeEvent event(width, height);
		onEvent(event);
	}

	void Window::setMode(WindowMode mode) {

		if (mode == m_Mode)
			return;

		switch (mode) {
		case WindowMode::FULLSCREEN:
			makeFullScreen();
			break;
		case WindowMode::WINDOWED:
			makeWindowed();
			break;
		case WindowMode::BORDERLESS_WINDOWED:
			makeBorderless();
			break;
		}
		
		// TODO: implement
		m_Mode = mode;
	}

	void Window::setOpacity(float opacity) {
		glfwSetWindowOpacity(native_window, opacity);
	}

	void Window::setResizable(bool resizable) {
		glfwSetWindowAttrib(native_window, GLFW_RESIZABLE, resizable);
	}

	void Window::makeFullScreen() {
		// backup window position and window size
		glfwGetWindowPos(native_window, &m_Position.first, &m_Position.second);
		glfwGetWindowSize(native_window, &m_Size.first, &m_Size.second);

		// get resolution of monitor
		const GLFWvidmode* mode = glfwGetVideoMode(monitor);

		glfwSetWindowMonitor(native_window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);

		updateViewport();
	}

	void Window::makeWindowed() {

		// restore last window size and position
		glfwSetWindowMonitor(native_window, nullptr, m_Position.first, m_Position.second, m_Size.first, m_Size.second, 0);

		updateViewport();
	}

	void Window::makeBorderless() {
		
		const GLFWvidmode* mode = glfwGetVideoMode(monitor);

		glfwWindowHint(GLFW_RED_BITS, mode->redBits);
		glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
		glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
		glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);		

		glfwSetWindowMonitor(native_window, nullptr, 0, 0, mode->width, mode->height, mode->refreshRate);

		updateViewport();
	}

	void Window::updateViewport() {

		int width = m_Data.width;
		int height = m_Data.height;
		
		glfwGetFramebufferSize(native_window, &width, &height);
		glViewport(0, 0, width, height);

		// Send the resize event to the application
		WindowResizeEvent event(width, height);
		onEvent(event);
	}

	void Window::onEvent(Event& e) {
		EventDispatcher dispatcher(e);
		dispatcher.dispatch<WindowCloseEvent>(BIND_EVENT_FN(onWindowClose));
		dispatcher.dispatch<WindowResizeEvent>(BIND_EVENT_FN(onWindowResize));

		// Pass it to the Application
		Application::get().onEvent(e);
	}

	uint32_t Window::getWidth() const {
		int width;
		int height;
		glfwGetWindowSize(native_window, &width, &height);
		
		return width;
	}

	uint32_t Window::getHeight() const {
		int width;
		int height;
		glfwGetWindowSize(native_window, &width, &height);

		return height;
	}

	uint32_t Window::getPosX() const {
		int x;
		int y;
		glfwGetWindowPos(native_window, &x, &y);

		return x;
	}

	uint32_t Window::getPosY() const {
		int x;
		int y;
		glfwGetWindowPos(native_window, &x, &y);

		return y;
	}

	void Window::listenToEvents() {

		m_Data.eventCallback = BIND_EVENT_FN(onEvent);

		// Set GLFW callbacks
		glfwSetWindowSizeCallback(native_window, [](GLFWwindow* window, int width, int height) {
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			data.width = width;
			data.height = height;

			WindowResizeEvent event(width, height);
			data.eventCallback(event);
			});

		glfwSetWindowCloseCallback(native_window, [](GLFWwindow* window) {
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			WindowCloseEvent event;
			data.eventCallback(event);
			});

		glfwSetKeyCallback(native_window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			switch (action) {
			case GLFW_PRESS:
			{
				KeyPressedEvent event(key, 0);
				data.eventCallback(event);
				break;
			}
			case GLFW_RELEASE:
			{
				KeyReleasedEvent event(key);
				data.eventCallback(event);
				break;
			}
			case GLFW_REPEAT:
			{
				KeyPressedEvent event(key, 1);
				data.eventCallback(event);
				break;
			}
			}
			});

		glfwSetCharCallback(native_window, [](GLFWwindow* window, unsigned int keycode) {
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			KeyTypedEvent event(keycode);
			data.eventCallback(event);
			});

		glfwSetMouseButtonCallback(native_window, [](GLFWwindow* window, int button, int action, int mods) {
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			switch (action) {
			case GLFW_PRESS:
			{
				MouseButtonPressedEvent event(button);
				data.eventCallback(event);
				break;
			}
			case GLFW_RELEASE:
			{
				MouseButtonReleasedEvent event(button);
				data.eventCallback(event);
				break;
			}
			}
			});

		glfwSetScrollCallback(native_window, [](GLFWwindow* window, double xOffset, double yOffset) {
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			MouseScrolledEvent event((float)xOffset, (float)yOffset);
			data.eventCallback(event);
			});

		glfwSetCursorPosCallback(native_window, [](GLFWwindow* window, double xPos, double yPos) {
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			MouseMovedEvent event((float)xPos, (float)yPos);
			data.eventCallback(event);
			});
	}

	bool Window::onWindowClose(WindowCloseEvent& e) {
		Application::close();
		return false;
	}

	bool Window::onWindowResize(WindowResizeEvent& e) {
		if (e.getWidth() == 0 || e.getHeight() == 0)
		{
			//m_Minimized = true;
			return false;
		}

		//m_Minimized = false;
		glViewport(0, 0, e.getWidth(), e.getHeight());


		m_Data.width = e.getWidth();
		m_Data.height = e.getHeight();

		return false;
	}

}
