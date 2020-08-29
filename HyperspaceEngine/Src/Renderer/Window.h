#pragma once
#include "Core/Core.h"
#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

//#define GLM_FORCE_RADIANS
//#define GLM_FORCE_DEPTH_ZERO_TO_ONE
//#include <glm/vec4.hpp>
//#include <glm/mat4x4.hpp>


/* this is the desktop window */

class hyWindow
{
	u32 Width;
	u32 Height;
	const char* Title;
	GLFWwindow* WindowContext;

public:

	hyWindow(u32 width, u32 height, const char* title);
	~hyWindow();

	void Update();
	bool ShouldClose();

	static void Init(hyWindow& window);
	static void Close(hyWindow& window);
	
		
};
\
