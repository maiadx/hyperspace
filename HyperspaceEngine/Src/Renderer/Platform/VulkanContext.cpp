#include "hypch.h"
#include "Renderer/Platform/VulkanContext.h"
#include <cstring>

#include "Utils/ArrayList.h"
#include "Utils/Array.h"
#include <string>
#include <cstdint>

#include <vector>
#include <array>
#include <set>
#include <iostream>


#ifdef HYPERSPACE_BUILD_DEBUG
	constexpr bool USE_VK_VALIDATION_LAYERS = true;
	static std::vector<const char*> sValidationLayers = { "VK_LAYER_KHRONOS_validation" };
#else
static std::vector<const char*> sValidationLayers(0);// = { "VK_LAYER_KHRONOS_validation" };
	constexpr bool USE_VK_VALIDATION_LAYERS = false;
#endif



	//static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, 
	//	VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {

static VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
													      VkDebugUtilsMessageTypeFlagsEXT messageType,
													      const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
													      void* userData)
{
	std::string errorLevel;

	switch (messageSeverity)
	{
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: 
		errorLevel = "Diagnostic Info";
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
		errorLevel = "Debug Info";
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
		errorLevel = "Anomalous Behavior";
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
		errorLevel = "Critical Error";
		break;
	};

	std::cerr << errorLevel << ": " << callbackData->pMessage << std::endl;

	return VK_FALSE;
}

hyVulkanContext::hyVulkanContext() : Instance(), DebugMessenger(), PhysicalDevice(VK_NULL_HANDLE), LogicalDevice(), 
									 GraphicsQueue(), WindowSurface(), PresentQueue(), SwapChain() {}


void hyVulkanContext::Init(hyWindow& window)
{
	CreateInstance();
	SetupDebugMessenger();
	CreateWindowSurface(window);
	SelectGfxDevice();
	CreateLogicalDevice();
	CreateSwapChain();
}

void hyVulkanContext::Shutdown()
{
	vkDestroySwapchainKHR(LogicalDevice, SwapChain, nullptr);
	vkDestroyDevice(LogicalDevice, nullptr);

	if (USE_VK_VALIDATION_LAYERS)
		DestroyDebugMessengerExt(nullptr);

	vkDestroySurfaceKHR(Instance, WindowSurface, nullptr);
	vkDestroyInstance(Instance, nullptr);
}

void hyVulkanContext::CreateInstance()
{
	VkApplicationInfo appInfo{};

	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Sandbox";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);

	appInfo.pEngineName = "Hyperspace Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_2;

	VkInstanceCreateInfo onCreateInfo{};
	onCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	onCreateInfo.pApplicationInfo = &appInfo;

	/* Extensions: */

	std::vector<const char*> extensions = GetRequiredExtensions();
	onCreateInfo.enabledExtensionCount = static_cast<u32>(extensions.size());
	onCreateInfo.ppEnabledExtensionNames = extensions.data();


	/* Validation Layers: */

	VkDebugUtilsMessengerCreateInfoEXT onCreateDebugInfo;

	/* */
#ifdef HYPERSPACE_BUILD_DEBUG

	if (!CheckValidationLayersAvailable(sValidationLayers))
		std::cerr << "Validation layers requested - but are not available!" << std::endl;

	onCreateInfo.enabledLayerCount = static_cast<u32>(sValidationLayers.size());
	onCreateInfo.ppEnabledLayerNames = sValidationLayers.data();

	PopulateDebugMessengerCreateInfo(onCreateDebugInfo);

	onCreateInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&onCreateDebugInfo;

#else
	onCreateInfo.enabledLayerCount = 0;
	onCreateInfo.pNext = nullptr;
#endif

	if (vkCreateInstance(&onCreateInfo, nullptr, &Instance) != VkResult::VK_SUCCESS)
	{
		std::cerr << "Error: Failed to Create Vulkan Instance!" << std::endl;
	}
}

void hyVulkanContext::SetupDebugMessenger()
{
	if (!USE_VK_VALIDATION_LAYERS)
		return;

	VkDebugUtilsMessengerCreateInfoEXT onCreateDebugInfo;
	PopulateDebugMessengerCreateInfo(onCreateDebugInfo);

	if (CreateDebugMessengerExt(&onCreateDebugInfo, nullptr) != VK_SUCCESS)
		std::cerr << "Error: Unable to initialize Debug Messenger!" << std::endl;
		
	
}


bool hyVulkanContext::CheckValidationLayersAvailable(std::vector<const char*>& validationLayers)
{
	u32 layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : validationLayers)
	{
		bool isAvailable = false;

		for (const VkLayerProperties& availableLayer : availableLayers)
		{
			if (strcmp(layerName, availableLayer.layerName) == 0)
			{
				isAvailable = true;
				break;
			}
		}

		if (!isAvailable)
			return false;
	}

	return true;
}

