#pragma once
#include "Renderer/Platform/RenderAPIContext.h"
#include "Core/Core.h"
#include <vulkan/vulkan.h>

#include <vector>
#include <array>

constexpr u32 NUM_ACTIVE_VK_VALIDATION_LAYERS = 1;

class hyVulkanContext : public hyRenderAPIContext
{
	VkInstance Instance;
	VkDebugUtilsMessengerEXT DebugMessenger;
	
	bool CheckValidationLayersAvailable(std::array<const char*, NUM_ACTIVE_VK_VALIDATION_LAYERS>& validationLayers);
	std::vector<const char*> GetRequiredExtensions();

	void SetupDebugMessenger();
	void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& onCreateDebugMessengerInfo);

	VkResult CreateDebugMessengerExt(const VkDebugUtilsMessengerCreateInfoEXT* onCreateDebugInfo, const VkAllocationCallbacks* allocator);
	void DestroyDebugMessengerExt(const VkAllocationCallbacks* allocator);


public:
	hyVulkanContext() : Instance(), DebugMessenger() {}

	void Init();
	void Shutdown();

};

