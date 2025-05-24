#include "Application.h"
#include <algorithm>
#include "asset/AssetManager.h" // <--- This is needed DO NOT REMOVE
#include "debug/Profile.h"
#include "Events/ApplicationEvent.h"
#include "GL/glew.h"
#include "project/Project.h"
#include "renderer/Renderer2D.h"
#include "scene/Scene.h"
#include "script/ScriptEngine.h"
#include "util/Random.h"

namespace Shado {
    // =========================== APPLICATION CLASS ===========================
    Application::Application(unsigned width, unsigned height, const std::string& title)
        : window(snew(Window) Window(width, height, title)), uiScene(snew(ImguiLayer) ImguiLayer) {
        Log::init();
        Random::init();
        ScriptEngine::GetMutable().InitializeHost();
    }

    Application::Application()
        : Application(1280, 720, "Shado OpenGL simple Rendering engine") {}

    Application::~Application() {
        SHADO_PROFILE_FUNCTION();

        ScriptEngine::GetMutable().ShutdownHost();

        for (Layer* layer : layers) {
            if (layer == nullptr)
                continue;
            layer->onDestroy();
            sdelete(layer);
        }

        Project::SetActive(nullptr);

        glfwTerminate();
    }

    Application& Application::get() {
        static Application* instance = nullptr;
        if (!instance) {
            instance = snew(Application) Application();
            return *instance;
        }
        return *instance;
    }

    void Application::run() {
        // Init Renderer if it hasn't been done
        Init();

        if (layers.empty())
            SHADO_CORE_WARN("No layers to draw");


        /* Loop until the user closes the window */
        while (m_Running) {
            float time = static_cast<float>(glfwGetTime()); // TODO: put it in platform specific
            float timestep = time - m_LastFrameTime;
            m_LastFrameTime = time;

            // Execute main thread Queue
            {
                std::scoped_lock<std::mutex> lock(m_MainThreadQueueMutex);

                for (auto& func : m_MainThreadQueue)
                    func();

                m_MainThreadQueue.clear();
            }

            if (!m_minimized) {
                /* Render here */
                // Draw scenes here
                for (Layer* layer : layers) {
                    if (layer == nullptr) {
                        continue;
                    }

                    layer->onUpdate(timestep);
                    layer->onDraw();
                }
                uiScene->onUpdate(timestep);
                uiScene->onDraw();

                // Render UI
                uiScene->begin();
                for (Layer* layer : layers) {
                    if (layer != nullptr)
                        layer->onImGuiRender();
                }
                uiScene->onImGuiRender();
                uiScene->end();
            }

            /* Swap front and back buffers */
            /* Poll for and process events */
            window->onUpdate();
        }
    }

    void Application::submit(Layer* layer) {
        // Init Renderer if it hasn't been done
        Init();

        // Init all layers
        layer->onInit();
        layers.push_back(layer);

        // Reverse sorting
        /*std::sort(allScenes.begin(), allScenes.end(), [](Scene* a, Scene* b) {
            return a->getZIndex() < b->getZIndex();
            });*/
    }

    void Application::onEvent(Event& e) {
        SHADO_PROFILE_FUNCTION();

        Init();

        //SHADO_CORE_TRACE("App received event {}", e.toString());

        EventDispatcher dispatcher(e);
        dispatcher.dispatch<WindowResizeEvent>([this](WindowResizeEvent& evt) {
            int width = evt.getWidth(), height = evt.getHeight();
            if (width == 0 || height == 0) {
                m_minimized = true;
                return false;
            }
            m_minimized = false;
            glViewport(0, 0, width, height);

            return false;
        });

        dispatcher.dispatch<WindowCloseEvent>([this](WindowCloseEvent& evt) {
            m_Running = false;
            return true;
        });

        dispatcher.dispatch<ProjectChangedEvent>([this](ProjectChangedEvent& evt) {
            this->getWindow().setTitle(
#ifdef SHADO_DEBUG
                "[Debug]" +
#elif SHADO_RELEASE
                "[Release] " +
#elif SHADO_DIST
				"[Dist] " +
#else
				"[Unknown build config] " +
#endif
                evt.getProject()->GetConfig().Name + " - " + evt.getProject()->GetProjectDirectory().string()
            );
            return false;
        });

        // Distaptch the event and execute the required code
        // Pass Events to layer

        uiScene->onEvent(e);
        if (e.isHandled()) {
            //SHADO_CORE_TRACE("UI layer handled event.");
            return;
        }

        for (auto it = layers.end(); it != layers.begin();) {
            Layer* layer = (*--it);
            if (layer != nullptr) {
                layer->onEvent(e);
                if (e.isHandled()) {
                    SHADO_CORE_TRACE("Layer {} handled event.", layer->getName());
                    break;
                }
            }
        }
    }

    void Application::SubmitToMainThread(const std::function<void()>& function) {
        std::scoped_lock lock(m_MainThreadQueueMutex);
        m_MainThreadQueue.emplace_back(function);
    }

    void Application::AddOnDestroyedCallback(const std::function<void()>& function) {
        std::scoped_lock lock(m_TeardownCallbacksMutex);
        m_TeardownCallbacks.emplace_back(function);
    }

    void Application::destroy() {
        SHADO_PROFILE_FUNCTION();

        for (Layer*& layer : get().layers) {
            layer->onDestroy();
            sdelete(layer);
            layer = nullptr;
        }
        get().uiScene->onDestroy();

        get().m_Running = false;

        Renderer2D::Shutdown();
        for (const auto& cb : get().m_TeardownCallbacks)
            cb();
        sdelete(&get());
    }

    void Application::close() {
        get().m_Running = false;
    }

    double Application::getTime() const {
        return glfwGetTime();
    }

    void Application::Init() {
        SHADO_PROFILE_FUNCTION();
        if (!Renderer2D::hasInitialized()) {
            Renderer2D::Init();
            uiScene->onInit();
        }
    }
}
