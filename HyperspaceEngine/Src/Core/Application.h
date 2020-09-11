#pragma once
#include "Core/Core.h"
#include "Scene/Scene.h"
#include "Renderer/Window.h"
#include "Utils/Time.h"

/* hyApplication: 
		Scene  - Holds ECS elements
		Window - GLFW desktop window
		Timer  - Controls refreshrate of glfw window
*/

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
	

