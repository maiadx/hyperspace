#include "hypch.h"
#include "Renderer/RenderAPIContext.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/DX12/DirectX12Context.h"

hyRenderAPIContext* hyRenderAPIContext::CreateContext(hyRenderAPI api)
{
	if (api == hyRenderAPI::VULKAN)
		return &hyVulkanContext::Get();
	else								/* else: platform is DX12 */
		return new hyDirectX12Context();
}