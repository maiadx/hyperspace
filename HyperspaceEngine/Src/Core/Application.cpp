#include "hypch.h"
#include "Application.h"
#include "Utils/Vector.h"
#include "Renderer/Window.h"
#include "Renderer/Renderer.h"
#include "Events/Event.h"


extern void OnUserInit();
extern void OnUserShutdown();
extern void OnUserUpdate();
extern void OnUserFrameUpdate();


hyApplication::hyApplication() : Window(750, 750, "Hyperspace")
{

}

hyApplication::~hyApplication()
{
	
}


void hyApplication::Init()
{
	hyWindow::Init(Window);
	hyRenderer::Init(hyRenderAPI::VULKAN, Window);

}


void hyApplication::Shutdown()
{
	hyRenderer::Shutdown();
	hyWindow::Close(Window);
}


void hyApplication::Run()
{
	Timer.Reset();

	while (!Window.ShouldClose())
	{
		OnUserUpdate();                  


		if (Timer.GetTime() > 1000.0 / 144)
		{
			//std::cout << Timer.GetTime() << std::endl;
			Window.Update();
			hyRenderer::BeginScene();

			hyRenderer::EndScene();

			OnUserFrameUpdate();

			Timer.Reset();
		}
	}
}