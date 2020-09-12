#pragma once
#include "Renderer/Platform/RenderAPIContext.h"
#include "Core/Core.h"
#include <vulkan/vulkan.h>
#include <optional>
#include <vector>
#include <array>

/* hyVulkanContext : Manages Vulkan state and is responsible for implementing all supported graphics api calls.
	
*/

/* */
struct hyQueueFamilyIndices
{
	std::optional<u32> GraphicsFamily;
	std::optional<u32> PresentFamily;

	bool IsComplete() { return GraphicsFamily.has_value() && PresentFamily.has_value(); }
};

struct hySwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR Capabilities;
	std::vector<VkSurfaceFormatKHR> Formats;
	std::vector<VkPresentModeKHR> PresentModes;
};

/* */
class hyVulkanContext : public hyRenderAPIContext
{
	VkInstance Instance;
	VkDebugUtilsMessengerEXT DebugMessenger;
	VkSurfaceKHR WindowSurface;								/* surface used by desktop window */

	/* Device Properties */
	VkPhysicalDevice PhysicalDevice;						/* todo: Move some this to their own structs? */
	VkDevice LogicalDevice;
	VkQueue GraphicsQueue;
	VkQueue PresentQueue;
	const std::vector<const char*> DeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

	/* Swap Chain Properties */
	VkSwapchainKHR SwapChain;
	VkFormat SwapChainImageFormat;
	VkExtent2D SwapChainExtent;
	std::vector<VkImage> SwapChainImages;

	/* Image Views */
	std::vector<VkImageView> SwapChainImageViews;



	
	void CreateInstance();

	/* Extensions + Validation Layers */
	bool CheckValidationLayersAvailable(std::vector<const char*>& validationLayers);
	std::vector<const char*> GetRequiredExtensions();

	/* Debug Messenger */
	void SetupDebugMessenger();
	void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& onCreateDebugMessengerInfo);
	VkResult CreateDebugMessengerExt(const VkDebugUtilsMessengerCreateInfoEXT* onCreateDebugInfo, const VkAllocationCallbacks* allocator);
	void DestroyDebugMessengerExt(const VkAllocationCallbacks* allocator);

	/* Window Surface */
	void CreateWindowSurface(hyWindow& window);

	/* Physical / Logical Devices + Queues */
	void SelectGfxDevice();
	u32 ScoreGraphicsDeviceSuitability(const VkPhysicalDevice& device);
	bool IsGfxDeviceSuitable(const VkPhysicalDevice&);
	bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
	VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

	void CreateLogicalDevice();
	
	hyQueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);

	/* Swap Chain */
	void CreateSwapChain();
	hySwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);
	VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

	/* Image Views */
	void CreateImageViews();

	/* Graphics Pipeline: Shaders */
	void CreateGraphicsPipeline();
	VkShaderModule CreateShaderModule(const std::vector<char>& spvCode);



public:
	hyVulkanContext();

	void Init(hyWindow& window);
	void Shutdown();

};

