#pragma once
#include <mutex>
#include <string>
#include <vector>
#include "Window.h"
#include "debug/Debug.h"
#include "Events/Event.h"
#include "ui/ImguiScene.h"
#include "util/Memory.h"

namespace Shado {
    class Application {
    public:
        Application(unsigned int width, unsigned int height, const std::string& title);

        Application();

        ~Application();

        static Application& get();

        static void destroy();

        static void close();

        template <typename T> requires std::is_base_of_v<Event, T>
        static void dispatchEvent(T& event) {
            SHADO_CORE_TRACE("Dispatched event {}", event.toString());
            Application& app = get();
            app.onEvent(event);
        }

        template <typename T> requires std::is_base_of_v<Event, T>
        static void dispatchEvent(const T& event) {
            Application::dispatchEvent(const_cast<T&>(event));
        }

        void run();

        void submit(Layer* layer);

        void onEvent(Event& e);

        void SubmitToMainThread(const std::function<void()>& function);

        void AddOnDestroyedCallback(const std::function<void()>& function);

        Window& getWindow() const { return *window; }
        ImguiLayer* getUILayer() const { return uiScene; }

        double getTime() const;

    private:
        void Init();

        ScopedRef<Window> window;
        ImguiLayer* uiScene;
        float m_LastFrameTime = 0.0f; // Time took to render last frame	

        bool m_Running = true;
        bool m_minimized = false;

        std::vector<Layer*> layers;

        std::vector<std::function<void()>> m_MainThreadQueue;
        std::mutex m_MainThreadQueueMutex;

        std::vector<std::function<void()>> m_TeardownCallbacks;
        std::mutex m_TeardownCallbacksMutex;
    };
}