std::vector<const char*> hyVulkanContext::GetRequiredExtensions()
{
	u32 numGlfwExtensions = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&numGlfwExtensions);
	//std::cout << glfwExtensions[0] << std::endl;

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + numGlfwExtensions);

	if (USE_VK_VALIDATION_LAYERS)
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

	return extensions;
}

void hyVulkanContext::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& onCreateDebugMessengerInfo)
{
	onCreateDebugMessengerInfo = {};
	onCreateDebugMessengerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	onCreateDebugMessengerInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	onCreateDebugMessengerInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	onCreateDebugMessengerInfo.pfnUserCallback = VulkanDebugCallback;
}

/* */
VkResult hyVulkanContext::CreateDebugMessengerExt(const VkDebugUtilsMessengerCreateInfoEXT* onCreateDebugInfo, const VkAllocationCallbacks* allocator)
{
	auto fn = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(Instance, "vkCreateDebugUtilsMessengerEXT");
	if (fn != nullptr)
		return fn(Instance, onCreateDebugInfo, allocator, &DebugMessenger);
	else
		return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void hyVulkanContext::DestroyDebugMessengerExt(const VkAllocationCallbacks* allocator)
{
	auto fn = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(Instance, "vkDestroyDebugUtilsMessengerEXT");
	if (fn != nullptr)
		fn(Instance, DebugMessenger, allocator);
}

void hyVulkanContext::CreateWindowSurface(hyWindow& window)
{
	/*std::cout << "Creating Window!" << std::endl;*/
	if (glfwCreateWindowSurface(Instance, window.GetGLFWContext(), nullptr, &WindowSurface) != VK_SUCCESS)
		std::cerr << "Window creation failed to succeed!" << std::endl;

}

/* Select a physical device */
void hyVulkanContext::SelectGfxDevice()
{
	u32 numDevices = 0;
	vkEnumeratePhysicalDevices(Instance, &numDevices, nullptr);

	if (numDevices == 0)
		std::cerr << "Error: Unable to find Graphics Processor with Vulkan Support!" << std::endl;

	std::vector<VkPhysicalDevice> gfxDevices(numDevices);

	vkEnumeratePhysicalDevices(Instance, &numDevices, gfxDevices.data());

	u32 highestScore = 0;
	u32 indexHighestScore = 0;
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
u32 hyVulkanContext::ScoreGraphicsDeviceSuitability(const VkPhysicalDevice& device)
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
										     + deviceProperties.limits.maxImageDimension2D)) * IsGfxDeviceSuitable(device);
}

bool hyVulkanContext::IsGfxDeviceSuitable(const VkPhysicalDevice& device)
{
	hyQueueFamilyIndices indices = FindQueueFamilies(device);
	bool extensionsSupported = CheckDeviceExtensionSupport(device);
	bool swapChainSuitable = false;
	
	if (extensionsSupported)
	{
		hySwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device);
		swapChainSuitable = !swapChainSupport.Formats.empty() && !swapChainSupport.PresentModes.empty();
	}

	return indices.IsComplete() && swapChainSuitable;
}

bool hyVulkanContext::CheckDeviceExtensionSupport(VkPhysicalDevice device)
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

void hyVulkanContext::CreateLogicalDevice()
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
		deviceCreateInfo.enabledLayerCount = static_cast<u32>(sValidationLayers.size());
		deviceCreateInfo.ppEnabledLayerNames = sValidationLayers.data();
	} 
	else 
	{
		deviceCreateInfo.enabledLayerCount = 0;
	}

	if (vkCreateDevice(PhysicalDevice, &deviceCreateInfo, nullptr, &LogicalDevice) != VK_SUCCESS)
		std::cerr << "Failed to Create Logical Device!" << std::endl;

	vkGetDeviceQueue(LogicalDevice, indices.GraphicsFamily.value(), 0, &GraphicsQueue);
	vkGetDeviceQueue(LogicalDevice, indices.PresentFamily.value(), 0, &PresentQueue);
}

hyQueueFamilyIndices hyVulkanContext::FindQueueFamilies(VkPhysicalDevice device)
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
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, WindowSurface, &presentSupport);

		if (presentSupport)
			indices.PresentFamily = i;

		if (indices.IsComplete())
			break;

		i++;
	}

	return indices;
}

