#include "Shado.h"
#include "EditorLayer.h"

using namespace Shado;


int main(int argc, const char** argv)
{
	SHADO_PROFILE_BEGIN_SESSION("Startup", "ShadoProfile-Startup.json");
	auto& application = Application::get();
	application.getWindow().resize(1920, 1080);
	application.submit(snew(EditorLayer) EditorLayer);
	SHADO_PROFILE_END_SESSION();


	SHADO_PROFILE_BEGIN_SESSION("Runtime", "ShadoProfile-Runtime.json");
	application.run();
	SHADO_PROFILE_END_SESSION();


	SHADO_PROFILE_BEGIN_SESSION("Shutdown", "ShadoProfile-Shutdown.json");
	Application::destroy();
	SHADO_PROFILE_END_SESSION();
}