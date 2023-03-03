#include "EditorLayer.h"

#include <iostream>
#include <glm/gtc/type_ptr.hpp>

#include "Application.h"
#include "scene/SceneSerializer.h"
#include "scene/utils/SceneUtils.h"
#include "ImGuizmo/ImGuizmo.h"
#include "math/Math.h"
#include "ui/UI.h"
#include "project/Project.h"
#include "script/ScriptEngine.h"

namespace Shado {
	static TimeStep lastDt = 1 / 60.0f;

	EditorLayer::EditorLayer()
		: Layer("Editor")
	{
		Application::get().getWindow().setTitle("Shado Engine Editor");
	}

    EditorLayer::~EditorLayer()
	{
	}

	void EditorLayer::onInit() {
		SHADO_PROFILE_FUNCTION();

		m_IconPlay = CreateRef<Texture2D>("resources/icons/PlayButton.png");
		m_IconStop = CreateRef<Texture2D>("resources/icons/StopButton.png");

        FramebufferSpecification specs;
		specs.Attachments = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::RED_INTEGER,  FramebufferTextureFormat::DEPTH24STENCIL8 };
        specs.Width = Application::get().getWindow().getWidth();
        specs.Height = Application::get().getWindow().getHeight();
        buffer = Framebuffer::create(specs);

        Renderer2D::SetClearColor({ 0.1, 0.1, 0.1, 1 });

        m_ActiveScene = CreateRef<Scene>();
		m_EditorScene = m_ActiveScene;
		Scene::ActiveScene = m_ActiveScene;

