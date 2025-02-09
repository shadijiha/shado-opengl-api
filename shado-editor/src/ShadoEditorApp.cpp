#include "Shado.h"
#include "EditorLayer.h"

using namespace Shado;

#if SHADO_PLATFORM_WINDOWS && SHADO_DIST
int WINAPI WinMain( HINSTANCE hInstance,    // HANDLE TO AN INSTANCE.  This is the "handle" to YOUR PROGRAM ITSELF.
					HINSTANCE hPrevInstance,// USELESS on modern windows (totally ignore hPrevInstance)
					LPSTR argv,				// Command line arguments.  similar to argv in standard C programs
					int iCmdShow )          // Start window maximized, minimized, etc.
#else
int main(int argc, const char** argv)
#endif
{
	std::filesystem::current_path(std::filesystem::path(__argv[0]).parent_path());
	
	SHADO_PROFILE_BEGIN_SESSION("Startup", "ShadoProfile-Startup.json");
	
	auto& application = Application::get();	// <--- Log is init here, don't call any logs functions before it
	application.getWindow().resize(1920, 1080);
	application.submit(snew(EditorLayer) EditorLayer);
	SHADO_CORE_INFO("fs current dir was set to: {0}", std::filesystem::current_path().string());
	SHADO_PROFILE_END_SESSION();


	SHADO_PROFILE_BEGIN_SESSION("Runtime", "ShadoProfile-Runtime.json");
	application.run();
	SHADO_PROFILE_END_SESSION();


	SHADO_PROFILE_BEGIN_SESSION("Shutdown", "ShadoProfile-Shutdown.json");
	Application::destroy();
	SHADO_PROFILE_END_SESSION();
}