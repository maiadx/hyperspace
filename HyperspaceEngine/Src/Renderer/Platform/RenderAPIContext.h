#pragma once


enum class hyRenderAPI
{
	VULKAN, DIRECTX12
};

class hyRenderAPIContext
{


public:
	static hyRenderAPIContext* CreateContext(hyRenderAPI api);
	
	virtual void Init() = 0;
	virtual void Shutdown() = 0;
		
};

