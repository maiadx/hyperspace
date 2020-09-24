#include "hypch.h"
#include "Platform/Vulkan/VulkanContext.h"
#include <cstring>

#include "Utils/Vector.h"
#include "Utils/Array.h"
#include <string>
#include <cstdint>

#include <vector>
#include <array>
#include <set>
#include <iostream>

#include "Core/AssetLoader.h"


#ifdef HYPERSPACE_BUILD_DEBUG
	static std::vector<const char*> sValidationLayers = { "VK_LAYER_KHRONOS_validation" };
#else
	static std::vector<const char*> sValidationLayers(0);
#endif
	

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

static void VulkanWindowResizeCallback(GLFWwindow* window, int width, int height)
{
	hyVulkanContext::Get().ResizeFrameBuffer(width, height);
}

hyVulkanContext::hyVulkanContext() : Instance(nullptr), DebugMessenger(nullptr), WindowSurface(nullptr) 
{
	std::cout << "\n --- Created Context! --- \n\n";
}

void hyVulkanContext::Init(hyWindow& window)
{
	CreateInstance();
	SetupDebugMessenger();
	CreateWindowSurface(window);

	glfwSetFramebufferSizeCallback(window.GetGLFWContext(), VulkanWindowResizeCallback);

	//SelectGfxDevice();
	GraphicsDevice.SelectPhysicalDevice();
	////CreateLogicalDevice();
	GraphicsDevice.CreateLogicalDevice();
	////CreateSwapChain();
	SwapChain.Create();
	////CreateImageViews();
	SwapChain.CreateImageViews();
	////CreateRenderPass();
	SwapChain.CreateRenderPass();
	////CreateGraphicsPipeline();
	GraphicsPipeline.Create();
	////CreateFrameBuffers();
	SwapChain.CreateFrameBuffers();
	////CreateCommandPool();
	SwapChain.CreateCommandPool();
	////CreateCommandBuffers();
	SwapChain.CreateCommandBuffers();

	CreateSyncObjects();
}

void hyVulkanContext::Shutdown()
{
	//ClearSwapChain();
	SwapChain.Destroy();
	
	VkDevice& device = GraphicsDevice.GetDevice();
	for (int i = 0; i < SwapChain.MAX_FRAMES_IN_FLIGHT; i++)
	{
		vkDestroySemaphore(device, SyncObjects.ImageAvailableSemahpores[i], nullptr);
		vkDestroySemaphore(device, SyncObjects.RenderFinishedSempaphores[i], nullptr);
		vkDestroyFence(device, SyncObjects.InFlightFences[i], nullptr);
	}

	vkDestroyCommandPool(device, SwapChain.GetCommandPool(), nullptr);
	vkDestroyDevice(GraphicsDevice.GetDevice(), nullptr);




	/* --- */
	//for (VkImageView& imageView : SwapChain.GetImageViews())
	//	vkDestroyImageView(device, imageView, nullptr);

	//vkDestroySwapchainKHR(device, SwapChain.GetSwapChain(), nullptr);
	//vkDestroyDevice(device, nullptr);

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
	VkResult result = vkCreateInstance(&onCreateInfo, nullptr, &Instance);
	if (result != VK_SUCCESS)
	{
		std::cerr << "Error: Failed to Create Vulkan Instance!" << std::endl;
		std::cerr << "Code: " << (int)result << std::endl;
	}
}

void hyVulkanContext::ResizeFrameBuffer(u32 width, u32 height)
{

}


//VkInstance& hyVulkanContext::GetInstance()
//{
//	return Instance;
//}

