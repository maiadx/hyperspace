#pragma once
#include "Renderer/SwapChain.h"
#include <vulkan/vulkan.h>
#include <vector>

struct hySwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR Capabilities;
	std::vector<VkSurfaceFormatKHR> Formats;
	std::vector<VkPresentModeKHR> PresentModes;
};


class hyVulkanSwapChain : public hySwapChain
{
	/* Swap Chain Properties */
	VkSwapchainKHR SwapChain;
	VkFormat ImageFormat;
	VkExtent2D Extent;
	VkRenderPass RenderPass;
	std::vector<VkImage> SwapChainImages;
	std::vector<VkFramebuffer> FrameBuffers;
	/* Image Views */
	std::vector<VkImageView> ImageViews;

	/* Command Buffers */
	VkCommandPool CommandPool;
	std::vector<VkCommandBuffer> CommandBuffers;

	/* */
	VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

public:
	const int MAX_FRAMES_IN_FLIGHT = 2;

	void Create();
	void Destroy();
	void Clear();
	void Recreate();

	hySwapChainSupportDetails QuerySupport(VkPhysicalDevice& device);
	VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	VkSwapchainKHR& GetSwapChain();
	VkFormat& GetImageFormat();
	VkExtent2D& GetExtent();
	VkRenderPass& GetRenderPass();

	/* Render Pass*/
	void CreateRenderPass();

	/* Image Views */
	void CreateImageViews();
	std::vector<VkImageView>& GetImageViews();
	std::vector<VkImage>& GetSwapChainImages();
	/* Frame Buffers */
	void CreateFrameBuffers();
	std::vector<VkFramebuffer>& GetFrameBuffers();

	/* Command Buffers */
	void CreateCommandPool();
	void CreateCommandBuffers();

	VkCommandPool& GetCommandPool();
	std::vector<VkCommandBuffer>& GetCommandBuffers();
	
};

