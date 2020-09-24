#include "hypch.h"
#include "VulkanGraphicsPipeline.h"
#include "Core/AssetLoader.h"
#include "Platform/Vulkan/VulkanContext.h"
#include <iostream>

void hyVulkanGraphicsPipeline::Create()
{
	std::vector<char> vertShaderCode = hyAssetLoader::ReadFile("C:/dev/hyperspace-engine/HyperspaceEngine/Resources/Shaders/vert.spv");
	std::vector<char> fragShaderCode = hyAssetLoader::ReadFile("C:/dev/hyperspace-engine/HyperspaceEngine/Resources/Shaders/frag.spv");

	/* local testing vert + frag shaders */
	VkShaderModule vertShaderModule = CreateShaderModule(vertShaderCode);
	VkShaderModule fragShaderModule = CreateShaderModule(fragShaderCode);

	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;

	/* specify: shader module containing spv code, function to invoke: */
	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main";

	/* final (optional) member, pSpecializationInfo:
	   allows you to specify values for shader constraints. (This is more efficient than
	   configuring shader variables at render-time, since compiler can do more optimizing)
	*/

	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

	/* Fixed Pipeline: */
	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 0;
	vertexInputInfo.pVertexBindingDescriptions = nullptr;
	vertexInputInfo.vertexAttributeDescriptionCount = 0;
	vertexInputInfo.pVertexAttributeDescriptions = nullptr;


	/* Input Assembly: What kind of geometry will be drawn from the vertices and if primitive restart should be enabled.
	* VK_PRIMITIVE_TOPOLOGY_POINT_LIST     : points from vertices
	* VK_PRIMITIVE_TOPOLOGY_LINE_LIST      : line from every 2 vertices without reuse
	* VK_PRIMITIVE_TOPOLOGY_LINE_STRIP     : end of vertex of every line is used as start vertex for next line
	* VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST  : triangle from every 3 vertices (no reuse)
	* VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP : second and third vertex of every triangle used in next
	*/

	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	/* Viewports and scissors
	*  A viewport is the region of the framebuffer the output will be rendered to.
	* Almost always (0, 0) to (width, height).
	*/

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)hyVulkanContext::Get().GetSwapChain().GetExtent().width;
	viewport.height = (float)hyVulkanContext::Get().GetSwapChain().GetExtent().height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	/* we want to draw to entire framebuffer, so: */
	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = hyVulkanContext::Get().GetSwapChain().GetExtent();

	/* It is possble to use multiple viewports and scissor rectangles on some gfx cards,
		but we'll skip doing any extension support checks to do that for now.
	*/
	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	/* Rasterizer: turns geometry from vertex shader into fragments that can be colored by
	*  fragment shader. Performs depth testing, face culling, and scissor test, and can
	*  output fragments that fill entirepolygons, or just the edges(wireframe), if configured to.
	*/
	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;							/* if true: fragments beyond near and far planes are clamped to them instead of being
																	   discarded. (Useful for shadow maps) [Requires enabling GPU extension] */
	rasterizer.rasterizerDiscardEnable = VK_FALSE;					/* Geometry is never passed through rasterizer stage, effectively disables output to framebuffer. */

	/* polygonMode:
	*  VK_POLYGON_MODE_FILL  : fill area of polygon with fragments
	*  VK_POLYGON_MODE_LINE  : polygon edges are drawn as lines
	*  VK_POLYGON_MODE_POINT : polygon vertices are drawn as points
	*/
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;

	rasterizer.lineWidth = 1.0;										/* wideLines required to change this from 1.0 */
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;					/* Cull backFace (versus cull frontFace) */
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;					/* vertex-order for faces to be considered 'front-facing' */

	rasterizer.depthBiasEnable = VK_FALSE;							/* rasterizer can alter the depth values by adding a constant or */
	rasterizer.depthBiasClamp = 0.0f;								/* biasing them based on a fragment's slope */
	rasterizer.depthBiasSlopeFactor = 0.0f;

	/* Multisampling */

	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f;
	multisampling.pSampleMask = nullptr;
	multisampling.alphaToCoverageEnable = VK_FALSE;
	multisampling.alphaToOneEnable = VK_FALSE;

	/* */
	//VkPipelineDepthStencilStateCreateInfo

	/* Color blending : After fragment shader has returned a color, it needs to be combined with the color
	*  that is already in the framebuffer.
	*  Two ways to do this - Mix old and new to produce final, or combine old and new using bitwise operation
	*/
	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
		VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

	/* if blendEnable is false, the new color from frag shader is passed through unmodified,
	   overwritting old data. if blendEnable is true, the two mixing operations are performed
	   to compute the new color. (bitwise AND between result-color and colorWriteMask)
	*/
	colorBlendAttachment.blendEnable = VK_TRUE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

	/* To use bitwise combination blending, you should set logicOpEnable to VK_TRUE, and
	   specify operation in logicOp field.
	*/
	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;

	/* Dynamic State : Viewport size, line width, and blend constants can be changed without
	   recreating the pipeline. (these values now specified at draw-time)
	*/
	VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_LINE_WIDTH };

	VkPipelineDynamicStateCreateInfo dynamicState;
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = 2;
	dynamicState.pDynamicStates = dynamicStates;

	/*
	*/
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0;
	pipelineLayoutInfo.pSetLayouts = nullptr;
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = nullptr;

	if (vkCreatePipelineLayout(hyVulkanContext::Get().GetGraphicsDevice().GetDevice(), &pipelineLayoutInfo, nullptr, &Layout) != VK_SUCCESS)
		std::cerr << "Error: Failed to create Pipeline layout!" << std::endl;

	/* Create Graphics pipeline */
	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;								/* Vertex, Fragment */
	pipelineInfo.pStages = shaderStages;

	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = nullptr;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = nullptr;

	pipelineInfo.layout = Layout;

	/* reference to the render pass and index of sub pass where gfx pipeline will be used */
	pipelineInfo.renderPass = hyVulkanContext::Get().GetSwapChain().GetRenderPass();															
	pipelineInfo.subpass = 0;
	/* Pipelines can be 'based off of' eachother for less expensive setup */
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineInfo.basePipelineIndex = -1;

	if (vkCreateGraphicsPipelines(hyVulkanContext::Get().GetGraphicsDevice().GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &Pipeline) != VK_SUCCESS)
		std::cerr << "Error: Failed to create Graphics Pipeline!" << std::endl;

	vkDestroyShaderModule(hyVulkanContext::Get().GetGraphicsDevice().GetDevice(), fragShaderModule, nullptr);
	vkDestroyShaderModule(hyVulkanContext::Get().GetGraphicsDevice().GetDevice(), vertShaderModule, nullptr);
}

void hyVulkanGraphicsPipeline::Destroy()
{
	VkDevice& device = hyVulkanContext::Get().GetGraphicsDevice().GetDevice();
	vkDestroyPipeline(device, Pipeline, nullptr);
	vkDestroyPipelineLayout(device, Layout, nullptr);
}

VkShaderModule hyVulkanGraphicsPipeline::CreateShaderModule(const std::vector<char>& spvCode)
{
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = spvCode.size();
	createInfo.pCode = reinterpret_cast<const u32*>(spvCode.data());

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(hyVulkanContext::Get().GetGraphicsDevice().GetDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
		std::cerr << "Error: Failed to create shader module!" << std::endl;

	return shaderModule;
}

VkPipelineLayout& hyVulkanGraphicsPipeline::GetLayout()
{
	return Layout;
}

VkPipeline& hyVulkanGraphicsPipeline::GetPipeline()
{
	return Pipeline;
}