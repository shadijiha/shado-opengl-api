#include "ImguiScene.h"
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include "Application.h"
#include "ImGuizmo.h"

namespace Shado {

	ImguiLayer::ImguiLayer(bool showDemo)
		: Layer("ImGui Scene"), m_ShowDemo(showDemo)
	{}
	ImguiLayer::~ImguiLayer() {}

	void ImguiLayer::onInit() {
		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui::SetAllocatorFunctions(
			[](size_t sz, void* data) { return Memory::HeapRaw(sz, "ImGui"); },
			[](void* ptr, void* data) { return Memory::FreeRaw(ptr, "ImGui"); }
		);
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
		//io.ConfigViewportsNoAutoMerge = true;
		//io.ConfigViewportsNoTaskBarIcon = true;

		io.Fonts->AddFontFromFileTTF("assets/fonts/Open_Sans/OpenSans-Bold.ttf", 18.0f);
		io.FontDefault = io.Fonts->AddFontFromFileTTF("assets/fonts/Open_Sans/OpenSans-Regular.ttf", 18.0f);
		io.FontGlobalScale = Application::get().getWindow().getWindowScale().first;
		ImGui::GetStyle().ScaleAllSizes(Application::get().getWindow().getWindowScale().first);

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();
		//ImGui::StyleColorsClassic();

		// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		setDarkThemeColors();

		// Setup Platform/Renderer backends
		ImGui_ImplGlfw_InitForOpenGL(Application::get().getWindow().getNativeWindow(), true);
		ImGui_ImplOpenGL3_Init("#version 410");

		Application& app = Application::get();
		io.DisplaySize = ImVec2(app.getWindow().getWidth(), app.getWindow().getHeight());
	}

	void ImguiLayer::onDestroy() {
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void ImguiLayer::onUpdate(TimeStep dt) {}

	void ImguiLayer::onEvent(Event& event) {
		if (m_BlockEvents) {
			bool handled = event.isHandled();
			ImGuiIO& io = ImGui::GetIO();
			handled |= event.isInCategory(EventCategoryMouse) & io.WantCaptureMouse;
			handled |= event.isInCategory(EventCategoryKeyboard) & io.WantCaptureKeyboard;

			event.setHandled(handled);
		}
	}

	void ImguiLayer::onImGuiRender() {
		if (m_ShowDemo)
			ImGui::ShowDemoWindow(&m_ShowDemo);
	}

	void ImguiLayer::begin() {
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGuizmo::BeginFrame();
	}

	void ImguiLayer::end() {
		ImGuiIO& io = ImGui::GetIO();
		Application& app = Application::get();
		io.DisplaySize = ImVec2(app.getWindow().getWidth(), app.getWindow().getHeight());
		
		// Render
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
			auto* window = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(window);
		}
	}

	void ImguiLayer::setDarkThemeColors() {
		auto& colors = ImGui::GetStyle().Colors;
		colors[ImGuiCol_WindowBg] = ImVec4{ 0.1f, 0.105f, 0.11f, 1.0f };

		// Headers
		colors[ImGuiCol_Header] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
		colors[ImGuiCol_HeaderHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
		colors[ImGuiCol_HeaderActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

		// Buttons
		colors[ImGuiCol_Button] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
		colors[ImGuiCol_ButtonHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
		colors[ImGuiCol_ButtonActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

		// Frame BG
		colors[ImGuiCol_FrameBg] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
		colors[ImGuiCol_FrameBgHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
		colors[ImGuiCol_FrameBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

		// Tabs
		colors[ImGuiCol_Tab] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TabHovered] = ImVec4{ 0.38f, 0.3805f, 0.381f, 1.0f };
		colors[ImGuiCol_TabActive] = ImVec4{ 0.28f, 0.2805f, 0.281f, 1.0f };
		colors[ImGuiCol_TabUnfocused] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };

		// Title
		colors[ImGuiCol_TitleBg] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TitleBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
	}
}
