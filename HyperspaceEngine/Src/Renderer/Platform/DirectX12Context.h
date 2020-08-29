#pragma once
#include "Renderer/Platform/RenderAPIContext.h"

class hyDirectX12Context : public hyRenderAPIContext
{

public:
	void Init();
	void Shutdown();
	
};

