#pragma once
#include "Renderer/RenderAPIContext.h"

class hyRenderCmd
{
	static hyRenderAPIContext* RenderAPIContext;

public:

	static void CreateContext(hyRenderAPI api, hyWindow& window);
	static void DestroyContext();	

	static void DrawFrame();



};

