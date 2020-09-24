#include "hypch.h"
#include "VulkanSwapChain.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanGraphicsDevice.h"
#include <iostream>

#include "Core/Core.h"

void hyVulkanSwapChain::Create()
{
	hySwapChainSupportDetails swapChainSupport = QuerySupport(hyVulkanContext::Get().GetGraphicsDevice().GetPhysicalDevice());

	VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.Formats);
	VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.PresentModes);
	VkExtent2D extent = ChooseSwapExtent(swapChainSupport.Capabilities);

	/* Min image count for SwapChain to function + added buffer of 1*/
	u32 minNumImages = swapChainSupport.Capabilities.minImageCount + 1;

	if (swapChainSupport.Capabilities.maxImageCount > 0 && minNumImages > swapChainSupport.Capabilities.maxImageCount)
		minNumImages = swapChainSupport.Capabilities.maxImageCount;

	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = hyVulkanContext::Get().GetActiveSurface();

	createInfo.minImageCount = minNumImages;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;									/* This is 1 unless doing stereoscopic rendering */
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;		/* What we use images in SwapChain for */
																		/* (For postprocesing you might use VK_IMAGE_USAGE_TRANSFER_DST_BIT) */
	hyQueueFamilyIndices indices = hyVulkanContext::Get().GetGraphicsDevice().FindQueueFamilies(hyVulkanContext::Get().GetGraphicsDevice().GetPhysicalDevice());
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

	VkDevice& device = hyVulkanContext::Get().GetGraphicsDevice().GetDevice();

	if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &SwapChain) != VK_SUCCESS)
		std::cerr << "Error: Failed to create swap chain!" << std::endl;

	/* */
	vkGetSwapchainImagesKHR(device, SwapChain, &minNumImages, nullptr);
	SwapChainImages.resize(minNumImages);
	vkGetSwapchainImagesKHR(device, SwapChain, &minNumImages, SwapChainImages.data());

	/* */
	ImageFormat = surfaceFormat.format;
	Extent = extent;
}

void hyVulkanSwapChain::Destroy()
{
	for (auto frameBuffer : FrameBuffers)
		vkDestroyFramebuffer(hyVulkanContext::Get().GetGraphicsDevice().GetDevice(), frameBuffer, nullptr);


	vkFreeCommandBuffers(hyVulkanContext::Get().GetGraphicsDevice().GetDevice(), CommandPool, static_cast<u32>(CommandBuffers.size()), CommandBuffers.data());

	hyVulkanContext::Get().GetGraphicsPipeline().Destroy();

	vkDestroyRenderPass(hyVulkanContext::Get().GetGraphicsDevice().GetDevice(), RenderPass, nullptr);

	for (auto imageView : ImageViews)
		vkDestroyImageView(hyVulkanContext::Get().GetGraphicsDevice().GetDevice(), imageView, nullptr);

	vkDestroySwapchainKHR(hyVulkanContext::Get().GetGraphicsDevice().GetDevice(), SwapChain, nullptr);


}

void hyVulkanSwapChain::Clear()
{
	VkDevice& device = hyVulkanContext::Get().GetGraphicsDevice().GetDevice();

	for (size_t i = 0; i < FrameBuffers.size(); i++)
		vkDestroyFramebuffer(device, FrameBuffers[i], nullptr);

	vkFreeCommandBuffers(device, CommandPool, static_cast<u32>(CommandBuffers.size()), CommandBuffers.data());
	vkDestroyPipeline(device, hyVulkanContext::Get().GetGraphicsPipeline().GetPipeline(), nullptr);
	vkDestroyPipelineLayout(device, hyVulkanContext::Get().GetGraphicsPipeline().GetLayout(), nullptr);
	vkDestroyRenderPass(device, RenderPass, nullptr);

	for (size_t i = 0; i < ImageViews.size(); i++)
		vkDestroyImageView(device, ImageViews[i], nullptr);

	vkDestroySwapchainKHR(device, SwapChain, nullptr);
}

