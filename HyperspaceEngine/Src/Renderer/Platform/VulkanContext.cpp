#include "hypch.h"
#include "Renderer/Platform/VulkanContext.h"
#include <GLFW/glfw3.h>
#include <cstring>

#include "Utils/ArrayList.h"
#include "Utils/Array.h"
#include <string>
#include <vector>
#include <array>

#include <iostream>



#ifdef HYPERSPACE_BUILD_DEBUG
	constexpr bool ENABLE_VALIDATION_LAYERS = true;							
#else 
	constexpr bool ENABLE_VALIDATION_LAYERS = false;
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


void hyVulkanContext::Init()
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
	std::array<const char*, static_cast<size_t>(NUM_ACTIVE_VK_VALIDATION_LAYERS)> validationLayers = { "VK_LAYER_KHRONOS_validation" };
	VkDebugUtilsMessengerCreateInfoEXT onCreateDebugInfo;

	/* */
	if (ENABLE_VALIDATION_LAYERS)
	{
		if (!CheckValidationLayersAvailable(validationLayers))
			std::cerr << "Validation layers requested - but are not available!" << std::endl;

		onCreateInfo.enabledLayerCount = NUM_ACTIVE_VK_VALIDATION_LAYERS;
		onCreateInfo.ppEnabledLayerNames = validationLayers.data();

		PopulateDebugMessengerCreateInfo(onCreateDebugInfo);

		onCreateInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &onCreateDebugInfo;

	}
	else
	{
		onCreateInfo.enabledLayerCount = 0;
		onCreateInfo.pNext = nullptr;
	}

	if (vkCreateInstance(&onCreateInfo, nullptr, &Instance) != VkResult::VK_SUCCESS)
	{
		std::cerr << "Error: Failed to Create Vulkan Instance!" << std::endl;
	}

	SetupDebugMessenger();
}

void hyVulkanContext::Shutdown()
{
	if (ENABLE_VALIDATION_LAYERS)
		DestroyDebugMessengerExt(nullptr);

	vkDestroyInstance(Instance, nullptr);
}

void hyVulkanContext::SetupDebugMessenger()
{
	if (!ENABLE_VALIDATION_LAYERS)
		return;

	VkDebugUtilsMessengerCreateInfoEXT onCreateDebugInfo;
	PopulateDebugMessengerCreateInfo(onCreateDebugInfo);

	if (CreateDebugMessengerExt(&onCreateDebugInfo, nullptr) != VK_SUCCESS)
		std::cerr << "Error: Unable to initialize Debug Messenger!" << std::endl;
		
	
}


bool hyVulkanContext::CheckValidationLayersAvailable(std::array<const char*, NUM_ACTIVE_VK_VALIDATION_LAYERS>& validationLayers)
{
	u32 layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : validationLayers)
	{
		bool isAvailable = false;

		for (const auto& availableLayer : availableLayers)
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

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + numGlfwExtensions);

	if (ENABLE_VALIDATION_LAYERS)							
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

VkResult hyVulkanContext::CreateDebugMessengerExt(const VkDebugUtilsMessengerCreateInfoEXT* onCreateDebugInfo, const VkAllocationCallbacks* allocator)
{
	auto fn = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(Instance, "vkCreateDebugUtilsMessengerEXT");
	if (fn != nullptr)
		fn(Instance, onCreateDebugInfo, allocator, &DebugMessenger);
	else
		return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void hyVulkanContext::DestroyDebugMessengerExt(const VkAllocationCallbacks* allocator)
{
	auto fn = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(Instance, "vkDestroyDebugUtilsMessengerEXT");
	if (fn != nullptr)
		fn(Instance, DebugMessenger, allocator);
}