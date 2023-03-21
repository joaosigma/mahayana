#include "objects.hpp"

#include "../stream.hpp"
#include "../scopedAction.hpp"

#include <cassert>

namespace hr::vulkan
{
	namespace
	{
		bool processUberShader(std::string& source, ShaderModule::ShaderType type)
		{
			if (source.empty())
				return false;

			std::string_view target;
			switch (type)
			{
				case ShaderModule::ShaderType::Vertex:
					target = "#stage (vertex)";
					break;
				case ShaderModule::ShaderType::Fragment:
					target = "#stage (fragment)";
					break;
				case ShaderModule::ShaderType::Compute:
					target = "#stage (compute)";
					break;
				default:
					return false;
			}

			size_t curIndex{0};
			size_t maxIndex{source.size()};

			auto isPragmaStage = [&curIndex, &maxIndex, &source](std::string_view stageType) -> bool
			{
				//all stages must start with '#'
				if (source[curIndex] != '#')
					return false;

				//it's not a pragma if it isn't at the beginning of the line
				if ((curIndex != 0) && (source[curIndex - 1] != '\n'))
					return false;

				if (stageType.empty())
					stageType = "#stage";

				if ((maxIndex - curIndex) < stageType.size())
					return false;

				if (std::memcmp(source.data() + curIndex, stageType.data(), stageType.size()) != 0)
					return false;

				//we are currently at the beginning of a line which matches a or the specified stage
				return true;
			};

			//find the target and until we do, replace text with spaces
			for (; curIndex < maxIndex; curIndex++)
			{
				switch (source[curIndex]) //ignore white spaces
				{
					case ' ':
					case '\t':
					case '\r':
					case '\n':
					case '\f':
					case '\v':
						continue;
					default:
						break;
				}

				//if the line isn't the intended stage line, just replace with a space and move on
				if (!isPragmaStage(target))
				{
					source[curIndex] = ' ';
					continue;
				}

				//we found the stage we want
				break;
			}

			//stage not found
			if (curIndex >= maxIndex)
				return false;

			//we're at the line that defines the start of the intended stage (but we also need to erase it from the source)
			for (; curIndex < maxIndex; curIndex++)
			{
				auto& curChar = source[curIndex];
				if (curChar == '\n')
				{
					curIndex++;
					break;
				}

				switch (curChar)
				{
					case ' ':
					case '\t':
					case '\r':
					case '\n':
					case '\f':
					case '\v':
						continue;
					default:
						curChar = ' ';
						continue;
				}
			}

			//we're at the start of the first line inside the intended stage, so lets move on until we find the next stage
			for (; (curIndex < maxIndex) && !isPragmaStage(""); curIndex++)
				;

			//if there wasn't another stages, we can leave
			if (curIndex >= maxIndex)
				return true;

			//all there's left to do is to fill remaining source with spaces
			for (; curIndex < maxIndex; curIndex++)
			{
				switch (source[curIndex])
				{
					case ' ':
					case '\t':
					case '\r':
					case '\n':
					case '\f':
					case '\v':
						continue;
					default:
						break;
				}

				source[curIndex] = ' ';
			}

			return true;
		}
	}

	Object<VkImageView>::~Object<VkImageView>()
	{
		if (!BaseObject::operator bool())
			return;

		assert(mDevice != VK_NULL_HANDLE);

		vkDestroyImageView(mDevice, mObj, nullptr);
		mObj = nullptr;
	}

	Object<VkImageView> Object<VkImageView>::gen2D(VkDevice device, VkImage sourceImg, VkFormat sourceFormat) noexcept
	{
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = sourceImg;
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = sourceFormat;
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		VkImageView imgView;
		if (vkCreateImageView(device, &createInfo, nullptr, &imgView) != VK_SUCCESS)
			return {};

		return Object(device, imgView);
	}

