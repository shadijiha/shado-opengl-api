#include "EditorLayer.h"

#include "Application.h"

namespace Shado {

	EditorLayer::EditorLayer()
		: Layer("Editor"), m_camera_controller(Application::get().getWindow().getAspectRatio(), true)
	{
		Application::get().getWindow().setTitle("Shado Engine Editor");
	}

    EditorLayer::~EditorLayer()
	{
	}

	void EditorLayer::onInit() {
        Renderer2D::Init();
        FrameBufferSpecification specs;
        specs.width = Application::get().getWindow().getWidth();
        specs.height = Application::get().getWindow().getHeight();
        buffer = FrameBuffer::create(specs);

        Renderer2D::SetClearColor({ 0, 0, 0, 1 });

		// Debug code
		int width = Application::get().getWindow().getWidth();
		int height = Application::get().getWindow().getHeight();

        m_ActiveScene = CreateRef<Scene>();
        m_Square = m_ActiveScene->createEntity("Cait queen");
		

        m_Camera = m_ActiveScene->createEntity("Camera");
        m_Camera.addComponent<CameraComponent>(CameraComponent::Type::Orthographic, width, height);

		class CameraController : public ScriptableEntity {
			void onUpdate(TimeStep ts) override {
				auto& transform = getComponent<TransformComponent>().position;
				float speed = 5.0f;

				if (Input::isKeyPressed(KeyCode::A))
					transform.x-= speed * ts;
				if (Input::isKeyPressed(KeyCode::D))
					transform.x += speed * ts;
				if (Input::isKeyPressed(KeyCode::W))
					transform.y += speed * ts;
				if (Input::isKeyPressed(KeyCode::S))
					transform.y -= speed * ts;
			}
		};
		m_Camera.addComponent<NativeScriptComponent>().bind<CameraController>();

		m_CameraSecondary = m_ActiveScene->createEntity("Camera 2");
		m_CameraSecondary.addComponent<CameraComponent>(CameraComponent::Type::Orbit, width, height).primary = false;
		m_CameraSecondary.getComponent<TransformComponent>().position.z = 4.0f;
		m_CameraSecondary.addComponent<NativeScriptComponent>().bind<CameraController>();
        
        m_Square.addComponent<SpriteRendererComponent>(glm::vec4{0, 1, 0, 1});
		m_ActiveScene->createEntity("Square2").addComponent<SpriteRendererComponent>(glm::vec4{1, 0, 0, 1});

		m_sceneHierarchyPanel.setContext(m_ActiveScene);
	}

	void EditorLayer::onUpdate(TimeStep dt) {
   //      if (m_viewportFocused)
			// m_camera_controller.onUpdate(dt);

		// If viewports don't match recreate frame buffer
		if (m_ViewportSize != *((glm::vec2*)&m_viewportPanelSize) && m_viewportPanelSize.x > 0 && m_viewportPanelSize.y > 0) {
			m_ViewportSize = { m_viewportPanelSize.x, m_viewportPanelSize.y };
			buffer->resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);

			m_camera_controller.onResize(m_ViewportSize.x, m_ViewportSize.y);

			m_ActiveScene->onViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		}


        // Update Scene
        m_ActiveScene->onUpdate(dt);
	}

	void EditorLayer::onDraw() {

        buffer->bind();
        Renderer2D::Clear();

        {
            //Renderer2D::BeginScene(m_camera_controller.getCamera());
            // Draw Scene
            m_ActiveScene->onDraw();

            //Renderer2D::DrawQuad({ 0.5, 0.5 }, { 1, 1 }, Color::CYAN);
           // Renderer2D::EndScene();
        }

        buffer->unbind();
	}

    void EditorLayer::onImGuiRender() {
        static bool p_open = true;
        static bool opt_fullscreen = true;
        static bool opt_padding = false;
        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

		{
	        // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
	        // because it would be confusing to have two docking targets within each others.
	        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
	        if (opt_fullscreen)
	        {
	            const ImGuiViewport* viewport = ImGui::GetMainViewport();
	            ImGui::SetNextWindowPos(viewport->WorkPos);
	            ImGui::SetNextWindowSize(viewport->WorkSize);
	            ImGui::SetNextWindowViewport(viewport->ID);
	            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	            window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
	            window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
	        } else
	        {
	            dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
	        }

	        // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
	        // and handle the pass-thru hole, so we ask Begin() to not render a background.
	        if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
	            window_flags |= ImGuiWindowFlags_NoBackground;

	        // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
	        // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
	        // all active windows docked into it will lose their parent and become undocked.
	        // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
	        // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
	        if (!opt_padding)
	            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	        ImGui::Begin("DockSpace Demo", &p_open, window_flags);
	        if (!opt_padding)
	            ImGui::PopStyleVar();

	        if (opt_fullscreen)
	            ImGui::PopStyleVar(2);

	        // Submit the DockSpace
	        ImGuiIO& io = ImGui::GetIO();
			ImGuiStyle& style = ImGui::GetStyle();
			float minWinSize = style.WindowMinSize.x;
			style.WindowMinSize.x = 370.0f;

	        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
	        {
	            ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
	            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
	        }
			style.WindowMinSize.x = minWinSize;

	        if (ImGui::BeginMenuBar())
	        {
	            if (ImGui::BeginMenu("File"))
	            {
	                // Disabling fullscreen would allow the window to be moved to the front of other windows,
	                // which we can't undo at the moment without finer window depth/z control.
	                if (ImGui::MenuItem("Exit")) {
	                    Application::close();
	                }

	                ImGui::EndMenu();
	            }

	            ImGui::EndMenuBar();
	        }
		}

		{
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
			ImGui::Begin("Viewport");
			m_viewportFocused = ImGui::IsWindowFocused();
			m_viewportHovered = ImGui::IsWindowHovered();
			Application::get().getUILayer()->setBlockEvents(!m_viewportFocused || !m_viewportHovered);

			m_viewportPanelSize = ImGui::GetContentRegionAvail();


			uint32_t textureID = buffer->getColorAttachmentRendererID();
			ImGui::Image((void*)textureID, { m_ViewportSize.x, m_ViewportSize.y }, ImVec2(0, 1), ImVec2(1, 0));
			ImGui::End();
			ImGui::PopStyleVar();
		}

		m_sceneHierarchyPanel.onImGuiRender();
		
        ImGui::End();
	}

	void EditorLayer::onDestroy() {
	}

	void EditorLayer::onEvent(Event& event) {
        //m_camera_controller.onEvent(event);
	}
}
