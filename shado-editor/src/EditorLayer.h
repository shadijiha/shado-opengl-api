#include "Shado.h"
#include "panels/ConsolePanel.h"
#include "panels/ContentBrowserPanel.h"
#include "panels/SceneHierarchyPanel.h"
#include "panels/MemoryPanel.h"
#include <deque>

#include "panels/PrefabEditor.h"
#include "panels/SceneInfoPanel.h"

namespace Shado {

	class EditorLayer : public Layer {
	public:
		EditorLayer();
		~EditorLayer() override;
		void onInit() override;
		void onUpdate(TimeStep dt) override;
		void onDraw() override;
		void onImGuiRender() override;
		void onDestroy() override;
		void onEvent(Event& event) override;

		inline void pushUndoQueue() {
			// If the undo queue is full, pop the front element
			if (m_UndoQueue.size() >= 20)
				m_UndoQueue.pop_front();
			m_UndoQueue.push_back(CreateRef<Scene>(*m_EditorScene.Raw()));
		}

	private:
		bool onKeyPressed(KeyPressedEvent& e);
		bool onMouseButtonPressed(MouseButtonPressedEvent& e);

		void saveScene(const std::optional<std::filesystem::path>& path);
		void newScene();
		void openScene();
		void openScene(const std::filesystem::path&);

		void onScenePlay();
		void onSceneStop();
		void setActiveScene(Ref<Scene> scene);

		// UI Stuff
		void UI_ToolBar();
		void UI_Viewport();
		void UI_RendererStats();
		void UI_Settings();
	private:
		EditorCamera m_EditorCamera;

		bool m_viewportFocused = false, m_viewportHovered = false, m_lastFrameGuizmosIsUsing = false;
		glm::vec2 m_ViewportSize = {0, 0};
		Ref<Framebuffer> buffer;

		glm::vec2 m_ViewportBounds[2];
		ImVec2 m_viewportPanelSize;

		// Scenes
		Ref<Scene> m_ActiveScene;
		Ref<Scene> m_EditorScene;
		std::deque<Ref<Scene>> m_UndoQueue;

		int m_GuizmosOperation = -1;
		std::string m_ScenePath;

		// Panels
		SceneHierarchyPanel m_sceneHierarchyPanel;
		ContentBrowserPanel m_ContentPanel;
		ConsolePanel m_ConsolPanel;
		MemoryPanel m_MemoryPanel;
		PrefabEditor m_PrefabEditor;
		SceneInfoPanel m_SceneInfoPanel;
		Entity m_HoveredEntity;

		// This is needed because on fresh install when no Imgui.ini, we need to be able to move the play button to its location
		bool m_UiToolbarMovable = false;	

		// Scene runtime / editing
		enum class SceneState
		{
			Edit = 0, Play = 1
		};
		SceneState m_SceneState = SceneState::Edit;

		// Editor resources
		Ref<Texture2D> m_IconPlay, m_IconStop;
	};
}