		m_sceneHierarchyPanel.setContext(m_ActiveScene);
	}

	void EditorLayer::onUpdate(TimeStep dt) {
		lastDt = dt;

		SHADO_PROFILE_FUNCTION();

		// If viewports don't match recreate frame buffer
		if (m_ViewportSize != *((glm::vec2*)&m_viewportPanelSize) && m_viewportPanelSize.x > 0 && m_viewportPanelSize.y > 0) {
			m_ViewportSize = { m_viewportPanelSize.x, m_viewportPanelSize.y };
			buffer->resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);

			m_EditorCamera.SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);

			m_ActiveScene->onViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		}

		// Update Scene
		switch (m_SceneState)
		{
			case SceneState::Edit:
				if (m_viewportFocused)
					m_EditorCamera.OnUpdate(dt);

				m_ActiveScene->onUpdateEditor(dt, m_EditorCamera);
				break;
			case SceneState::Play:
				m_ActiveScene->onUpdateRuntime(dt);
				break;
		}
	}

	void EditorLayer::onDraw() {
		SHADO_PROFILE_FUNCTION();

		 Renderer2D::ResetStats();
         buffer->bind();
         Renderer2D::Clear();
		 buffer->clearAttachment(1, -1);

		 switch (m_SceneState)
		 {
			 case SceneState::Edit:
				 m_ActiveScene->onDrawEditor(m_EditorCamera);
				 break;
			 case SceneState::Play:
				 m_ActiveScene->onDrawRuntime();
				 break;
		 }

        
		// For mouse picking
		auto [mx, my] = ImGui::GetMousePos();
		mx -= m_ViewportBounds[0].x;
		my -= m_ViewportBounds[0].y;
		glm::vec2 viewportSize = m_ViewportBounds[1] - m_ViewportBounds[0];
		my = viewportSize.y - my;
		int mouseX = (int)mx;
		int mouseY = (int)my;

		if (mouseX >= 0 && mouseY >= 0 && mouseX < (int)viewportSize.x && mouseY < (int)viewportSize.y)
		{
			int pixelData = buffer->readPixel(1, mouseX, mouseY);
			m_HoveredEntity = pixelData == -1 ? Entity() : Entity{ (entt::entity)(uint32_t)pixelData, m_ActiveScene.get()};
		}
		
        buffer->unbind();
	}

    void EditorLayer::onImGuiRender() {
		SHADO_PROFILE_FUNCTION();

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
				if (ImGui::BeginMenu("File")) {

					if (ImGui::MenuItem("New Project", "Ctrl+P+N")) {
						Ref<Project> project = Project::New();
						project->GetConfig().Name = "Test123";
						project->GetConfig().StartScene = "sample.shadoscene";
					}

					if (ImGui::MenuItem("Open Project...", "Ctrl+P+O")) {
						std::string path = FileDialogs::openFile("Shado Project (*.sproj)\0*.sproj\0");

						if (!path.empty() && Project::Load(path))
						{
							ScriptEngine::Init();

							auto startScenePath = Project::GetAssetFileSystemPath(Project::GetActive()->GetConfig().StartScene);
							openScene(startScenePath);
							m_ContentPanel = ContentBrowserPanel();

						}
					}

					if (Project::GetActive() && ImGui::MenuItem("Save Project...", "Ctrl+P+S")) {
						//std::string path = FileDialogs::chooseFolder();
						std::string path = std::filesystem::absolute("resources/projects/Test 123").string();
						auto& config = Project::GetActive()->GetConfig();
						config.AssetDirectory = path + "/Assets";
						config.ScriptModulePath = "bin/Release-windows-x86_64/" + config.Name + "/" + config.Name + ".dll";

						Project::SaveActive(path + "/Test 123.sproj");
					}

					// Disabling fullscreen would allow the window to be moved to the front of other windows,
					// which we can't undo at the moment without finer window depth/z control.
					if (ImGui::MenuItem("Exit")) {
						Application::close();
					}

					ImGui::EndMenu();
				}

	            if (Project::GetActive() && ImGui::BeginMenu("Scene"))
	            {
					// New scene
					if (ImGui::MenuItem("New", "Ctrl+N")) {
						newScene();
					}

					// Open Scene file
					if (ImGui::MenuItem("Open...", "Ctrl+O")) {
						openScene();
					}

					if (ImGui::MenuItem("Save", "Ctrl+S"))
						saveScene();

					// Save scene file
					if (ImGui::MenuItem("Save As...")) {
						m_ScenePath = "";
						saveScene();
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
		
		// UI
		UI_Viewport();
		UI_ToolBar();
		UI_RendererStats();
		UI_Settings();

		m_sceneHierarchyPanel.onImGuiRender();
		m_ContentPanel.onImGuiRender();
		m_ConsolPanel.onImGuiRender();

        ImGui::End();
	}

	void EditorLayer::onDestroy() {
	}

	void EditorLayer::onEvent(Event& event) {
        m_EditorCamera.OnEvent(event);
		m_sceneHierarchyPanel.onEvent(event);

		EventDispatcher dispatcher(event);
		dispatcher.dispatch<KeyPressedEvent>(SHADO_BIND_EVENT_FN(EditorLayer::onKeyPressed));
		dispatcher.dispatch<MouseButtonPressedEvent>(SHADO_BIND_EVENT_FN(EditorLayer::onMouseButtonPressed));	
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
				if (control)
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

			// Entity
			case KeyCode::D:
				if (control)
					m_ActiveScene->duplicateEntity(m_sceneHierarchyPanel.getSelected());
				break;
			case KeyCode::Delete:
				m_ActiveScene->destroyEntity(m_sceneHierarchyPanel.getSelected());
				m_sceneHierarchyPanel.resetSelection();
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
		return false;
	}

	bool EditorLayer::onMouseButtonPressed(MouseButtonPressedEvent& e) {
		if (e.getMouseButton() == GLFW_MOUSE_BUTTON_LEFT && m_viewportHovered && !ImGuizmo::IsOver()) {
			m_sceneHierarchyPanel.setSelected(m_HoveredEntity);
		}
		return false;
	}

	void EditorLayer::saveScene() {
		if (m_SceneState == SceneState::Play) {
			Dialog::alert("Cannot save a scene while playing", "Save Error");
			return;
		}

		auto filepath = m_ScenePath.empty() ? FileDialogs::saveFile("Shado Scene(*.shadoscene)\0*.shadoscene\0") : m_ScenePath;
		
		if (!filepath.empty()) {
			SceneSerializer serializer(m_EditorScene);
			serializer.serialize(filepath);
			m_ScenePath = filepath;
		}		
	}

	void EditorLayer::newScene() {
		Ref<Scene> scene = CreateRef<Scene>();
		scene->onViewportResize(m_ViewportSize.x, m_ViewportSize.y);
		m_ActiveScene = scene;
		m_EditorScene = m_ActiveScene;
		m_sceneHierarchyPanel.setContext(scene);
		m_ScenePath = "";
		Scene::ActiveScene = m_ActiveScene;
	}

	void EditorLayer::openScene() {
		auto filepath = FileDialogs::openFile("Shado Scene(*.shadoscene)\0*.shadoscene\0");
		if (!filepath.empty()) {
			openScene(filepath);
		}
	}

	void EditorLayer::openScene(const std::filesystem::path& path) {
		if (m_SceneState != SceneState::Edit)
			onSceneStop();

		if (path.extension().string() != ".shadoscene") {
			std::string message = "Could not load scene file " + path.string();
			SHADO_WARN(message);
			Dialog::alert(message, "Scene load error", Dialog::DialogIcon::ERROR_ICON);
			return;
		}

		Ref<Scene> scene = CreateRef<Scene>();
		scene->onViewportResize(m_ViewportSize.x, m_ViewportSize.y);

		SceneSerializer serializer(scene);
		std::string errorMsg;
		if (!serializer.deserialize(path.string(), errorMsg)) {
			Dialog::alert("Unable to load file " + path.string() + ". " + errorMsg, "Error loading scene", Dialog::DialogIcon::ERROR_ICON);
			return;
		}

		m_EditorScene = scene;
		m_ActiveScene = m_EditorScene;
		m_sceneHierarchyPanel.setContext(scene);
		m_ScenePath = path.string();
		Scene::ActiveScene = m_ActiveScene;
	}

	// ============================== For runtime
	void EditorLayer::onScenePlay() {
		m_SceneState = SceneState::Play;
		m_ActiveScene = CreateRef<Scene>(*m_EditorScene.get());
		m_sceneHierarchyPanel.setContext(m_ActiveScene);
		Scene::ActiveScene = m_ActiveScene;
		//m_sceneHierarchyPanel.setContext(m_ActiveScene); // TODO maybe uncomment this
		m_ActiveScene->onRuntimeStart();
	}

	void EditorLayer::onSceneStop() {
		m_SceneState = SceneState::Edit;
		m_ActiveScene->onRuntimeStop();
		m_ActiveScene = m_EditorScene;
		Scene::ActiveScene = m_ActiveScene;
		m_sceneHierarchyPanel.setContext(m_ActiveScene);
	}

	// =============================== UI Stuff
	void EditorLayer::UI_ToolBar() {
		SHADO_PROFILE_FUNCTION();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 2));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0, 0));
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
		auto& colors = ImGui::GetStyle().Colors;
		const auto& buttonHovered = colors[ImGuiCol_ButtonHovered];
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(buttonHovered.x, buttonHovered.y, buttonHovered.z, 0.5f));
		const auto& buttonActive = colors[ImGuiCol_ButtonActive];
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(buttonActive.x, buttonActive.y, buttonActive.z, 0.5f));

		ImGui::Begin("##toolbar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

		float size = ImGui::GetWindowHeight() - 4.0f;
		Ref<Texture2D> icon = m_SceneState == SceneState::Edit ? m_IconPlay : m_IconStop;
		ImGui::SetCursorPosX((ImGui::GetWindowContentRegionMax().x * 0.5f) - (size * 0.5f));
		if (ImGui::ImageButton((ImTextureID)icon->getRendererID(), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1), 0))
		{
			if (m_SceneState == SceneState::Edit)
				onScenePlay();
			else if (m_SceneState == SceneState::Play)
				onSceneStop();
		}
		ImGui::PopStyleVar(2);
		ImGui::PopStyleColor(3);
		ImGui::End();
	}

	void EditorLayer::UI_Viewport() {
		SHADO_PROFILE_FUNCTION();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
		ImGui::Begin("Viewport");
		auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
		auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
		auto viewportOffset = ImGui::GetWindowPos();
		m_ViewportBounds[0] = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
		m_ViewportBounds[1] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };

		m_viewportFocused = ImGui::IsWindowFocused();
		m_viewportHovered = ImGui::IsWindowHovered();
		Application::get().getUILayer()->setBlockEvents(!m_viewportFocused && !m_viewportHovered);

		m_viewportPanelSize = ImGui::GetContentRegionAvail();

		uint32_t textureID = buffer->getColorAttachmentRendererID();
		ImGui::Image((void*)textureID, { m_ViewportSize.x, m_ViewportSize.y }, ImVec2(0, 1), ImVec2(1, 0));

		// For drag and drop
		if (ImGui::BeginDragDropTarget()) {

			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM")) {
				const wchar_t* pathStr = (const wchar_t*)payload->Data;
				auto path = Project::GetActive()->GetProjectDirectory() / pathStr;
				auto extension = path.extension();

				if (extension == ".jpg" || extension == ".png") {
					if (m_HoveredEntity && m_HoveredEntity.hasComponent<SpriteRendererComponent>())
						m_HoveredEntity.getComponent<SpriteRendererComponent>().texture = CreateRef<Texture2D>(path.string());

					else if (m_HoveredEntity && m_HoveredEntity.hasComponent<CircleRendererComponent>())
						m_HoveredEntity.getComponent<CircleRendererComponent>().texture = CreateRef<Texture2D>(path.string());
				} else
					openScene(path);
			}

			ImGui::EndDragDropTarget();
		}

		// Gizmos
		Entity selected = m_sceneHierarchyPanel.getSelected();
		if (selected && m_GuizmosOperation != -1) {
			// Camera
			glm::mat4 projection, cameraView;

			if (m_SceneState == SceneState::Edit) {

				projection = m_EditorCamera.getProjectionMatrix();
				cameraView = m_EditorCamera.getViewMatrix();
				ImGuizmo::SetOrthographic(false);
			}
			else {
				auto runtimeCamera = m_ActiveScene->getPrimaryCameraEntity();
				auto cameraComponent = runtimeCamera.getComponent<CameraComponent>();

				projection = cameraComponent.camera->getProjectionMatrix();
				cameraView = glm::inverse(runtimeCamera.getComponent<TransformComponent>().getTransform());
				// Disable or enable orthographic depending on if we are using Ortho camera or not
				ImGuizmo::SetOrthographic(cameraComponent.type == CameraComponent::Type::Orthographic);
			}
			
			ImGuizmo::SetDrawlist();

			ImGuizmo::SetRect(m_ViewportBounds[0].x, m_ViewportBounds[0].y, m_ViewportBounds[1].x - m_ViewportBounds[0].x, m_ViewportBounds[1].y - m_ViewportBounds[0].y);

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
				// Pause physics if we are using gizmos
				m_ActiveScene->enablePhysics(false);
				m_ActiveScene->softResetPhysics();

				glm::vec3 position, rotation, scale;
				Math::decomposeTransform(transform, position, rotation, scale);

				auto deltaRotation = rotation - tc.rotation;	// To avoid gimbull lock
				tc.position = transform[3];
				tc.rotation += deltaRotation;
				tc.scale = scale;
			}
			else {
				// If Gizmos are not in use then resume physics
				m_ActiveScene->enablePhysics(true);
			}
		}

		ImGui::End();
		ImGui::PopStyleVar();
	}

	void EditorLayer::UI_RendererStats() {

		ImGui::Begin("Renderer stats");

		auto stats = Renderer2D::GetStats();

		ImGui::Text("Draw calls: %d", stats.DrawCalls);
		ImGui::Text("Quads count: %d", stats.QuadCount);
		ImGui::Text("Lines calls: %d", stats.LineCount);
		ImGui::Text("Total indices: %d", stats.GetTotalVertexCount());
		ImGui::Text("Total vertices: %d", stats.GetTotalVertexCount());
		ImGui::NewLine();
		ImGui::Text("FPS: %d", (int)lastDt.toFPS());
		ImGui::End();
	}

	void EditorLayer::UI_Settings() {

		ImGui::Begin("Settings");

		static bool CPUZSorting = true;
		static bool VSync = Application::get().getWindow().isVSync();

		if (ImGui::Checkbox("Enable CPU alpha Z sorting", &CPUZSorting)) {
			Renderer2D::setCPUAlphaZSorting(CPUZSorting);
		}

		if (ImGui::Checkbox("VSync", &VSync)) {
			Application::get().getWindow().setVSync(VSync);
		}

		static std::string currentType = "";
		UI::DropDown("Window mode", {
			{ "", []() { }  },
			{ "Windowed", []() { Application::get().getWindow().setMode(WindowMode::WINDOWED); }  },
			{ "Borderless", []() { Application::get().getWindow().setMode(WindowMode::BORDERLESS_WINDOWED); }  },
			{ "Fullscreen", []() { Application::get().getWindow().setMode(WindowMode::FULLSCREEN); }  },
		}, currentType);

		static float windowOpacity = 1.0;
		if (ImGui::DragFloat("Opacity", &windowOpacity, 0.01, 0.1f, 1.0f))
			Application::get().getWindow().setOpacity(windowOpacity);

		ImGui::End();
	}
}
