#include "Shado.h"
#include "EditorLayer.h"
using namespace Shado;


int main(int argc, const char** argv)
{
	auto& application = Application::get();
	application.getWindow().resize(1920, 1080);
	application.submit(new EditorLayer);
	application.run();

	Application::destroy();
}