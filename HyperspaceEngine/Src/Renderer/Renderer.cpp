#include "hypch.h"
#include "Renderer.h"
#include "Platform/Vulkan/VulkanContext.h"


void hyRenderer::Init(hyRenderAPI api, hyWindow& window)
{
	//hyRenderCmd::CreateContext(api, window);
	//hyRenderAPIContext* renderAPIContext = hyRenderAPIContext::CreateContext(hyRenderAPI::VULKAN);
	hyVulkanContext& context = hyVulkanContext::Get();
	hyVulkanContext::Get().Init(window);


}

void hyRenderer::Shutdown()
{
	hyVulkanContext::Get().Shutdown();
}

void hyRenderer::BeginScene()
{

}

void hyRenderer::EndScene()
{
	hyVulkanContext::Get().DrawFrame();
}