/* Recreate swap chain (ex. window resizing events) */
void hyVulkanSwapChain::Recreate()
{
	//vkDeviceWaitIdle(hyVulkanContext::Get().GetGraphicsDevice().GetDevice());
	//ClearSwapChain();

	//CreateSwapChain();
	//CreateImageViews();
	//CreateRenderPass();
	//CreateGraphicsPipeline();
	//CreateFrameBuffers();
	//CreateCommandBuffers();
}

/* Possible Swap Present Modes are:
*  VK_PRESENT_MODE_IMMEDIATE_KHR    - Immediate swap, leads to screen-tearing
*  VK_PRESENT_MODE_FIFO_KHR         - Queue-based swap, if queue is full then program has to wait (provides "sync")
*  VK_PRESENT_MODE_FIFO_RELAXED_KHR - Similar to FIFO, except if the application is late and queue is empty the image is
*									  displayed immediately upon arrival.
*  VK_PRESENT_MODE_MAILBOX_KHR      - Also like FIFO, but instead of blocking when queue is full, images already in the
*									  queue get replaced with newer ones. Can be used to implement triple-buffering.
*/
VkPresentModeKHR hyVulkanSwapChain::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
	for (const VkPresentModeKHR& availablePresentMode : availablePresentModes)
	{
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			return availablePresentMode;
	}
	return VK_PRESENT_MODE_FIFO_KHR;
}


/* Swap Extent - resolution of swap chain images, almost always equal to monitor resolution.
*  Some window managers use a different resolution, and this is indicated using UINT32_MAX as its value. */
VkExtent2D hyVulkanSwapChain::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
	if (capabilities.currentExtent.width != UINT32_MAX)
		return capabilities.currentExtent;

	std::cout << "Warning: Window Manager using non-native swap image resolution!" << std::endl;
	VkExtent2D actualExtent = { 800, 600 };

	actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
	actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));
	return actualExtent;

}



VkSwapchainKHR& hyVulkanSwapChain::GetSwapChain()
{
	return SwapChain;
}

VkFormat& hyVulkanSwapChain::GetImageFormat()
{
	return ImageFormat;
}

std::vector<VkImageView>& hyVulkanSwapChain::GetImageViews()
{
	return ImageViews;
}

std::vector<VkImage>& hyVulkanSwapChain::GetSwapChainImages()
{
	return SwapChainImages;
}

VkExtent2D& hyVulkanSwapChain::GetExtent()
{
	return Extent;
}

VkRenderPass& hyVulkanSwapChain::GetRenderPass()
{
	return RenderPass;
}

VkCommandPool& hyVulkanSwapChain::GetCommandPool()
{
	return CommandPool;
}

std::vector<VkCommandBuffer>& hyVulkanSwapChain::GetCommandBuffers()
{
	return CommandBuffers;
}

std::vector<VkFramebuffer>& hyVulkanSwapChain::GetFrameBuffers()
{
	return FrameBuffers;
}

void hyVulkanSwapChain::CreateFrameBuffers()
{
	FrameBuffers.resize(ImageViews.size());

	for (size_t i = 0; i < ImageViews.size(); i++)
	{
		VkImageView attachments[] = { ImageViews[i] };

		VkFramebufferCreateInfo createInfo{};

		createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		createInfo.renderPass = RenderPass;
		createInfo.attachmentCount = 1;
		createInfo.pAttachments = attachments;
		createInfo.width = Extent.width;
		createInfo.height = Extent.height;
		createInfo.layers = 1;

		if (vkCreateFramebuffer(hyVulkanContext::Get().GetGraphicsDevice().GetDevice(), &createInfo, nullptr, &FrameBuffers[i]) != VK_SUCCESS)
			std::cerr << "Error: Failed to create frame buffer!" << std::endl;
	}
}

