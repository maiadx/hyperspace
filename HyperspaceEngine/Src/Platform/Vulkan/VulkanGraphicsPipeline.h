#pragma once
#include "Renderer/GraphicsPipeline.h"
#include <vector>
#include <vulkan/vulkan.h>

class hyVulkanGraphicsPipeline : public hyGraphicsPipeline
{
	/* Graphics Pipeline */
	VkPipelineLayout Layout;
	VkPipeline Pipeline;

public:
	void Create() override;
	void Destroy() override;
	VkShaderModule CreateShaderModule(const std::vector<char>& spvCode);

	VkPipelineLayout& GetLayout();
	VkPipeline& GetPipeline();
};

