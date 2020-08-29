#include "hypch.h"
#include "Renderer/Platform/RenderAPIContext.h"
#include "Renderer/Platform/VulkanContext.h"
#include "Renderer/Platform/DirectX12Context.h"

hyRenderAPIContext* hyRenderAPIContext::CreateContext(hyRenderAPI api)
{
	if (api == hyRenderAPI::VULKAN)
		return new hyVulkanContext();
	else								/* else: platform is DX12 */
		return new hyDirectX12Context();
}