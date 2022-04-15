#include "Shado.h"
#include "EditorLayer.h"
#include "script/ScriptManager.h"
using namespace Shado;


int main(int argc, const char** argv)
{
	// Testing script
	// ScriptManager::init("C:\\Users\\shadi\\Desktop\\Test CS Shado engine\\Test CS Shado engine\\bin\\Debug\\net6.0\\Test CS Shado engine.dll");
	//
	// void (*hehexd)(void) = []() { SHADO_CORE_INFO("HEllo!"); };
	// ScriptManager::addInternalCall("Shado.TestClass::SayHello_Impl", hehexd);
	// ScriptManager::invokeStaticMethod("Shado.TestClass::HelloCS()");


	SHADO_PROFILE_BEGIN_SESSION("Startup", "ShadoProfile-Startup.json");
	auto& application = Application::get();
	application.getWindow().resize(1920, 1080);
	application.submit(new EditorLayer);
	SHADO_PROFILE_END_SESSION();


	SHADO_PROFILE_BEGIN_SESSION("Runtime", "ShadoProfile-Runtime.json");
	application.run();
	SHADO_PROFILE_END_SESSION();


	SHADO_PROFILE_BEGIN_SESSION("Shutdown", "ShadoProfile-Shutdown.json");
	Application::destroy();
	SHADO_PROFILE_END_SESSION();
}