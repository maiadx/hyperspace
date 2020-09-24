#pragma once
#include "Renderer/RenderAPIContext.h"

class hyDirectX12Context : public hyRenderAPIContext
{

public:
	void Init(hyWindow& window);
	void Shutdown();
	
	void DrawFrame() {}
};