void hyVulkanContext::CreateSwapChain()
{
	hySwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(PhysicalDevice);

	VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.Formats);
	VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.PresentModes);
	VkExtent2D extent = ChooseSwapExtent(swapChainSupport.Capabilities);

	/* Min image count for SwapChain to function + added buffer of 1*/
	u32 minNumImages = swapChainSupport.Capabilities.minImageCount + 1;

	if (swapChainSupport.Capabilities.maxImageCount > 0 && minNumImages > swapChainSupport.Capabilities.maxImageCount)
		minNumImages = swapChainSupport.Capabilities.maxImageCount;

	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = WindowSurface;

	createInfo.minImageCount = minNumImages;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;									/* This is 1 unless doing stereoscopic rendering */
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;		/* What we use images in SwapChain for */
																		/* (For postprocesing you may use VK_IMAGE_USAGE_TRANSFER_DST_BIT) */
	hyQueueFamilyIndices indices = FindQueueFamilies(PhysicalDevice);
	u32 queueFamilyIndices[] = { indices.GraphicsFamily.value(), indices.PresentFamily.value() };

	if (indices.GraphicsFamily != indices.PresentFamily)
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	} 
	else
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;		/* last two are optional: */
		createInfo.queueFamilyIndexCount = 0;		
		createInfo.pQueueFamilyIndices = nullptr;
	}
	/* Specify how to handle swap chain images that will be used across multiple queue families. */
	/* If queue families differ, use VK_SHARING_MODE_CONCURRENT to avoid ownership issues */

	createInfo.preTransform = swapChainSupport.Capabilities.currentTransform;	/* no image transform (flip, rotate, etc.) */
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;				/* image rendered to windows should not be given an alpha */
	
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;												/* if part of windows is obscured, we don't care about color of pixels. */
	createInfo.oldSwapchain = VK_NULL_HANDLE;									/* SwapChain becomes old if window gets resized, for example. */
	

	if (vkCreateSwapchainKHR(LogicalDevice, &createInfo, nullptr, &SwapChain) != VK_SUCCESS)
		std::cerr << "Error: Failed to create swap chain!" << std::endl;

	/* */
	vkGetSwapchainImagesKHR(LogicalDevice, SwapChain, &minNumImages, nullptr);
	SwapChainImages.resize(minNumImages);
	vkGetSwapchainImagesKHR(LogicalDevice, SwapChain, &minNumImages, SwapChainImages.data());

	/* */
	SwapChainImageFormat = surfaceFormat.format;
	SwapChainExtent = extent;
}

/* 
*/
hySwapChainSupportDetails hyVulkanContext::QuerySwapChainSupport(VkPhysicalDevice device)
{
	hySwapChainSupportDetails details;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, WindowSurface, &details.Capabilities);

	u32 numFormats;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, WindowSurface, &numFormats, nullptr);

	if (numFormats != 0)
	{
		details.Formats.resize(numFormats);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, WindowSurface, &numFormats, details.Formats.data());
	}

	u32 numPresentModes;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, WindowSurface, &numPresentModes, nullptr);

	if (numPresentModes != 0)
	{
		details.PresentModes.resize(numPresentModes);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, WindowSurface, &numPresentModes, details.PresentModes.data());
	}
	return details;
}

/* */
VkSurfaceFormatKHR hyVulkanContext::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	for (const VkSurfaceFormatKHR& availableFormat : availableFormats)
	{
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			return availableFormat;
	}
	return availableFormats[0];					/* */
}


/* Possible Swap Present Modes are: 
*  VK_PRESENT_MODE_IMMEDIATE_KHR    - Immediate swap, leads to screen-tearing 
*  VK_PRESENT_MODE_FIFO_KHR         - Queue-based swap, if queue is full then program has to wait (provides "sync")
*  VK_PRESENT_MODE_FIFO_RELAXED_KHR - Similar to FIFO, except if the application is late and queue is empty the image is   
*									  displayed immediately upon arrival.
*  VK_PRESENT_MODE_MAILBOX_KHR      - Also like FIFO, but instead of blocking when queue is full, images already in the
*									  queue get replaced with newer ones. Can be used to implement triple-buffering.
*/
VkPresentModeKHR hyVulkanContext::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
	for (const VkPresentModeKHR& availablePresentMode : availablePresentModes)
	{
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			return availablePresentMode;
	}
	return VK_PRESENT_MODE_FIFO_KHR;
}
/* Swap Extent - resolution of swap chain images, almost always equal to monitor resolution.
*  Some window managers use a different resolution, and this is indicated using UINT32_MAX as its value.
*/
VkExtent2D hyVulkanContext::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
	if (capabilities.currentExtent.width != UINT32_MAX)											
		return capabilities.currentExtent;

		std::cout << "Warning: Window Manager using non-native swap image resolution!" << std::endl;
		VkExtent2D actualExtent = { 800, 600 };

		actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));
		return actualExtent;
	
}