void hyVulkanContext::DrawFrame()
{
	VkDevice& device = GraphicsDevice.GetDevice();
	vkWaitForFences(device, 1, &SyncObjects.InFlightFences[SyncObjects.CurrentFrame], VK_TRUE, UINT64_MAX);
	

	u32 imageIndex;
	vkAcquireNextImageKHR(device, SwapChain.GetSwapChain(), UINT64_MAX, SyncObjects.ImageAvailableSemahpores[SyncObjects.CurrentFrame], VK_NULL_HANDLE, &imageIndex);
	/* params - 
	* 3rd     : timeout in nanoseconds for an image to become unavailable (UINT64_MAX -> disabled)
	* 4th/5th : objects to be signaled when presentation engine is finished using image (5th is fence)
	* 6th     : specifies var to output the index of swap chain image that has become available
	*/

	/* Submitting the CommandBuffer - queue submission and sync */
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	
	VkSemaphore waitSemaphores[] = { SyncObjects.ImageAvailableSemahpores[SyncObjects.CurrentFrame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT }; 
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	/* params - 0->2 specify which semaphores to wait on before execution begins,
	*  we want to wait with writing colors to the image until  it's available, 
	* so specify the stage of the  gfx pipeline that writes to the color attachment
	*/
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &SwapChain.GetCommandBuffers()[imageIndex];
	
	/* Which commands buffers actually submit for execution */
	VkSemaphore signalSemaphores[] = { SyncObjects.RenderFinishedSempaphores[SyncObjects.CurrentFrame] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;
	

	vkResetFences(device, 1, &SyncObjects.InFlightFences[SyncObjects.CurrentFrame]);

	if (vkQueueSubmit(GraphicsDevice.GetGraphicsQueue(), 1, &submitInfo, SyncObjects.InFlightFences[SyncObjects.CurrentFrame]) != VK_SUCCESS)
		std::cerr << "Error: Failed to draw command buffer!" << std::endl;

	/* Presentation */
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;
	/* two semaphores to wait on before presentation begins */

	VkSwapchainKHR swapChains[] = { SwapChain.GetSwapChain() };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;

	
	presentInfo.pResults = nullptr;
	/* allows you to speicy an array of VkResult values for every individual swap chain
	* if present successful */

	vkQueuePresentKHR(GraphicsDevice.GetPresentQueue(), &presentInfo);
	/* */


	SyncObjects.CurrentFrame = (SyncObjects.CurrentFrame + 1) % SwapChain.MAX_FRAMES_IN_FLIGHT;
}

void hyVulkanContext::SetupDebugMessenger()
{
	if (!USE_VK_VALIDATION_LAYERS)
		return;

	VkDebugUtilsMessengerCreateInfoEXT onCreateDebugInfo;
	PopulateDebugMessengerCreateInfo(onCreateDebugInfo);	
	VkResult result = CreateDebugMessengerExt(&onCreateDebugInfo, nullptr);
	if (result != VK_SUCCESS)
		std::cerr << "Error: " << result << " -- Unable to initialize Debug Messenger!" << std::endl;
		
	
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
		std::cerr << "Error: Window surface creation failed!" << std::endl;

}

std::vector<const char*>& hyVulkanContext::GetValidationLayers()
{
	return sValidationLayers;
}

//VkSurfaceKHR& hyVulkanContext::GetActiveSurface()
//{
//	return WindowSurface;
//}

hyVulkanSwapChain& hyVulkanContext::GetSwapChain()
{
	return SwapChain;
}


hyVulkanGraphicsPipeline& hyVulkanContext::GetGraphicsPipeline()
{
	return GraphicsPipeline;
}

hyVulkanGraphicsDevice& hyVulkanContext::GetGraphicsDevice()
{
	return GraphicsDevice;
}





//void hyVulkanContext::RecreateSwapChain()


/* 
*/














//VkShaderModule hyVulkanContext::CreateShaderModule(const std::vector<char>& spvCode)
//{
//	VkShaderModuleCreateInfo createInfo{};
//	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
//	createInfo.codeSize = spvCode.size();
//	createInfo.pCode = reinterpret_cast<const u32*>(spvCode.data());
//
//	VkShaderModule shaderModule;
//	if (vkCreateShaderModule(Device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
//		std::cerr << "Error: Failed to create shader module!" << std::endl;
//	
//	return shaderModule;
//}

///* */
//void hyVulkanContext::CreateRenderPass()





/* */
void hyVulkanContext::CreateSyncObjects()
{
	SyncObjects.InFlightFences.resize(SwapChain.MAX_FRAMES_IN_FLIGHT);
	SyncObjects.ImagesInFlight.resize(SwapChain.GetSwapChainImages().size(), VK_NULL_HANDLE);
	SyncObjects.ImageAvailableSemahpores.resize(SwapChain.MAX_FRAMES_IN_FLIGHT);
	SyncObjects.RenderFinishedSempaphores.resize(SwapChain.MAX_FRAMES_IN_FLIGHT);

	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	/* initialize fence in singaled state */
	VkDevice& device = GraphicsDevice.GetDevice();
	for (size_t i = 0; i < SwapChain.MAX_FRAMES_IN_FLIGHT; i++)
	{
		if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &SyncObjects.ImageAvailableSemahpores[i]) != VK_SUCCESS
			|| vkCreateSemaphore(device, &semaphoreInfo, nullptr, &SyncObjects.RenderFinishedSempaphores[i]) != VK_SUCCESS
			|| vkCreateFence(device, &fenceInfo, nullptr, &SyncObjects.InFlightFences[i]) != VK_SUCCESS)
		{
			std::cerr << "Error: Failed to create sync objects!" << std::endl;
		}
	}
	}


