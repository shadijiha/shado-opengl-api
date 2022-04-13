#include "EditorLayer.h"

#include <glm/gtc/type_ptr.hpp>

#include "Application.h"
#include "scene/SceneSerializer.h"
#include "scene/utils/SceneUtils.h"
#include "ImGuizmo/ImGuizmo.h"
#include "math/Math.h"

namespace Shado {
	static void saveScene();
	static void newScene();
	static void openScene();

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

        m_ActiveScene = CreateRef<Scene>();

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

			// MENU BAR
	        if (ImGui::BeginMenuBar())
	        {
	            if (ImGui::BeginMenu("File"))
	            {
					// New scene
					if (ImGui::MenuItem("New", "Ctrl+N")) {
						newScene();
					}

					// Open Scene file
					if (ImGui::MenuItem("Open", "Ctrl+O")) {
						openScene();
					}

					// Save scene file
					if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S")) {
						saveScene();
					}

	                // Disabling fullscreen would allow the window to be moved to the front of other windows,
	                // which we can't undo at the moment without finer window depth/z control.
	                if (ImGui::MenuItem("Exit")) {
	                    Application::close();
	                }

	                ImGui::EndMenu();
	            }

				if (ImGui::BeginMenu("Gizmos")) {

					if (ImGui::MenuItem("Translation")) {
						m_GuizmosOperation = ImGuizmo::OPERATION::TRANSLATE;
					}

					if (ImGui::MenuItem("Rotation")) {
						m_GuizmosOperation = ImGuizmo::OPERATION::ROTATE;
					}

					if (ImGui::MenuItem("Scale")) {
						m_GuizmosOperation = ImGuizmo::OPERATION::SCALE;
					}

					if (ImGui::MenuItem("None")) {
						m_GuizmosOperation = -1;
					}

					ImGui::EndMenu();
				}

	            ImGui::EndMenuBar();
	        }
		}

		// Viewport
		{
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
			ImGui::Begin("Viewport");
			m_viewportFocused = ImGui::IsWindowFocused();
			m_viewportHovered = ImGui::IsWindowHovered();
			Application::get().getUILayer()->setBlockEvents(!m_viewportFocused || !m_viewportHovered);

			m_viewportPanelSize = ImGui::GetContentRegionAvail();

			uint32_t textureID = buffer->getColorAttachmentRendererID();
			ImGui::Image((void*)textureID, { m_ViewportSize.x, m_ViewportSize.y }, ImVec2(0, 1), ImVec2(1, 0));


			// Gizmos
			Entity selected = m_sceneHierarchyPanel.getSelected();
			if (selected && m_GuizmosOperation != -1) {
				ImGuizmo::SetOrthographic(false);
				ImGuizmo::SetDrawlist();

				float windowWidth = ImGui::GetWindowWidth();
				float windowHeight = ImGui::GetWindowHeight();
				ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, windowWidth, windowHeight);

				// Camera
				auto cameraEntity = m_ActiveScene->getPrimaryCameraEntity();
				const auto& camera = cameraEntity.getComponent<CameraComponent>().camera;
				const auto& projection = camera->getProjectionMatrix();
				glm::mat4 cameraView = glm::inverse(cameraEntity.getComponent<TransformComponent>().getTransform());

				// Entity transform
				auto& tc = selected.getComponent<TransformComponent>();
				auto transform = tc.getTransform();

				// Snapping
				bool snap = Input::isKeyPressed(KeyCode::LeftControl);
				float snapValue = m_GuizmosOperation == ImGuizmo::OPERATION::ROTATE ? 45.0f : 0.5f;	// Snap to 45 degrees for rotation, and 0.5m for scale and translation
				float snapValues[3] = { snapValue, snapValue, snapValue };


				ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(projection),
					(ImGuizmo::OPERATION)m_GuizmosOperation, ImGuizmo::LOCAL, glm::value_ptr(transform), nullptr, snap ? snapValues : nullptr);

				if (ImGuizmo::IsUsing()) {
					glm::vec3 position, rotation, scale;
					Math::decomposeTransform(transform, position, rotation, scale);

					auto deltaRotation = rotation -  tc.rotation;	// To avoid gimbull lock
					tc.position = transform[3];
					tc.rotation += deltaRotation;
					tc.scale = scale;
				}
			}

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

		EventDispatcher dispatcher(event);
		dispatcher.dispatch<KeyPressedEvent>(SHADO_BIND_EVENT_FN(EditorLayer::onKeyPressed));
	}

	// Helpers
	bool EditorLayer::onKeyPressed(KeyPressedEvent& e) {
		// For shortcuts
		if (e.getRepeatCount() > 0)
			return false;

		bool control = Input::isKeyPressed(KeyCode::LeftControl) || Input::isKeyPressed(KeyCode::RightControl);
		bool shift = Input::isKeyPressed(KeyCode::LeftShift) || Input::isKeyPressed(KeyCode::RightShift);

		switch (e.getKeyCode()) {
			case KeyCode::S:
				if (control && shift)
					saveScene();
				break;
			case KeyCode::N:
				if (control)
					newScene();
				break;
			case KeyCode::O:
				if (control)
					openScene();
				break;

			// Gizmos
			case KeyCode::Q:
				m_GuizmosOperation = -1;
				break;
			case KeyCode::W:
				m_GuizmosOperation = ImGuizmo::OPERATION::TRANSLATE;
				break;
			case KeyCode::E:
				m_GuizmosOperation = ImGuizmo::OPERATION::ROTATE;
				break;
			case KeyCode::R:
				m_GuizmosOperation = ImGuizmo::OPERATION::SCALE;
				break;
		}
	}

	void EditorLayer::saveScene() {
		auto filepath = FileDialogs::saveFile("Shado Scene(*.shadoscene)\0*.shadoscene\0");
		if (!filepath.empty()) {
			SceneSerializer serializer(m_ActiveScene);
			serializer.serialize(filepath);
		}		
	}

	void EditorLayer::newScene() {
		Ref<Scene> scene = CreateRef<Scene>();
		scene->onViewportResize(m_ViewportSize.x, m_ViewportSize.y);
		m_ActiveScene = scene;
		m_sceneHierarchyPanel.setContext(scene);
	}

	void EditorLayer::openScene() {
		auto filepath = FileDialogs::openFile("Shado Scene(*.shadoscene)\0*.shadoscene\0");

		if (!filepath.empty()) {
			Ref<Scene> scene = CreateRef<Scene>();
			scene->onViewportResize(m_ViewportSize.x, m_ViewportSize.y);

			SceneSerializer serializer(scene);
			serializer.deserialize(filepath);

			m_ActiveScene = scene;
			m_sceneHierarchyPanel.setContext(scene);
		}
	}
}
