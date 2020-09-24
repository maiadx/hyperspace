#pragma once
#include "Renderer/Window.h"

enum class hyRenderAPI
{
	VULKAN, DIRECTX12
};

class hyRenderAPIContext
{
	//CreateGraphicsDevice
	//...

public:
	static hyRenderAPIContext* CreateContext(hyRenderAPI api);
	
	virtual void Init(hyWindow& window) = 0;
	virtual void Shutdown() = 0;

	virtual void DrawFrame() = 0;
		
};

