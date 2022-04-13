#include "Shado.h"
#include "panels/ContentBrowserPanel.h"
#include "panels/SceneHierarchyPanel.h"

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

	
	private:
		bool onKeyPressed(KeyPressedEvent& e);
		bool onMouseButtonPressed(MouseButtonPressedEvent& e);

		void saveScene();
		void newScene();
		void openScene();
		void openScene(const std::filesystem::path&);
	private:
		EditorCamera m_EditorCamera;

		bool m_viewportFocused = false, m_viewportHovered = false;
		glm::vec2 m_ViewportSize = {0, 0};
		Ref<Framebuffer> buffer;

		glm::vec2 m_ViewportBounds[2];
		ImVec2 m_viewportPanelSize;
		Ref<Scene> m_ActiveScene;

		int m_GuizmosOperation = -1;

		// Panels
		SceneHierarchyPanel m_sceneHierarchyPanel;
		ContentBrowserPanel m_ContentPanel;
		Entity m_HoveredEntity;
	};
}
