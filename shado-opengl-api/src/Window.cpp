#include "Window.h"

#include <filesystem>


#include "Application.h"
#include "debug/Debug.h"
#include "Events/ApplicationEvent.h"
#include "Events/KeyEvent.h"
#include "Events/MouseEvent.h"
#include "GL/glew.h"
#include <GLFW/glfw3.h>
#if SHADO_PLATFORM_WINDOWS
#include <shellscalingapi.h>
#endif

#define BIND_EVENT_FN(x) std::bind(&Window::x, this, std::placeholders::_1)

namespace Shado {
    Window::Window(uint32_t width, uint32_t height, const std::string& title, WindowMode mode)
        : m_Mode(mode) {
        GLFWallocator allocator;
        allocator.allocate = [](size_t size, void* user) { return Memory::HeapRaw(size, "GLFW"); };
        allocator.reallocate = [](void* block, size_t size, void* user) {
            return Memory::ReallocRaw(block, size, "GLFW");
        };
        allocator.deallocate = [](void* block, void* user) { Memory::FreeRaw(block, "GLFW"); };
        allocator.user = NULL;
        glfwInitAllocator(&allocator);

#if defined(SHADO_PLATFORM_MACOS)
        // On macOS, GLFW changes the process working directory to the app
        // bundle's Resources folder by default. The engine loads assets/shaders
        // with paths relative to the executable directory, so disable this.
        glfwInitHint(GLFW_COCOA_CHDIR_RESOURCES, GLFW_FALSE);
#endif

        /* Initialize the library */
        if (!glfwInit())
            SHADO_CORE_ASSERT(false, "Failed to initialize GLFW!");

        m_Data.title = title;
        m_Data.width = width;
        m_Data.height = height;

#if defined(SHADO_PLATFORM_MACOS)
        /*
         * macOS only exposes Core profile contexts for OpenGL 3.2+, capped at
         * 4.1, and requires forward compatibility. The engine's shaders are
         * downgraded to GLSL 4.1 there, so request a 4.1 Core context.
         * Windows/Linux keep the default context (compatibility, up to 4.6),
         * which provides the Direct State Access functions the renderer uses.
         */
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif

        /* Create a windowed mode window and its OpenGL context */
        native_window = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);
        if (!native_window) {
            glfwTerminate();
            SHADO_CORE_ASSERT(false, "Failed to create window");
        }

        /* Make the window's context current */
        glfwMakeContextCurrent(native_window);

#if defined(SHADO_PLATFORM_MACOS)
        // Under an OpenGL Core profile context, GLEW must use "experimental"
        // entry points to resolve function pointers correctly.
        glewExperimental = GL_TRUE;
#endif
        if (glewInit() != GLEW_OK) {
            glfwTerminate();
            SHADO_CORE_ASSERT(false, "Failed to create GLEW context");
        }
#if defined(SHADO_PLATFORM_MACOS)
        // GLEW on a Core profile triggers a benign GL_INVALID_ENUM; clear it.
        glGetError();
#endif

        glfwSetWindowUserPointer(native_window, &m_Data);
        listenToEvents();

        glfwGetWindowSize(native_window, (int*)&m_Data.width, (int*)&m_Data.height);
        glfwGetWindowPos(native_window, &m_Position.first, &m_Position.second);

        monitor = glfwGetPrimaryMonitor();


#if defined(SHADO_PLATFORM_WINDOWS)
        SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
#endif
    }

    Window::Window()
        : Window(1280, 720) {
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
        glfwSetWindowMonitor(native_window, nullptr, m_Position.first, m_Position.second, m_Size.first, m_Size.second,
                             0);

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

    std::pair<float, float> Window::getWindowScale() const {
        float xscale = 1.0f, yscale = 1.0f;
        // glfwGetPrimaryMonitor() can return NULL (e.g. display asleep or no
        // monitor); passing NULL to glfwGetMonitorContentScale asserts inside
        // GLFW and aborts. Fall back to a 1.0 scale.
        GLFWmonitor* mon = monitor ? monitor : glfwGetPrimaryMonitor();
        if (mon)
            glfwGetMonitorContentScale(mon, &xscale, &yscale);
        return std::pair(xscale, yscale);
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
            case GLFW_PRESS: {
                KeyPressedEvent event(key, 0);
                data.eventCallback(event);
                break;
            }
            case GLFW_RELEASE: {
                KeyReleasedEvent event(key);
                data.eventCallback(event);
                break;
            }
            case GLFW_REPEAT: {
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
            case GLFW_PRESS: {
                MouseButtonPressedEvent event(button);
                data.eventCallback(event);
                break;
            }
            case GLFW_RELEASE: {
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

        glfwSetDropCallback(native_window, [](GLFWwindow* window, int pathCount, const char* paths[]) {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
            std::vector<std::filesystem::path> filepaths(pathCount);
            for (int i = 0; i < pathCount; i++)
                filepaths[i] = paths[i];
            WindowDropEvent event(std::move(filepaths));
            data.eventCallback(event);
        });
    }

    bool Window::onWindowClose(WindowCloseEvent& e) {
        Application::close();
        return false;
    }

    bool Window::onWindowResize(WindowResizeEvent& e) {
        if (e.getWidth() == 0 || e.getHeight() == 0) {
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
