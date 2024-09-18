#include "Application.h"
#include "debug/Profile.h"
#include "GL/glew.h"
#include "renderer/Renderer2D.h"
#include <algorithm>
#include "cameras/OrthoCamera.h"
#include "Events/ApplicationEvent.h"
#include "Events/KeyEvent.h"
#include "Events/MouseEvent.h"
#include "project/Project.h"
#include "scene/Scene.h"
#include "script/ScriptEngine.h"
#include "util/Random.h"

namespace Shado {
    // =========================== APPLICATION CLASS ===========================

    Application* Application::singleton = snew(Application) Application();

    Application::Application(unsigned width, unsigned height, const std::string& title)
        : window(snew(Window) Window(width, height, title)), uiScene(snew(ImguiLayer) ImguiLayer) {
        Log::init();
        Random::init();
        ScriptEngine::GetMutable().InitializeHost();

        /* Initialize the library */
        // TODO: Might want to un comment this if application crashes
        //window = std::make_unique<Window>(width, height, title);
    }

    Application::Application()
        : Application(1280, 720, "Shado OpenGL simple Rendering engine") {
    }

    Application::~Application() {
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

    void Application::run() {
        // Init Renderer if it hasn't been done
        Init();

        if (layers.size() == 0)
            SHADO_CORE_WARN("No layers to draw");


        /* Loop until the user closes the window */
        while (m_Running) {
            float time = (float)glfwGetTime(); // TODO: put it in platform specific
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
        Init();

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

        // Distaptch the event and excute the required code
        // Pass Events to layer

        uiScene->onEvent(e);
        if (e.isHandled())
            return;

        for (auto it = layers.end(); it != layers.begin();) {
            Layer* layer = (*--it);
            if (layer != nullptr) {
                layer->onEvent(e);
                if (e.isHandled())
                    break;
            }
        }
    }

    void Application::SubmitToMainThread(const std::function<void()>& function) {
        std::scoped_lock<std::mutex> lock(m_MainThreadQueueMutex);

        m_MainThreadQueue.emplace_back(function);
    }

    void Application::destroy() {
        SHADO_PROFILE_FUNCTION();

        for (Layer*& layer : singleton->layers) {
            layer->onDestroy();
            sdelete(layer);
            layer = nullptr;
        }
        singleton->uiScene->onDestroy();

        singleton->m_Running = false;

        Renderer2D::Shutdown();
        sdelete(singleton);
    }

    void Application::close() {
        singleton->m_Running = false;
    }

    float Application::getTime() const {
        return glfwGetTime();
    }

    void Application::Init() {
        if (!Renderer2D::hasInitialized()) {
            Renderer2D::Init();
            uiScene->onInit();
        }
    }
}
