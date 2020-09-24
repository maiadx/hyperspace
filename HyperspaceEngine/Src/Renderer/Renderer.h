#pragma once
#include "Renderer/RenderAPIContext.h"
#include "Renderer/RenderCmd.h"

class hyRenderer
{
	//static hyRenderer* sInstance;

	//hyRenderer();
	//~hyRenderer();

public:

	static void Init(hyRenderAPI api, hyWindow& window);
	static void Shutdown();

	static void BeginScene();
	static void EndScene();

};
