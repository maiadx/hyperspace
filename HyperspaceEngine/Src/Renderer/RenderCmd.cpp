#include "hypch.h"
#include "Renderer/RenderCmd.h"
#include "Platform/Vulkan/VulkanContext.h"
hyRenderAPIContext* hyRenderCmd::RenderAPIContext;

void hyRenderCmd::CreateContext(hyRenderAPI api, hyWindow& window)
{
	//RenderAPIContext = hyRenderAPIContext::CreateContext(api);
	//RenderAPIContext->Init(window);
	//hyVulkanContext context = hyVulkanContext::Get();
	//context.Init(window);
}

void hyRenderCmd::DestroyContext()
{
	//RenderAPIContext->Shutdown();
}

void hyRenderCmd::DrawFrame()
{
	//RenderAPIContext->DrawFrame();
}