/* */
VkSurfaceFormatKHR hyVulkanSwapChain::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	for (const VkSurfaceFormatKHR& availableFormat : availableFormats)
	{
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			return availableFormat;
	}
	return availableFormats[0];					/* */
}


void hyVulkanSwapChain::CreateRenderPass()
{
	/* Just a single color buffer represented by one of the images from the swap chain : */
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = hyVulkanContext::Get().GetSwapChain().GetImageFormat();
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;

	/* LoadOp and storeOp determine what to do with data in attachment before and after rending -
	* VK_ATTACHMENT_...
	* LOAD_OP_LOAD      : Preserve the existing contents of attachment
	* LOAD_OP_CLEAR     : Clear the values to a constant at the start
	* LOAD_OP_DONT_CARE : Existing contents undefined, we don't care (why do I find this funny?)
	* And for storeOp -
	*	VK_ATTACHMENT_STORE_OP_STORE : Rendered contents remain in memory and can be used later
	*	VK_ATTACHMENT_STORE_OP_DONT_CARE : Contents of framebuffer will be undefined after rendering
	*/
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

	/* Not currently using stencil buffer */
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

	/* Common layouts are: VK_IMAGE_LAYOUT_...
	* COLOR_ATTACHMENT_OPTIMAL : Images used as color attachment
	* PRESENT_SRC_KHR          : Images to be presented in swap chain
	* TRANSFER_DST_OPTIMAL     : Images to be used as dest. for mem. copy operation
	*/
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	/* Subpasses and attachment references - A single render pass can consist of multiple subpasses
	*
	*/
	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	/* */
	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;	/* Vulkan may support Compute subpasses in the future */
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	/* Subpass dependencies - subpasses automatically handle image layout transitions,
	* which are controlled by subpass dependencies. (memory and exec. deps between subpasses)
	*/
	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	/* src : implicit subpass before or after render pass
	* dst  : index 0, our subpass, the first and only one
	* dst must always be higher than src to prevent dependency graph cycles
	*/

	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	/* operations to wait on the stages in which these ops occur */

	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;

	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	if (vkCreateRenderPass(hyVulkanContext::Get().GetGraphicsDevice().GetDevice(), &renderPassInfo, nullptr, &RenderPass) != VK_SUCCESS)
		std::cerr << "Error: Failed to create render pass!" << std::endl;

}

hySwapChainSupportDetails hyVulkanSwapChain::QuerySupport(VkPhysicalDevice& device)
{
	VkSurfaceKHR surface = hyVulkanContext::Get().GetActiveSurface();

	hySwapChainSupportDetails details;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.Capabilities);

	u32 numFormats;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &numFormats, nullptr);

	if (numFormats != 0)
	{
		details.Formats.resize(numFormats);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &numFormats, details.Formats.data());
	}

	u32 numPresentModes;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &numPresentModes, nullptr);

	if (numPresentModes != 0)
	{
		details.PresentModes.resize(numPresentModes);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &numPresentModes, details.PresentModes.data());
	}
	return details;
}


/* Command Buffers */
void hyVulkanSwapChain::CreateCommandPool()
{
	/* FIX THIS LINE SOMEHOW */
	hyQueueFamilyIndices queueFamilyIndices = hyVulkanContext::Get().GetGraphicsDevice().FindQueueFamilies(hyVulkanContext::Get().GetGraphicsDevice().GetPhysicalDevice());
	VkCommandPoolCreateInfo createInfo;

	createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	createInfo.queueFamilyIndex = queueFamilyIndices.GraphicsFamily.value();
	createInfo.flags = 0;
	createInfo.pNext = nullptr;

	/* Two possible command queues -
	* VK_COMMAND_POOL_CREATE_TRANSIENT_BIT : Hint command buffers are rerecorded with new commands very
	*									     often (can affect memory alloc. behavior)
	* VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT : Allow buffers to be rerecorded invdividually, without this
	*                                                   flag they have to be reset collectively
	*/
	if (vkCreateCommandPool(hyVulkanContext::Get().GetGraphicsDevice().GetDevice(), &createInfo, nullptr, &CommandPool) != VK_SUCCESS)
		std::cerr << "Error: Failed to create command pool!" << std::endl;
}

