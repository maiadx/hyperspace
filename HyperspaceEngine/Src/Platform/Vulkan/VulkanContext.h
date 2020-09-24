#pragma once
#include "Renderer/RenderAPIContext.h"
#include "Core/Core.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <array>

#include "Platform/Vulkan/VulkanSwapChain.h"
#include "Platform/Vulkan/VulkanGraphicsDevice.h"
#include "Platform/Vulkan/VulkanGraphicsPipeline.h"


/* hyVulkanContext : Manages Vulkan state and is responsible for implementing all supported graphics api calls.
	
*/

#ifdef HYPERSPACE_BUILD_DEBUG
constexpr bool USE_VK_VALIDATION_LAYERS = true;
#else
constexpr bool USE_VK_VALIDATION_LAYERS = false;
#endif



struct hyVulkanSyncObjects
{
	/* Rendering and Presentation */
	const u32 MAX_FRAMES_IN_FLIGHT = 2;
	size_t CurrentFrame = 0;
	std::vector<VkSemaphore> ImageAvailableSemahpores;	  /* image has been aquired - ready for rendering */
	std::vector<VkSemaphore> RenderFinishedSempaphores;	  /* rendering finished - ready for presentation */
	std::vector<VkFence> InFlightFences;
	std::vector<VkFence> ImagesInFlight;
};

/* */
class hyVulkanContext : public hyRenderAPIContext
{
	VkInstance Instance;
	VkDebugUtilsMessengerEXT DebugMessenger;
	VkSurfaceKHR WindowSurface;								/* surface used by desktop window */

	hyVulkanGraphicsDevice GraphicsDevice;
	hyVulkanGraphicsPipeline GraphicsPipeline;
	hyVulkanSwapChain SwapChain;
	hyVulkanSyncObjects SyncObjects;

	hyVulkanContext();
	void CreateInstance();

	/* Extensions and Validation Layers */
	bool CheckValidationLayersAvailable(std::vector<const char*>& validationLayers);
	std::vector<const char*> GetRequiredExtensions();

	/* Debug Messenger */
	void SetupDebugMessenger();
	void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& onCreateDebugMessengerInfo);
	VkResult CreateDebugMessengerExt(const VkDebugUtilsMessengerCreateInfoEXT* onCreateDebugInfo, const VkAllocationCallbacks* allocator);
	void DestroyDebugMessengerExt(const VkAllocationCallbacks* allocator);

	/* Window Surface */
	void CreateWindowSurface(hyWindow& window);

	/* Presentation */
	void CreateSyncObjects();

public:

	/* Returns singleton of vulkan context */
	static hyVulkanContext& Get()
	{
		static hyVulkanContext context;
		return context;
	}

	void Init(hyWindow& window) override;
	void Shutdown() override;

	void ResizeFrameBuffer(u32 width, u32 height);
	void DrawFrame();
	std::vector<const char*>& GetValidationLayers();
	
	inline VkInstance& GetInstance()
	{
		return Instance;
	}
	
	inline VkSurfaceKHR& GetActiveSurface()
	{
		return WindowSurface;
	}

	hyVulkanSwapChain& GetSwapChain();
	hyVulkanGraphicsPipeline& GetGraphicsPipeline();
	hyVulkanGraphicsDevice& GetGraphicsDevice();

};