#pragma once
#include <string>
#include <vector>
#include <mutex>
#include "util/Memory.h"
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

        template <typename T> requires std::is_base_of_v<Event, T>
        static void dispatchEvent(T& event) {
            Application& app = get();
            app.onEvent(event);
        }

        template <typename T>
        static void dispatchEvent(const T& event) {
            Application::dispatchEvent(const_cast<T&>(event));
        }

        void run();
        void submit(Layer* scene);
        void onEvent(Event& e);

        void SubmitToMainThread(const std::function<void()>& function);

        Window& getWindow() { return *window; }
        ImguiLayer* getUILayer() { return uiScene; }
        float getTime() const;

    private:
        void Init();
        ScopedRef<Window> window; // TODO: This might be a bad idea, might want to revert to std::unique_ptr
        ImguiLayer* uiScene;
        float m_LastFrameTime = 0.0f; // Time took to render last frame	

        bool m_Running = true;
        bool m_minimized = false;

        std::vector<Layer*> layers;

        std::vector<std::function<void()>> m_MainThreadQueue;
        std::mutex m_MainThreadQueueMutex;

        static Application* singleton;
    };
}
