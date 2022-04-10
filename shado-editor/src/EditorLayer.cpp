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
	}

	void EditorLayer::onUpdate(TimeStep dt) {
        if (m_viewportFocused)
			m_camera_controller.onUpdate(dt);
	}

	void EditorLayer::onDraw() {
        buffer->bind();
        Renderer2D::Clear();
        Renderer2D::BeginScene(m_camera_controller.getCamera());

        Renderer2D::DrawQuad({ 0.5, 0.5 }, { 1, 1 }, Color::CYAN);

        Renderer2D::EndScene();
        buffer->unbind();
	}

	void EditorLayer::onImGuiRender() {
        static bool p_open = true;
        static bool opt_fullscreen = true;
        static bool opt_padding = false;
        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

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
        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
        {
            ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
        }

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

        {
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
            ImGui::Begin("Viewport");
            m_viewportFocused = ImGui::IsWindowFocused();
            m_viewportHovered = ImGui::IsWindowHovered();
            Application::get().getUILayer()->setBlockEvents(!m_viewportFocused || !m_viewportHovered);
            
            ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();

            // If viewports don't match recreate frame buffer
            if (m_ViewportSize != *((glm::vec2*)&viewportPanelSize)  && viewportPanelSize.x > 0 && viewportPanelSize.y > 0) {
                m_ViewportSize = { viewportPanelSize.x, viewportPanelSize.y };
                buffer->resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);

                m_camera_controller.onResize(m_ViewportSize.x, m_ViewportSize.y);
            }

            uint32_t textureID = buffer->getColorAttachmentRendererID();
            ImGui::Image((void*)textureID, { m_ViewportSize.x, m_ViewportSize.y }, ImVec2(0, 1), ImVec2(1, 0));
            ImGui::End();
            ImGui::PopStyleVar();
        }


        ImGui::Begin("Test");
        ImGui::Text("hehexd");
        ImGui::End();

        ImGui::End();
	}

	void EditorLayer::onDestroy() {
	}

	void EditorLayer::onEvent(Event& event) {
        m_camera_controller.onEvent(event);
	}
}
