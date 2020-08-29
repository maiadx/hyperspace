#include "hypch.h"
#include "Renderer/Window.h"


hyWindow::hyWindow(u32 width, u32 height, const char* title) : Width(width), Height(height), Title(title), WindowContext(nullptr)
{
	
}

hyWindow::~hyWindow()
{

}

void hyWindow::Init(hyWindow& window)
{

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	if (!glfwInit())
	{
		
	}

	window.WindowContext = glfwCreateWindow(window.Width, window.Height, window.Title, nullptr, nullptr);
	//std::cout << extensionCount << " extensions supported." << std::endl;

	




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
