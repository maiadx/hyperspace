#include "hypch.h"
#include "Renderer/Window.h"
#include <iostream>


hyWindow::hyWindow(u32 width, u32 height, const char* title) : Width(width), Height(height), Title(title), WindowContext(nullptr)
{
	
}

hyWindow::~hyWindow()
{

}

void hyWindow::Init(hyWindow& window)
{


	if (!glfwInit())
	{
		std::cerr << "GLFW Initalization failed!" << std::endl;
	}
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	window.WindowContext = glfwCreateWindow(window.Width, window.Height, window.Title, nullptr, nullptr);



}

void hyWindow::Close(hyWindow& window)
{
	glfwDestroyWindow(window.WindowContext);
	glfwTerminate();

}

void hyWindow::Update()
{
	glfwPollEvents();
}

bool hyWindow::ShouldClose()
{
	return glfwWindowShouldClose(WindowContext);
}

GLFWwindow* hyWindow::GetGLFWContext()
{
	return WindowContext;
}