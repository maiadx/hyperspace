#pragma once
#include "Core/Core.h"
#include "Core/Scene.h"
#include "Renderer/Window.h"
#include "Utils/Time.h"

class hyApplication
{
	hyScene Scene;
	hyWindow Window;
	hyChrono Timer;

public:

	hyApplication();
	~hyApplication();

	void Init();
	void Shutdown();
	void Run(); 
		
};
	

