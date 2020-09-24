#pragma once
#include "Renderer/GraphicsDevice.h"
#include "Core/Core.h"
#include <vector>
#include <optional>
#include <vulkan/vulkan.h>
/* */
struct hyQueueFamilyIndices
{
	std::optional<u32> GraphicsFamily;
	std::optional<u32> PresentFamily;

	bool IsComplete() { return GraphicsFamily.has_value() && PresentFamily.has_value(); }
};


class hyVulkanGraphicsDevice : public hyGraphicsDevice
{
	/* Physical / Logical Devices + Queues */
	VkPhysicalDevice PhysicalDevice;
	VkDevice Device;										/* Logical Device */
	VkQueue GraphicsQueue;
	VkQueue PresentQueue;
	const std::vector<const char*> DeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

public:

	void SelectPhysicalDevice() override;
	void CreateLogicalDevice() override;
	void DestroyLogicalDevice() override;

	VkPhysicalDevice& GetPhysicalDevice();
	VkDevice& GetDevice();
	VkQueue& GetGraphicsQueue();
	VkQueue& GetPresentQueue();
	

	u32 ScoreGraphicsDeviceSuitability(VkPhysicalDevice& device);
	bool IsGraphicsDeviceSuitable(VkPhysicalDevice&);
	bool CheckDeviceExtensionSupport(VkPhysicalDevice& device);
	hyQueueFamilyIndices FindQueueFamilies(VkPhysicalDevice& device);
};

