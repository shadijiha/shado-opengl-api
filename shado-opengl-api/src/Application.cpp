#include "Application.h"

#include "Debug.h"
#include "GL/glew.h"
#include "Renderer2D.h"
#include <algorithm>
#include "cameras/OrthoCamera.h"
#include "Events/ApplicationEvent.h"
#include "Events/KeyEvent.h"
#include "Events/MouseEvent.h"
#include "Renderer3D.h"
#include "util/Random.h"

namespace Shado {

	// =========================== APPLICATION CLASS ===========================

	Application* Application::singleton = new Application();

	Application::Application(unsigned width, unsigned height, const std::string& title)
		: window(new Window(width, height, title)), uiScene(new ImguiLayer)
	{
		Log::init();
		Random::init();

		/* Initialize the library */
		// TODO: Might want to un comment this if application crashes
		//window = std::make_unique<Window>(width, height, title);
	}

	Application::Application()
		: Application(1280, 720, "Shado OpenGL simple Rendering engine")
	{
	}

	Application::~Application() {

		for (Scene* scene : allScenes) {
			if (scene == nullptr)
				continue;
			
			for (Layer* layer : m_activeScene->getLayers()) {
				if (layer == nullptr)
					continue;

				layer->onDestroy();
			}
			
			delete scene;
		}

		glfwTerminate();
	}

	void Application::run() {

		// Init Renderer if it hasn't been done
		if (!Renderer2D::hasInitialized()) {
			Renderer2D::Init();
			Renderer3D::Init();
			uiScene->onInit();
		}

		if (allScenes.size() == 0)
			SHADO_CORE_WARN("No layers to draw");


		/* Loop until the user closes the window */
		while (m_Running) {

			float time = (float)glfwGetTime();	// TODO: put it in platform specific
			float timestep = time - m_LastFrameTime;
			m_LastFrameTime = time;

			/* Render here */
			Renderer2D::Clear();

			// Draw scenes here
			if (m_activeScene != nullptr) {
				for (Layer* layer : m_activeScene->getLayers()) {
					if (layer == nullptr) {
						continue;
					}

					layer->onUpdate(timestep);
					layer->onDraw();
				}
				m_activeScene->onUpdate(timestep);
			}
			uiScene->onUpdate(timestep);
			uiScene->onDraw();

			// Render UI
			uiScene->begin();
			if (m_activeScene != nullptr) {
				for (Layer* layer : m_activeScene->getLayers()) {
					if (layer != nullptr)
						layer->onImGuiRender();
				}
			}
			uiScene->onImGuiRender();
			uiScene->end();

			/* Swap front and back buffers */
			/* Poll for and process events */
			window->onUpdate();
		}
	}

	void Application::submit(Scene* scene) {

		// Init Renderer if it hasn't been done
		if (!Renderer2D::hasInitialized()) {
			Renderer2D::Init();
			Renderer3D::Init();
			uiScene->onInit();
		}

		// Init all layers
		for (const auto& layer : scene->getLayers()) {
			layer->onInit();
		}
		
		allScenes.push_back(scene);

		// The first scene submitted should be the active one
		if (m_activeScene == nullptr)
			m_activeScene = scene;

		// Reverse sorting
		/*std::sort(allScenes.begin(), allScenes.end(), [](Scene* a, Scene* b) {
			return a->getZIndex() < b->getZIndex();
			});*/
	}

	void Application::onEvent(Event& e) {
		if (m_activeScene == nullptr)
			return;
		
		// Distaptch the event and excute the required code
		// Pass Events to layer
		for (Layer* layer : m_activeScene->getLayers()) {
			if (layer != nullptr) {
				layer->onEvent(e);
				if (e.isHandled())
					break;
			}
		}
	}

	void Application::setActiveScene(Scene* scene) {
		if (std::find(allScenes.begin(), allScenes.end(), scene) == allScenes.end()) {
			SHADO_CORE_ERROR("Scene {0} is set as active but is not added to Application!", scene->getName());
		}

		// Unmount the current scene
		if (m_activeScene != nullptr)
			m_activeScene->onUnMount();

		m_activeScene = scene;
		m_activeScene->onMount();
	}

	void Application::setActiveScene(const std::string& name) {
		Scene* ptr = nullptr;
		for (Scene* scene : allScenes) {
			if (scene->getName() == name) {
				ptr = scene;
				break;
			}
		}

		// Scene was not found
		if (ptr == nullptr) {
			SHADO_CORE_ERROR("Attempted to set an invalid active scene ({0})!", name);
			return;
		}

		setActiveScene(ptr);
	}

	void Application::destroy() {

		for (Scene*& scene : singleton->allScenes) {
			for (auto& layer : scene->getLayers()) {
				layer->onDestroy();
			}
			delete scene;
			scene = nullptr;
		}
		singleton->uiScene->onDestroy();

		singleton->m_Running = false;

		delete singleton;
	}

	void Application::close() {
		singleton->m_Running = false;
	}

}