void hyVulkanSwapChain::CreateCommandBuffers()
{
	CommandBuffers.resize(FrameBuffers.size());

	VkCommandBufferAllocateInfo allocInfo;
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = CommandPool;

	/* Command Buffers can be either primary or secondary -
	* VK_COMMAND_BUFFER_LEVEL_PRIMARY : Can be submitted for execution, but not called by other cmd buffers
	* VK_COMMAND_BUFFER_LEVEL_SECONDARY : Cannot be submitted for execution directly, but can be called by primaries
	*/
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (u32)CommandBuffers.size();
	allocInfo.pNext = nullptr;

	if (vkAllocateCommandBuffers(hyVulkanContext::Get().GetGraphicsDevice().GetDevice(), &allocInfo, CommandBuffers.data()) != VK_SUCCESS)
		std::cerr << "Error: Failed to create command buffers!" << std::endl;

	for (size_t i = 0; i < CommandBuffers.size(); i++)
	{
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = 0;
		/* beginInfo.flags - VK_COMMAND_BUFFER_USAGE_...
		* ONE_TIME SUBMIT_BIT : Cmd buffer rerecorded right after executing it once
		* RENDER_PASS_CONTINUE_BIT : Secondary cmd buffer - entirely within one render pass
		* SIMULTANEOUS_USE_BIT : Cmd buffer can be submitted while it is also pending execution
		*/
		beginInfo.pInheritanceInfo = nullptr;

		if (vkBeginCommandBuffer(CommandBuffers[i], &beginInfo) != VK_SUCCESS)
			std::cerr << "Error: Failed to begin recording command buffer!" << std::endl;

		/* Starting a render pass */
		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = RenderPass;
		renderPassInfo.framebuffer = FrameBuffers[i];

		/* size of render area */
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = Extent;

		/* values for VK_ATTACHMENT_LOAD_OP_CLEAR */
		VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(CommandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		/* VK_SUBPASS_CONTENTS_...
		* INLINE : render pass commands are in primary cmd buffer, no secondary cmd buffers needed
		* SECONDARY_COMMAND_BUFFERS : render pass will be executed from secondary cmd buffers
		*/

		/* Basic drawing commands */
		vkCmdBindPipeline(CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, hyVulkanContext::Get().GetGraphicsPipeline().GetPipeline());

		vkCmdDraw(CommandBuffers[i], 3, 1, 0, 0);
		/* params: vertexCount (even though we have no vertex buffer)
		* instanceCount, firstVertex, firstInstance
		*/
		vkCmdEndRenderPass(CommandBuffers[i]);

		if (vkEndCommandBuffer(CommandBuffers[i]) != VK_SUCCESS)
			std::cerr << "Error: Failed to record command buffer!" << std::endl;
	}

}

void hyVulkanSwapChain::CreateImageViews()
{
	ImageViews.resize(SwapChainImages.size());		/* resize list to fit all the image views */

	for (size_t i = 0; i < SwapChainImages.size(); i++)
	{
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = SwapChainImages[i];
		/* How image data should be interpreted: */
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = ImageFormat;
		/* Components let you swizzle color channels, ex: you could map all channels to red for a monochromatic texture */
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		/* subresourceRange: What image purpose is, how it should be accessed */
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(hyVulkanContext::Get().GetGraphicsDevice().GetDevice(), &createInfo, nullptr, &ImageViews[i]) != VK_SUCCESS)
			std::cerr << "Error: Failed to create image views!" << std::endl;

	}
	/* Note from vulkan-tutorial: If you were working on a stereographic 3D application,
	then you would create a swap chain with multiple layers. You could then create
	multiple image views for each image representing the views for the left and right
	eyes by accessing different layers.
	*/
	/* These are explicitly created, and so need to be destroyed in ShutDown() */

}