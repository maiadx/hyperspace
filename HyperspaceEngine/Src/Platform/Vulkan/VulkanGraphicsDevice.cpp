#include "hypch.h"
#include "VulkanGraphicsDevice.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanSwapChain.h"
#include "Core/Core.h"
#include <set>
#include <iostream>

/* Select a physical device */
void hyVulkanGraphicsDevice::SelectPhysicalDevice()
{
	u32 numDevices = 0;
	VkInstance instance = hyVulkanContext::Get().GetInstance();

	vkEnumeratePhysicalDevices(instance, &numDevices, nullptr);

	if (numDevices == 0)
		std::cerr << "Error: Unable to find Graphics Processor with Vulkan Support!" << std::endl;

	std::vector<VkPhysicalDevice> gfxDevices(numDevices);

	vkEnumeratePhysicalDevices(hyVulkanContext::Get().GetInstance(), &numDevices, gfxDevices.data());

	u32 highestScore = 0;
	size_t indexHighestScore = 0;
	/* Find best available GPU */
	for (u32 i = 0; i < numDevices; i++)
	{
		u32 score = ScoreGraphicsDeviceSuitability(gfxDevices[i]);

		if (score > highestScore)
		{
			highestScore = score;
			indexHighestScore = i;
		}
	}

	if (highestScore == 0)
		std::cerr << "Error: No suitable Vulkan Gfx device found!" << std::endl;

	PhysicalDevice = gfxDevices[indexHighestScore];

}

/* Score each available GPU based upon feature-set */
u32 hyVulkanGraphicsDevice::ScoreGraphicsDeviceSuitability(VkPhysicalDevice& device)
{
	VkPhysicalDeviceProperties deviceProperties;
	VkPhysicalDeviceFeatures deviceFeatures;

	vkGetPhysicalDeviceProperties(device, &deviceProperties);
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	/* rules: 1. device must have geometry shader support
			  2. Discrete Gfx strongly preferred
			  3. Max image resolution is signficant perf. factor
	*/

	return (deviceFeatures.geometryShader * ((deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) * 1000
		+ deviceProperties.limits.maxImageDimension2D)) * IsGraphicsDeviceSuitable(device);
}

void hyVulkanGraphicsDevice::CreateLogicalDevice()
{
	hyQueueFamilyIndices indices = FindQueueFamilies(PhysicalDevice);

	std::vector<VkDeviceQueueCreateInfo> allQueuesCreateInfo;
	std::set<u32> uniqueQueueFamilies = { indices.GraphicsFamily.value(), indices.PresentFamily.value() };

	float queuePriority = 1.0f;

	for (u32 queueFamily : uniqueQueueFamilies)
	{
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;

		queueCreateInfo.pQueuePriorities = &queuePriority;
		allQueuesCreateInfo.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures{};
	VkDeviceCreateInfo deviceCreateInfo{};

	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	deviceCreateInfo.queueCreateInfoCount = static_cast<u32>(allQueuesCreateInfo.size());
	deviceCreateInfo.pQueueCreateInfos = allQueuesCreateInfo.data();
	deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

	deviceCreateInfo.enabledExtensionCount = static_cast<u32>(DeviceExtensions.size());
	deviceCreateInfo.ppEnabledExtensionNames = DeviceExtensions.data();

	if (USE_VK_VALIDATION_LAYERS)
	{
		deviceCreateInfo.enabledLayerCount = static_cast<u32>(hyVulkanContext::Get().GetValidationLayers().size());
		deviceCreateInfo.ppEnabledLayerNames = hyVulkanContext::Get().GetValidationLayers().data();
	}
	else
	{
		deviceCreateInfo.enabledLayerCount = 0;
	}

	if (vkCreateDevice(PhysicalDevice, &deviceCreateInfo, nullptr, &Device) != VK_SUCCESS)
		std::cerr << "Failed to Create Logical Device!" << std::endl;

	vkGetDeviceQueue(Device, indices.GraphicsFamily.value(), 0, &GraphicsQueue);
	vkGetDeviceQueue(Device, indices.PresentFamily.value(), 0, &PresentQueue);
}

void hyVulkanGraphicsDevice::DestroyLogicalDevice()
{

}

VkPhysicalDevice& hyVulkanGraphicsDevice::GetPhysicalDevice()
{
	return PhysicalDevice;
}

VkDevice& hyVulkanGraphicsDevice::GetDevice()
{
	return Device;
}

VkQueue& hyVulkanGraphicsDevice::GetGraphicsQueue()
{
	return GraphicsQueue;
}

VkQueue& hyVulkanGraphicsDevice::GetPresentQueue()
{
	return PresentQueue;
}


bool hyVulkanGraphicsDevice::IsGraphicsDeviceSuitable(VkPhysicalDevice& device)
{
	hyQueueFamilyIndices indices = FindQueueFamilies(device);
	bool extensionsSupported = CheckDeviceExtensionSupport(device);
	bool swapChainSuitable = false;

	if (extensionsSupported)
	{
		hySwapChainSupportDetails swapChainSupport = hyVulkanContext::Get().GetSwapChain().QuerySupport(device);
		swapChainSuitable = !swapChainSupport.Formats.empty() && !swapChainSupport.PresentModes.empty();
	}

	return indices.IsComplete() && swapChainSuitable;
}

hyQueueFamilyIndices hyVulkanGraphicsDevice::FindQueueFamilies(VkPhysicalDevice& device)
{
	hyQueueFamilyIndices indices;

	u32 numQueueFamilies = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &numQueueFamilies, nullptr);

	//std::cout << numQueueFamilies << std::endl;
	std::vector<VkQueueFamilyProperties> queueFamilies(numQueueFamilies);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &numQueueFamilies, queueFamilies.data());

	/* need to find at least one queue family that supports VK_QUEUE_GRAPHICS_BIT */
	u32 i = 0;
	for (const VkQueueFamilyProperties& queueFamily : queueFamilies)
	{
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			indices.GraphicsFamily = i;

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, hyVulkanContext::Get().GetActiveSurface(), &presentSupport);

		if (presentSupport)
			indices.PresentFamily = i;

		if (indices.IsComplete())
			break;

		i++;
	}

	return indices;
}

bool hyVulkanGraphicsDevice::CheckDeviceExtensionSupport(VkPhysicalDevice& device)
{
	u32 extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(DeviceExtensions.begin(), DeviceExtensions.end());

	for (const VkExtensionProperties& extension : availableExtensions)
		requiredExtensions.erase(extension.extensionName);

	return requiredExtensions.empty();
}

