#include "FrameBuffer.h"
#include "Layer.h"
#include "cameras/OrthoCamera.h"

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
		OrthoCameraController m_camera_controller;

		glm::vec2 m_ViewportSize = {0, 0};
		Ref<FrameBuffer> buffer;
	};


}
