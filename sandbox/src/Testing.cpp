#include "Shado.h"
#include <iostream>

#include "script/LuaScript.h"

using namespace Shado;

float map(float x, float in_min, float in_max, float out_min, float out_max)
{
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

class TestScene : public Scene {
public:
	TestScene() :
		Scene("Test scene"),
		camera(Application::get().getWindow().getAspectRatio()),
		orthoCamera(Application::get().getWindow().getAspectRatio())
	{
		
	}

	virtual ~TestScene() {
		delete script;
	}

	void onInit() override {
		script = new LuaScript(*this, luaScriptFile);
		/*EntityDefinition def;
		def.type = EntityType::DYNAMIC;
		def.texture = riven2;
		def.scale = { 0.25, 0.25 };

		for (float y = -1.5f; y < 1.5f; y += 0.3f)
			for (float x = -1.5f; x < 1.5f; x += 0.3f) {
				def.position = { x, y, -2 };
				objects.push_back({def, world});
			}
		*/
		script->onCreate();

		EntityDefinition def2;
		def2.type = EntityType::DYNAMIC;
		def2.scale = { 0.45f, 0.45f };

		for (float y = -5.0f; y < 5.0f; y += 0.5f)
		{
			for (float x = -5.0f; x < 5.0f; x += 0.5f)
			{
				def2.color = { (x + 5.0f) / 10.0f, 0.4f, (y + 5.0f) / 10.0f, 0.7f };
				def2.position = { x, y, -15 };
				objects.push_back(this->addEntityToWorld(def2));
			}
		}

	}

	void onUpdate(TimeStep dt) override {
		camera.onUpdate(dt);
		orthoCamera.onUpdate(dt);
		script->onUpdate(dt);
	}

	void onDraw() override {

		Renderer2D::BeginScene(orthoCamera.getCamera());

		/*for (const auto* object : objects) {
			object->draw();
		}*/

		for (const Entity* entity : getAllEntities()) {
			entity->draw();
		}

		Renderer2D::EndScene();
	}

	void onDestroy() override {
		script->onDestoy();
	}

	void onEvent(Event& e) override {

		camera.onEvent(e);
		orthoCamera.onEvent(e);
		script->onEvent(e);
	}

	void onImGuiRender() override {
		ImGui::SliderFloat4("Sphere Colour", (float*)&color, 0, 1.0f);
		ImGui::SliderInt("Tiling factor", &tileFactor, 1, 100);

		if (ImGui::Button("Reload script")) {
			script->onDestoy();
			delete script;

			script = new LuaScript(*this, luaScriptFile);
			script->onCreate();
		}

		if (ImGui::Button("Clear entities")) {
			for (Entity* e : entities) {
				e->destroy();
				delete e;
			}
			entities.clear();
		}
	}

private:
	Ref<Texture2D> riven1 = CreateRef<Texture2D>("assets/riven3.png");
	Ref<Texture2D> riven2 = CreateRef<Texture2D>("assets/riven2.jpg");

	std::vector<Entity*> objects;

	OrbitCameraController camera;
	OrthoCameraController orthoCamera;
	Color color = { 1, 1, 1 };
	int tileFactor = 1;

	std::string luaScriptFile = "test.lua";
	LuaScript* script;
};

class TestScene2 : public Scene {
public:
	TestScene2() :
		Scene("Test scene2"),
		camera(Application::get().getWindow().getAspectRatio())
	{
	}

	void onUpdate(TimeStep dt) override {
		camera.onUpdate(dt);
	}

	void onDraw() override {
		Renderer2D::BeginScene(camera.getCamera());

		Renderer2D::DrawQuad({ 0, 0, 0 }, { 1, 1 }, { 1, 1, 1, 1 });
		Renderer2D::DrawRotatedQuad({ 0, 0, -3 }, { 2, 2 }, 45.0f, { 0.3, 0.614, 0.21, 1.0 });

		Renderer2D::EndScene();
	}

	void onEvent(Event& e) override {
		camera.onEvent(e);

		EventDispatcher dispatcher(e);
		dispatcher.dispatch<KeyPressedEvent>([&](KeyPressedEvent& event) {

			if (event.getKeyCode() == SHADO_KEY_B) {
				Application::get().setActiveScene("Test scene");
			}

			return false;
			});
	}

	void onImGuiRender() override {
		ImGui::TextColored({ 0.5, 0.5, 0, 1 }, "LULW");
	}

	OrbitCameraController camera;
};


int main(int argc, const char** argv)
{
	auto& application = Application::get();
	application.getWindow().resize(1920, 1080);
	application.submit(new TestScene);
	application.submit(new TestScene2);
	application.run();

	Application::destroy();
}
