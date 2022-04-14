#include "Shado.h"
#include <iostream>

using namespace Shado;

class TestLayer : public Layer {
public:
	TestLayer() :
		Layer("layer1"),
		camera(Application::get().getWindow().getAspectRatio()),
		orthoCamera(Application::get().getWindow().getAspectRatio())
	{
	}
	
	virtual ~TestLayer() {}

	void onInit() override {
		FramebufferSpecification specification;
		specification.Attachments = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::DEPTH24STENCIL8 };
		specification.Width = Application::get().getWindow().getWidth();
		specification.Height = Application::get().getWindow().getHeight();
		//buffer = Framebuffer::create(specification);

		Renderer2D::SetClearColor({ 0, 0, 0, 1 });
	}

	void onUpdate(TimeStep dt) override {
		camera.onUpdate(dt);
		orthoCamera.onUpdate(dt);
	}
	 
	void onDraw() override {
		//buffer->bind();
		Renderer2D::Clear();
		Renderer2D::BeginScene(orthoCamera.getCamera());

		for (float y = -5.0f; y < 5.0f; y += 0.5f)
		{
			for (float x = -5.0f; x < 5.0f; x += 0.5f)
			{
				glm::vec4 color = { (x + 5.0f) / 10.0f, 0.4f, (y + 5.0f) / 10.0f, 0.7f };
				glm::vec3 position = { x, y, -15 };
				Renderer2D::DrawQuad(position, glm::vec2{ 0.45f, 0.45f }, color);
			}
		}

		Renderer2D::EndScene();
		//buffer->unbind();
	}

	void onDestroy() override {}

	void onEvent(Event& e) override {

		camera.onEvent(e);
		orthoCamera.onEvent(e);

		EventDispatcher dispatcher(e);
		dispatcher.dispatch<KeyPressedEvent>([&](KeyPressedEvent& event) {

			if (event.getKeyCode() == KeyCode::Escape) {
				Application::get().getWindow().setMode(WindowMode::WINDOWED);
			}

			if (event.getKeyCode() == KeyCode::F) {
				Application::get().getWindow().setMode(WindowMode::FULLSCREEN);
			}

			return false;
		});


		dispatcher.dispatch<MouseButtonPressedEvent>([&](MouseButtonPressedEvent& event) {
			Window& win = Application::get().getWindow();
			return false;
		});
	}

	void onImGuiRender() override {
		ImGui::Begin("Hehexd");
		ImGui::Text("hehexd");
		
		/*ImGui::SliderFloat4("Sphere Colour", (float*)&color, 0, 1.0f);
		ImGui::SliderInt("Tiling factor", &tileFactor, 1, 100);

		uint32_t textureID = buffer->getColorAttachmentRendererID();
		ImGui::Image((void*)textureID, {1280.0f, 720.0f}, ImVec2(0, 1), ImVec2(1, 0));*/

		ImGui::End();
	}

private:
	Ref<Texture2D> riven1 = CreateRef<Texture2D>("assets/riven3.png");
	Ref<Texture2D> riven2 = CreateRef<Texture2D>("assets/riven2.jpg");

	OrbitCameraController camera;
	OrthoCameraController orthoCamera;
	Color color = { 1, 1, 1 };
	int tileFactor = 1;

	//Ref<Framebuffer> buffer;
};

int main(int argc, const char** argv)
{
	auto& application = Application::get();
	application.getWindow().resize(1920, 1080);
	application.submit(new TestLayer);
	application.run();

	Application::destroy();
}