	bool Object<VkShaderModule>::compileShader(const std::filesystem::path& path,
	  const std::function<bool(const std::filesystem::path&, const std::filesystem::path&, ShaderType)>& compilerCb)
	{
		auto pathExt = path.extension();

		if (pathExt == ".ushader")
		{
			auto fileContent = hr::streams::FileStream::readEntireFileAsString(path);
			if (fileContent.empty())
				return false;

			for (auto shaderType : {ShaderType::Vertex, ShaderType::Fragment})
			{
				std::filesystem::path inputPath;
				{
					auto stageContent = fileContent;
					if (!processUberShader(stageContent, shaderType))
						continue;

					switch (shaderType)
					{
						case ShaderType::Vertex:
							inputPath = streams::FileStream::createTmpFile("shader_vstage", ".tmp");
							break;
						case ShaderType::Fragment:
							inputPath = streams::FileStream::createTmpFile("shader_fstage", ".tmp");
							break;
						default:
							inputPath = streams::FileStream::createTmpFile("shader_stage", ".tmp");
							break;
					}

					auto fs = streams::FileStream(inputPath, false, true);
					fs.write(stageContent.data(), stageContent.size());
				}
				ScopedAction deleteInputFile(
				  [inputPath]()
				  {
					  std::filesystem::remove(inputPath);
				  });

				auto outputPath = path;
				switch (shaderType)
				{
					case ShaderType::Vertex:
						outputPath.replace_extension(".v_spv");
						break;
					case ShaderType::Fragment:
						outputPath.replace_extension(".f_spv");
						break;
					default:
						continue;
				}

				if (!compilerCb(inputPath, outputPath, shaderType))
					return false;
			}

			return true;
		}

		if (pathExt == ".vshader")
		{
			auto outputPath = path;
			outputPath.replace_extension(".v_spv");

			if (!compilerCb(path, outputPath, ShaderType::Vertex))
				return false;

			return true;
		}

		if (pathExt == ".fshader")
		{
			auto outputPath = path;
			outputPath.replace_extension(".f_spv");

			if (!compilerCb(path, outputPath, ShaderType::Fragment))
				return false;

			return true;
		}

		if (pathExt == ".cshader")
		{
			auto outputPath = path;
			outputPath.replace_extension(".c_spv");

			if (!compilerCb(path, outputPath, ShaderType::Compute))
				return false;

			return true;
		}

		return false;
	}

	Object<VkShaderModule> Object<VkShaderModule>::loadShader(VkDevice device, const std::filesystem::path& path)
	{
		auto fileContent = hr::streams::FileStream::readEntireFileAsString(path);

		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = fileContent.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(fileContent.data());

		VkShaderModule shaderModule;
		if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
			return {};

		return Object(device, shaderModule);
	}

	Object<VkShaderModule>::~Object<VkShaderModule>()
	{
		if (!BaseObject::operator bool())
			return;

		assert(mDevice != VK_NULL_HANDLE);

		vkDestroyShaderModule(mDevice, mObj, nullptr);
		mObj = nullptr;
	}

	Object<VkFramebuffer> Object<VkFramebuffer>::gen(VkDevice device, VkRenderPass renderPass, std::initializer_list<VkImageView> imageViews, uint32_t imageViewsWidth, uint32_t imageViewsHeight)
	{
		std::array<VkImageView, 16> arrayBuffer;
		if (imageViews.size() <= arrayBuffer.size())
		{
			size_t curIndex{0};
			for (const auto& imageView : imageViews)
				arrayBuffer[curIndex++] = imageView;
			assert(curIndex == imageViews.size());

			return Object::gen(device, renderPass, std::span<VkImageView>{arrayBuffer.data(), imageViews.size()}, imageViewsWidth, imageViewsHeight);
		}
		else
		{
			std::vector<VkImageView> vecBuffer;
			vecBuffer.reserve(imageViews.size());

			for (const auto& imageView : imageViews)
				vecBuffer.push_back(imageView);
			assert(vecBuffer.size() == imageViews.size());

			return Object::gen(device, renderPass, vecBuffer, imageViewsWidth, imageViewsHeight);
		}
	}

	Object<VkFramebuffer> Object<VkFramebuffer>::gen(VkDevice device, VkRenderPass renderPass, std::span<VkImageView> imageViews, uint32_t imageViewsWidth, uint32_t imageViewsHeight) noexcept
	{
		if ((imageViews.size() <= 0) || (imageViewsWidth <= 0) || (imageViewsHeight <= 0))
			return {};

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = static_cast<uint32_t>(imageViews.size());
		framebufferInfo.pAttachments = imageViews.data();
		framebufferInfo.width = imageViewsWidth;
		framebufferInfo.height = imageViewsHeight;
		framebufferInfo.layers = 1;

		VkFramebuffer framebuffer;
		if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &framebuffer) != VK_SUCCESS)
			return {};

		return Object(device, framebuffer);
	}

	Object<VkFramebuffer>::~Object<VkFramebuffer>()
	{
		if (!BaseObject::operator bool())
			return;

		assert(mDevice != VK_NULL_HANDLE);

		vkDestroyFramebuffer(mDevice, mObj, nullptr);
		mObj = nullptr;
	}

	Object<VkPipeline>::Builder::Builder(VkDevice device) noexcept
	  : mDevice{device}
	{
		mVertexInputInfo = VkPipelineVertexInputStateCreateInfo{};
		mVertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		mVertexInputInfo.vertexBindingDescriptionCount = 0;
		mVertexInputInfo.pVertexBindingDescriptions = nullptr;
		mVertexInputInfo.vertexAttributeDescriptionCount = 0;
		mVertexInputInfo.pVertexAttributeDescriptions = nullptr;

		mInputAssembly = VkPipelineInputAssemblyStateCreateInfo{};
		mInputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		mInputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		mInputAssembly.primitiveRestartEnable = VK_FALSE;

		mViewportState = VkPipelineViewportStateCreateInfo{};
		mViewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		mViewportState.viewportCount = 0;
		mViewportState.scissorCount = 0;

		mRasterizer = VkPipelineRasterizationStateCreateInfo{};
		mRasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		mRasterizer.depthClampEnable = VK_FALSE;
		mRasterizer.rasterizerDiscardEnable = VK_FALSE;
		mRasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		mRasterizer.lineWidth = 1.0f;
		mRasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		mRasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
		mRasterizer.depthBiasEnable = VK_FALSE;
		mRasterizer.depthBiasConstantFactor = 0.0f;
		mRasterizer.depthBiasClamp = 0.0f;
		mRasterizer.depthBiasSlopeFactor = 0.0f;

		mMultisampling = VkPipelineMultisampleStateCreateInfo{};
		mMultisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		mMultisampling.sampleShadingEnable = VK_FALSE;
		mMultisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		mMultisampling.minSampleShading = 1.0f;
		mMultisampling.pSampleMask = nullptr;
		mMultisampling.alphaToCoverageEnable = VK_FALSE;
		mMultisampling.alphaToOneEnable = VK_FALSE;

		mColorBlendAttachment = VkPipelineColorBlendAttachmentState{};
		mColorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		mColorBlendAttachment.blendEnable = VK_FALSE;
		mColorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		mColorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		mColorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		mColorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		mColorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		mColorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
	}

	Object<VkPipeline>::Builder& Object<VkPipeline>::Builder::setupShader(ShaderModule module, ShaderModule::ShaderType shaderType)
	{
		assert(module);

		switch (shaderType)
		{
			case ShaderModule::ShaderType::Vertex:
				mStageShaders.vertexShaderModule = std::move(module);
				break;
			case ShaderModule::ShaderType::Fragment:
				mStageShaders.fragmentShaderModule = std::move(module);
				break;
			default:
				break;
		}

		return *this;
	};

	Object<VkPipeline>::Builder& Object<VkPipeline>::Builder::addVertexBinding(size_t bindingIndex, size_t stride)
	{
		if (mStageVertexInput.numBindings >= mStageVertexInput.bindings.size())
			return *this;

		auto& binding = mStageVertexInput.bindings[mStageVertexInput.numBindings];
		mStageVertexInput.numBindings++;

		binding = VkVertexInputBindingDescription{};
		binding.binding = static_cast<uint32_t>(bindingIndex);
		binding.stride = static_cast<uint32_t>(stride);
		binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return *this;
	}

	Object<VkPipeline>::Builder& Object<VkPipeline>::Builder::addVertexAttribute(size_t bindingIndex, size_t locationIndex, size_t offset, VkFormat format)
	{
		if (mStageVertexInput.numAttribs >= mStageVertexInput.attribs.size())
			return *this;

		auto& attrib = mStageVertexInput.attribs[mStageVertexInput.numAttribs];
		mStageVertexInput.numAttribs++;

		attrib.binding = static_cast<uint32_t>(bindingIndex);
		attrib.location = static_cast<uint32_t>(locationIndex);
		attrib.format = format;
		attrib.offset = static_cast<uint32_t>(offset);

		return *this;
	}

	Object<VkPipeline>::Builder& Object<VkPipeline>::Builder::setupInputAssembly(VkPrimitiveTopology topology, bool primitiveRestart)
	{
		mInputAssembly.topology = topology;
		mInputAssembly.primitiveRestartEnable = primitiveRestart ? VK_TRUE : VK_FALSE;
		return *this;
	}

	Object<VkPipeline>::Builder& Object<VkPipeline>::Builder::setupViewport(float x, float y, float width, float height, float minDepth, float maxDepth)
	{
		VkViewport viewport{};
		viewport.x = x;
		viewport.y = y;
		viewport.width = width;
		viewport.height = height;
		viewport.minDepth = minDepth;
		viewport.maxDepth = maxDepth;

		mStageViewport.viewport = std::move(viewport);
		return *this;
	}

	Object<VkPipeline>::Builder& Object<VkPipeline>::Builder::setupScissor(int32_t x, int32_t y, uint32_t width, uint32_t height)
	{
		VkRect2D scissor{};
		scissor.offset = VkOffset2D{x, y};
		scissor.extent = VkExtent2D{width, height};

		mStageViewport.scissor = std::move(scissor);
		return *this;
	}

	Object<VkPipeline> Object<VkPipeline>::Builder::build(VkRenderPass renderPass)
	{
		uint32_t numShaderStages{0};
		std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;
		{
			if (mStageShaders.vertexShaderModule.has_value())
			{
				VkPipelineShaderStageCreateInfo createInfo{};
				createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
				createInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
				createInfo.module = mStageShaders.vertexShaderModule->native();
				createInfo.pName = "main";

				shaderStages[numShaderStages] = std::move(createInfo);
				numShaderStages++;
			}

			if (mStageShaders.fragmentShaderModule.has_value())
			{
				VkPipelineShaderStageCreateInfo createInfo{};
				createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
				createInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
				createInfo.module = mStageShaders.fragmentShaderModule->native();
				createInfo.pName = "main";

				shaderStages[numShaderStages] = std::move(createInfo);
				numShaderStages++;
			}
		}

		if (mStageVertexInput.numBindings > 0)
		{
			mVertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(mStageVertexInput.numBindings);
			mVertexInputInfo.pVertexBindingDescriptions = mStageVertexInput.bindings.data();
		}
		if (mStageVertexInput.numAttribs > 0)
		{
			mVertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(mStageVertexInput.numAttribs);
			mVertexInputInfo.pVertexAttributeDescriptions = mStageVertexInput.attribs.data();
		}

		std::vector<VkDynamicState> mDynamicStates;
		{
			if (mStageViewport.viewport.has_value())
			{
				mViewportState.viewportCount = 1;
				mViewportState.pViewports = &(*mStageViewport.viewport);
			}
			else
			{
				mViewportState.viewportCount = 0;
				mDynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT);
			}

			if (mStageViewport.scissor.has_value())
			{
				mViewportState.scissorCount = 1;
				mViewportState.pScissors = &(*mStageViewport.scissor);
			}
			else
			{
				mViewportState.scissorCount = 0;
				mDynamicStates.push_back(VK_DYNAMIC_STATE_SCISSOR);
			}
		}

		VkPipelineDynamicStateCreateInfo dynamicState{};
		{
			dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
			if (!mDynamicStates.empty())
			{
				dynamicState.dynamicStateCount = static_cast<uint32_t>(mDynamicStates.size());
				dynamicState.pDynamicStates = mDynamicStates.data();
			}
		}

		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &mColorBlendAttachment;
		colorBlending.blendConstants[0] = colorBlending.blendConstants[1] = colorBlending.blendConstants[2] = 0.0f;
		colorBlending.blendConstants[3] = 0.0f;

		VkPipelineLayout pipelineLayout;
		{
			VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
			pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			pipelineLayoutInfo.setLayoutCount = 0;
			pipelineLayoutInfo.pSetLayouts = nullptr;
			pipelineLayoutInfo.pushConstantRangeCount = 0;
			pipelineLayoutInfo.pPushConstantRanges = nullptr;

			if (vkCreatePipelineLayout(mDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
				return {};
		}

		//we can finally create the pipeline

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = numShaderStages; //don't use the size of shaderStages
		pipelineInfo.pStages = shaderStages.data();
		pipelineInfo.pVertexInputState = &mVertexInputInfo;
		pipelineInfo.pInputAssemblyState = &mInputAssembly;
		pipelineInfo.pViewportState = &mViewportState;
		pipelineInfo.pRasterizationState = &mRasterizer;
		pipelineInfo.pMultisampleState = &mMultisampling;
		pipelineInfo.pDepthStencilState = nullptr;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = &dynamicState;
		pipelineInfo.layout = pipelineLayout;
		pipelineInfo.renderPass = renderPass;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
		pipelineInfo.basePipelineIndex = -1;

		VkPipeline graphicsPipeline;
		if (vkCreateGraphicsPipelines(mDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
		{
			vkDestroyPipelineLayout(mDevice, pipelineLayout, nullptr);
			return {};
		}

		return Object<VkPipeline>{mDevice, graphicsPipeline, pipelineLayout};
	}

	Object<VkPipeline>::~Object<VkPipeline>()
	{
		if (!BaseObject::operator bool())
			return;

		assert(mDevice != VK_NULL_HANDLE);

		vkDestroyPipeline(mDevice, mObj, nullptr);
		mObj = nullptr;

		vkDestroyPipelineLayout(mDevice, mPipelineLayout, nullptr);
		mPipelineLayout = nullptr;
	}

	Object<VkRenderPass>::Builder::Builder(VkDevice device) noexcept
	  : mDevice{device}
	{
	}

	Object<VkRenderPass>::Builder& Object<VkRenderPass>::Builder::addAttachment(VkFormat format)
	{
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = format;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		mAttachments.push_back(std::move(colorAttachment));
		return *this;
	}

	Object<VkRenderPass>::Builder& Object<VkRenderPass>::Builder::addSubpass()
	{
		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;

		mSubpasses.push_back(std::move(subpass));
		return *this;
	}

	Object<VkRenderPass>::Builder& Object<VkRenderPass>::Builder::addSubpassDependency()
	{
		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		mSubpassDependencies.push_back(std::move(dependency));
		return *this;
	}

	Object<VkRenderPass> Object<VkRenderPass>::Builder::build()
	{
		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(mAttachments.size());
		renderPassInfo.pAttachments = mAttachments.data();
		renderPassInfo.subpassCount = static_cast<uint32_t>(mSubpasses.size());
		renderPassInfo.pSubpasses = mSubpasses.data();
		renderPassInfo.dependencyCount = static_cast<uint32_t>(mSubpassDependencies.size());
		renderPassInfo.pDependencies = mSubpassDependencies.data();

		VkRenderPass renderPass;
		if (vkCreateRenderPass(mDevice, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
			return {};

		return Object<VkRenderPass>{mDevice, renderPass};
	}

	Object<VkRenderPass>::~Object<VkRenderPass>()
	{
		if (!BaseObject::operator bool())
			return;

		assert(mDevice != VK_NULL_HANDLE);

		vkDestroyRenderPass(mDevice, mObj, nullptr);
		mObj = nullptr;
	}

	Object<VkCommandPool> Object<VkCommandPool>::gen(VkDevice device, uint32_t queueFamilyIndex) noexcept
	{
		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = queueFamilyIndex;

		VkCommandPool commandPool;
		if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
			return {};

		return Object(device, commandPool);
	}

	Object<VkCommandPool>::~Object<VkCommandPool>()
	{
		if (!BaseObject::operator bool())
			return;

		assert(mDevice != VK_NULL_HANDLE);

		vkDestroyCommandPool(mDevice, mObj, nullptr);
		mObj = nullptr;
	}

	CommandBuffer Object<VkCommandPool>::allocateBuffer(bool destroyWithPool) noexcept
	{
		return CommandBuffer::gen(mDevice, mObj, destroyWithPool);
	}

	Object<VkCommandBuffer> Object<VkCommandBuffer>::gen(VkDevice device, VkCommandPool commandPool, bool destroyWithPool) noexcept
	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = commandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		if (vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer) != VK_SUCCESS)
			return {};

		return Object(device, commandPool, commandBuffer, destroyWithPool);
	}

	bool Object<VkCommandBuffer>::Recorder::doRenderPass(VkRenderPass renderPass, VkFramebuffer framebuffer, uint32_t renderWidth, uint32_t renderHeight, const std::function<void(Recorder&)>& cb)
	{
		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderPass;
		renderPassInfo.framebuffer = framebuffer;

		renderPassInfo.renderArea.offset = {0, 0};
		renderPassInfo.renderArea.extent = {renderWidth, renderHeight};

		VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(mCmdBuffer.native(), &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		cb(*this);

		vkCmdEndRenderPass(mCmdBuffer.native());

		return true;
	}

	Object<VkCommandBuffer>::Recorder& Object<VkCommandBuffer>::Recorder::copyBuffer(VkBuffer dest, VkBuffer source, size_t size) noexcept
	{
		return copyBuffer(dest, 0, source, 0, size);
	}

	Object<VkCommandBuffer>::Recorder& Object<VkCommandBuffer>::Recorder::copyBuffer(VkBuffer dest, size_t destOffset, VkBuffer source, size_t sourceOffset, size_t size) noexcept
	{
		VkBufferCopy copyRegion{};
		copyRegion.srcOffset = static_cast<VkDeviceSize>(sourceOffset);
		copyRegion.dstOffset = static_cast<VkDeviceSize>(destOffset);
		copyRegion.size = static_cast<VkDeviceSize>(size);
		vkCmdCopyBuffer(mCmdBuffer.native(), source, dest, 1, &copyRegion);

		return *this;
	}

	Object<VkCommandBuffer>::Recorder& Object<VkCommandBuffer>::Recorder::bindVertexBuffer(uint32_t bindingIndex, VkBuffer buffer, uint32_t bufferOffset) noexcept
	{
		VkDeviceSize offsets[] = {bufferOffset};
		vkCmdBindVertexBuffers(mCmdBuffer.native(), bindingIndex, 1, &buffer, offsets);

		return *this;
	}

	Object<VkCommandBuffer>::Recorder& Object<VkCommandBuffer>::Recorder::bindIndexBuffer(VkBuffer buffer, uint32_t bufferOffset, VkIndexType indexType) noexcept
	{
		vkCmdBindIndexBuffer(mCmdBuffer.native(), buffer, bufferOffset, indexType);
		return *this;
	}

	Object<VkCommandBuffer>::Recorder& Object<VkCommandBuffer>::Recorder::bindPipeline(VkPipeline pipeline) noexcept
	{
		vkCmdBindPipeline(mCmdBuffer.native(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
		return *this;
	}

	Object<VkCommandBuffer>::Recorder& Object<VkCommandBuffer>::Recorder::setViewport(float x, float y, float width, float height, float minDepth, float maxDepth) noexcept
	{
		VkViewport viewport{};
		viewport.x = x;
		viewport.y = y;
		viewport.width = width;
		viewport.height = height;
		viewport.minDepth = minDepth;
		viewport.maxDepth = maxDepth;

		vkCmdSetViewport(mCmdBuffer.native(), 0, 1, &viewport);
		return *this;
	}

	Object<VkCommandBuffer>::Recorder& Object<VkCommandBuffer>::Recorder::setScissor(int32_t x, int32_t y, uint32_t width, uint32_t height) noexcept
	{
		VkRect2D scissor{};
		scissor.offset = VkOffset2D{x, y};
		scissor.extent = VkExtent2D{width, height};

		vkCmdSetScissor(mCmdBuffer.native(), 0, 1, &scissor);
		return *this;
	}

	Object<VkCommandBuffer>::Recorder& Object<VkCommandBuffer>::Recorder::draw(uint32_t numIndices) noexcept
	{
		vkCmdDrawIndexed(mCmdBuffer.native(), numIndices, 1, 0, 0, 0);
		return *this;
	}

	bool Object<VkCommandBuffer>::Recorder::finish() noexcept
	{
		if (std::exchange(mFinished, true))
			return false;

		if (vkEndCommandBuffer(mCmdBuffer.native()) != VK_SUCCESS)
			return false;

		return true;
	}

	Object<VkCommandBuffer>::~Object() noexcept
	{
		if (mDestroyWithPool)
		{
			mObj = nullptr; //otherwise BaseObject will complain
			return;
		}

		if (!BaseObject::operator bool())
			return;

		assert(mDevice != VK_NULL_HANDLE);
		assert(mCommandPool != VK_NULL_HANDLE);

		vkFreeCommandBuffers(mDevice, mCommandPool, 1, &mObj);
		mObj = nullptr;
	}

	void Object<VkCommandBuffer>::reset() noexcept
	{
		vkResetCommandBuffer(mObj, 0);
	}

	std::optional<Object<VkCommandBuffer>::Recorder> Object<VkCommandBuffer>::record(bool oneTimeOnly) noexcept
	{
		VkCommandBufferBeginInfo cmdBeginInfo{};
		cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		cmdBeginInfo.flags = oneTimeOnly ? VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT : VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
		cmdBeginInfo.pInheritanceInfo = nullptr;

		if (vkBeginCommandBuffer(mObj, &cmdBeginInfo) != VK_SUCCESS)
			return std::nullopt;

		return Object<VkCommandBuffer>::Recorder{*this};
	}

	Object<VkSemaphore>::~Object<VkSemaphore>()
	{
		if (!BaseObject::operator bool())
			return;

		assert(mDevice != VK_NULL_HANDLE);

		vkDestroySemaphore(mDevice, mObj, nullptr);
		mObj = nullptr;
	}

	Object<VkSemaphore> Object<VkSemaphore>::gen(VkDevice device) noexcept
	{
		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkSemaphore semaphore;
		if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &semaphore) != VK_SUCCESS)
			return {};

		return Object(device, semaphore);
	}

	void Object<VkFence>::waitAll(VkDevice device, std::initializer_list<Object> fences) noexcept
	{
		std::array<VkFence, 16> arrayBuffer;
		if (fences.size() <= arrayBuffer.size())
		{
			size_t curIndex{0};
			for (const auto& fence : fences)
				arrayBuffer[curIndex++] = fence.native();
			assert(curIndex == fences.size());

			vkWaitForFences(device, static_cast<uint32_t>(fences.size()), arrayBuffer.data(), VK_TRUE, UINT64_MAX);
		}
		else
		{
			std::vector<VkFence> vecBuffer;
			vecBuffer.reserve(fences.size());

			for (const auto& fence : fences)
				vecBuffer.push_back(fence.native());
			assert(vecBuffer.size() == fences.size());

			vkWaitForFences(device, static_cast<uint32_t>(vecBuffer.size()), vecBuffer.data(), VK_TRUE, UINT64_MAX);
		}
	}

	Object<VkFence>::~Object<VkFence>()
	{
		if (!BaseObject::operator bool())
			return;

		assert(mDevice != VK_NULL_HANDLE);

		vkDestroyFence(mDevice, mObj, nullptr);
		mObj = nullptr;
	}

	Object<VkFence> Object<VkFence>::gen(VkDevice device, bool signaled) noexcept
	{
		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;

		VkFence fence;
		if (vkCreateFence(device, &fenceInfo, nullptr, &fence) != VK_SUCCESS)
			return {};

		return Object(device, fence);
	}

	Object<VkFence>& Object<VkFence>::wait() noexcept
	{
		vkWaitForFences(mDevice, 1, &mObj, VK_TRUE, UINT64_MAX);
		return *this;
	}

	Object<VkFence>& Object<VkFence>::reset() noexcept
	{
		vkResetFences(mDevice, 1, &mObj);
		return *this;
	}

	Object<VkDeviceMemory>::~Object<VkDeviceMemory>()
	{
		if (!BaseObject::operator bool())
			return;

		assert(mDevice != VK_NULL_HANDLE);

		vkFreeMemory(mDevice, mObj, nullptr);
		mObj = nullptr;
	}

	Object<VkDeviceMemory> Object<VkDeviceMemory>::gen(VkDevice device, VkDeviceMemory memory, size_t size) noexcept
	{
		return Object(device, memory, size);
	}

	std::optional<size_t> Object<VkDeviceMemory>::reserve(size_t size) noexcept
	{
		if (size <= 0)
			return std::nullopt;

		if (size > (mSpace.total - mSpace.used))
			return std::nullopt;

		auto offset = mSpace.used;
		mSpace.used += size;
		return offset;
	}

	std::optional<size_t> Object<VkDeviceMemory>::reserve(size_t size, size_t alignment) noexcept
	{
		assert(alignment > 0);

		if (size <= 0)
			return std::nullopt;

		if (mSpace.used <= 0) //first one
		{
			if (size > mSpace.total)
				return std::nullopt;

			mSpace.used += size;
			return 0; //since we're at the beginning, the alignment isn't required (it will always match)
		}

		auto alignedOffset = ((mSpace.used + (alignment - 1)) & ~(alignment - 1)); //must align to requested alignment
		if ((alignedOffset >= mSpace.total) || (size > (mSpace.total - alignedOffset)))
			return std::nullopt;

		mSpace.used = alignedOffset + size;
		return alignedOffset;
	}

	Object<VkBuffer>::~Object<VkBuffer>()
	{
		if (!BaseObject::operator bool())
			return;

		assert(mDevice != VK_NULL_HANDLE);

		vkDestroyBuffer(mDevice, mObj, nullptr);
		mObj = nullptr;
		mSize = 0;
	}

	Object<VkBuffer> Object<VkBuffer>::gen(VkDevice device, size_t size, VkBufferUsageFlags usage) noexcept
	{
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = static_cast<VkDeviceSize>(size);
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VkBuffer buffer;
		if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
			return {};

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

		return Object(device, buffer, size, memRequirements);
	}

	bool Object<VkBuffer>::allocate(Object<VkDeviceMemory>& memory) const noexcept
	{
		auto offset = memory.reserve(mMemRequirements.size, mMemRequirements.alignment);
		if (!offset)
			return false;

		if (vkBindBufferMemory(mDevice, mObj, memory.native(), *offset) != VK_SUCCESS)
			return false;

		return true;
	}
}