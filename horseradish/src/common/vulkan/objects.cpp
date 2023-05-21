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

	Object<VkImageView> Object<VkImageView>::genDepth(VkDevice device, VkImage sourceImg, VkFormat sourceFormat) noexcept
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
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		VkImageView imgView;
		if (vkCreateImageView(device, &createInfo, nullptr, &imgView) != VK_SUCCESS)
			return {};

		return Object(device, imgView);
	}
	
	Object<VkImage>::~Object<VkImage>()
	{
		if (!BaseObject::operator bool())
			return;

		assert(mDevice != VK_NULL_HANDLE);

		vkDestroyImage(mDevice, mObj, nullptr);
		mObj = nullptr;
	}

	Object<VkImage> Object<VkImage>::gen2D(VkDevice device, size_t width, size_t height, bool withMipMaps, VkFormat format) noexcept
	{
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = static_cast<uint32_t>(width);
		imageInfo.extent.height = static_cast<uint32_t>(height);
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = withMipMaps ? static_cast<uint32_t>(calculateNumMipMaps(width, height)) : 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = format;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.flags = 0;

		VkImage image;
		if (vkCreateImage(device, &imageInfo, nullptr, &image) != VK_SUCCESS)
			return {};

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(device, image, &memRequirements);

		return Object(device, image, memRequirements);
	}

	Object<VkImage> Object<VkImage>::genDepth(VkDevice device, size_t width, size_t height, VkFormat format) noexcept
	{
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = static_cast<uint32_t>(width);
		imageInfo.extent.height = static_cast<uint32_t>(height);
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = format;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.flags = 0;

		VkImage image;
		if (vkCreateImage(device, &imageInfo, nullptr, &image) != VK_SUCCESS)
			return {};

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(device, image, &memRequirements);

		return Object(device, image, memRequirements);
	}

	bool Object<VkImage>::allocate(Object<VkDeviceMemory>& memory) const noexcept
	{
		auto offset = memory.reserve(mMemRequirements.size, mMemRequirements.alignment);
		if (!offset)
			return false;

		if (vkBindImageMemory(mDevice, mObj, memory.native(), *offset) != VK_SUCCESS)
			return false;

		return true;
	}

	VkSamplerCreateInfo Object<VkSampler>::defaultSampler() noexcept
	{
		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_NEAREST;
		samplerInfo.minFilter = VK_FILTER_NEAREST;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.anisotropyEnable = VK_FALSE;
		samplerInfo.maxAnisotropy = 1.0f;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;

		return samplerInfo;
	}

	Object<VkSampler> Object<VkSampler>::create(VkDevice device, VkFilter minFilter, VkFilter magFilter) noexcept
	{
		auto samplerInfo = defaultSampler();
		samplerInfo.magFilter = magFilter;
		samplerInfo.minFilter = minFilter;

		VkSampler sampler;
		if (vkCreateSampler(device, &samplerInfo, nullptr, &sampler) != VK_SUCCESS)
			return {};

		return Object(device, sampler);
	}

	Object<VkSampler> Object<VkSampler>::createAnisotropic(VkDevice device, VkFilter minFilter, VkFilter magFilter, float maxAnisotropy) noexcept
	{
		auto samplerInfo = defaultSampler();
		samplerInfo.magFilter = magFilter;
		samplerInfo.minFilter = minFilter;
		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = maxAnisotropy;

		VkSampler sampler;
		if (vkCreateSampler(device, &samplerInfo, nullptr, &sampler) != VK_SUCCESS)
			return {};

		return Object(device, sampler);
	}

	Object<VkSampler>::~Object<VkSampler>()
	{
		if (!BaseObject::operator bool())
			return;

		assert(mDevice != VK_NULL_HANDLE);

		vkDestroySampler(mDevice, mObj, nullptr);
		mObj = nullptr;
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
	}

	Object<VkPipeline>::Builder& Object<VkPipeline>::Builder::addDescriptorSetLayout(VkDescriptorSetLayout dsetLayout)
	{
		mDSetLayouts.push_back(dsetLayout);
		return *this;
	}

	Object<VkPipeline>::Builder& Object<VkPipeline>::Builder::addPushConstant(size_t offset, size_t size, VkShaderStageFlags stageFlags)
	{
		VkPushConstantRange pushRange{};
		pushRange.offset = static_cast<uint32_t>(offset);
		pushRange.size = static_cast<uint32_t>(size);
		pushRange.stageFlags = stageFlags;
		mPushConstants.push_back(std::move(pushRange));

		return *this;
	}

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

	Object<VkPipeline>::Builder& Object<VkPipeline>::Builder::setupViewport(float width, float height, bool flipY)
	{
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = !flipY ? 0.0f : height;
		viewport.width = width;
		viewport.height = !flipY ? height : -height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		mStageViewport.viewport = std::move(viewport);
		return *this;
	}

	Object<VkPipeline>::Builder& Object<VkPipeline>::Builder::setupViewport(float x, float y, float width, float height)
	{
		return setupViewport(x, y, width, height, 0.0f, 1.0f);
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

	Object<VkPipeline>::Builder& Object<VkPipeline>::Builder::setupDepth(bool enableDepthTest, bool enableDepthWrite)
	{
		if (!mDepthStencil)
		{
			VkPipelineDepthStencilStateCreateInfo depthStencilDefault{};
			depthStencilDefault.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
			depthStencilDefault.depthTestEnable = VK_FALSE;
			depthStencilDefault.depthWriteEnable = VK_FALSE;
			depthStencilDefault.depthCompareOp = VK_COMPARE_OP_GREATER_OR_EQUAL;
			depthStencilDefault.depthBoundsTestEnable = VK_FALSE;
			depthStencilDefault.minDepthBounds = 0.0f;
			depthStencilDefault.maxDepthBounds = 1.0f;
			depthStencilDefault.stencilTestEnable = VK_FALSE;
			depthStencilDefault.front = {};
			depthStencilDefault.back = {};

			mDepthStencil = std::move(depthStencilDefault);
		}

		mDepthStencil->depthTestEnable = enableDepthTest ? VK_TRUE : VK_FALSE;
		mDepthStencil->depthWriteEnable = enableDepthWrite ? VK_TRUE : VK_FALSE;

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

			if (!mDSetLayouts.empty())
			{
				pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(mDSetLayouts.size());
				pipelineLayoutInfo.pSetLayouts = mDSetLayouts.data();
			}

			if (!mPushConstants.empty())
			{
				pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(mPushConstants.size());
				pipelineLayoutInfo.pPushConstantRanges = mPushConstants.data();
			}

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
		if (mDepthStencil)
			pipelineInfo.pDepthStencilState = &mDepthStencil.value();
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
		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = static_cast<uint32_t>(mAttachments.size());
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		mAttachmentColorRefs.push_back(std::move(colorAttachmentRef));

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

	Object<VkRenderPass>::Builder& Object<VkRenderPass>::Builder::addAttachmentDepth(VkFormat format)
	{
		//we can only set one depth attachment
		if (mAttachmentDepthRef)
			return *this;

		mAttachmentDepthRef = VkAttachmentReference{};
		mAttachmentDepthRef->attachment = static_cast<uint32_t>(mAttachments.size());
		mAttachmentDepthRef->layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentDescription depthAttachment{};
		depthAttachment.format = format;
		depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		mAttachments.push_back(std::move(depthAttachment));

		return *this;
	}

	Object<VkRenderPass>::Builder& Object<VkRenderPass>::Builder::addSubpass()
	{
		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = static_cast<uint32_t>(mAttachmentColorRefs.size());
		subpass.pColorAttachments = mAttachmentColorRefs.data();
		if (mAttachmentDepthRef)
			subpass.pDepthStencilAttachment = &mAttachmentDepthRef.value();

		mSubpasses.push_back(std::move(subpass));
		return *this;
	}

	Object<VkRenderPass>::Builder& Object<VkRenderPass>::Builder::addSubpassDependency(bool hasColor, bool hasDepth)
	{
		assert(hasColor || hasDepth);
		if (!hasColor && !hasDepth)
			return *this;

		VkPipelineStageFlags srcStageMask{VK_PIPELINE_STAGE_NONE};
		VkPipelineStageFlags dstStageMask{VK_PIPELINE_STAGE_NONE};
		VkAccessFlags dstAccessMask{VK_ACCESS_NONE};

		if (hasColor)
		{
			srcStageMask |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dstStageMask |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dstAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		}

		if (hasDepth)
		{
			srcStageMask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			dstStageMask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			dstAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		}

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = srcStageMask;
		dependency.srcAccessMask = VK_ACCESS_NONE;
		dependency.dstStageMask = dstStageMask;
		dependency.dstAccessMask = dstAccessMask;

		mSubpassDependencies.push_back(std::move(dependency));
		return *this;
	}

	Object<VkRenderPass> Object<VkRenderPass>::Builder::build() const noexcept
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

	bool Object<VkCommandBuffer>::Recorder::doRenderPass(VkRenderPass renderPass, VkFramebuffer framebuffer, uint32_t renderWidth, uint32_t renderHeight,
	  std::span<const float, 4> clearColor, float clearDepth, uint32_t clearStencil, const std::function<void(Recorder&)>& cb)
	{
		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderPass;
		renderPassInfo.framebuffer = framebuffer;

		renderPassInfo.renderArea.offset = {0, 0};
		renderPassInfo.renderArea.extent = {renderWidth, renderHeight};

		std::array<VkClearValue, 2> clearColorsInfo;
		clearColorsInfo[0] = VkClearValue{};
		std::memcpy(clearColorsInfo[0].color.float32, clearColor.data(), clearColor.size_bytes());
		clearColorsInfo[1] = VkClearValue{};
		clearColorsInfo[1].depthStencil = {clearDepth, clearStencil};
		renderPassInfo.clearValueCount = 2;
		renderPassInfo.pClearValues = clearColorsInfo.data();

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
		VkBufferCopy region{};
		region.srcOffset = static_cast<VkDeviceSize>(sourceOffset);
		region.dstOffset = static_cast<VkDeviceSize>(destOffset);
		region.size = static_cast<VkDeviceSize>(size);
		vkCmdCopyBuffer(mCmdBuffer.native(), source, dest, 1, &region);

		return *this;
	}

	Object<VkCommandBuffer>::Recorder& Object<VkCommandBuffer>::Recorder::copyBufferToImage(VkImage dest, VkBuffer source, size_t width, size_t height) noexcept
	{
		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;
		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;
		region.imageOffset = {0, 0, 0};
		region.imageExtent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1};
		vkCmdCopyBufferToImage(mCmdBuffer.native(), source, dest, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

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

	Object<VkCommandBuffer>::Recorder& Object<VkCommandBuffer>::Recorder::bindDescriptorSets(VkPipelineLayout pipelineLayout, VkDescriptorSet descriptorSet)
	{
		vkCmdBindDescriptorSets(mCmdBuffer.native(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);
		return *this;
	}

	Object<VkCommandBuffer>::Recorder& Object<VkCommandBuffer>::Recorder::pushConstants(VkPipelineLayout pipelineLayout, VkShaderStageFlags stageFlags, const void* data, size_t dataSize, size_t dataOffset) noexcept
	{
		vkCmdPushConstants(mCmdBuffer.native(), pipelineLayout, stageFlags, static_cast<uint32_t>(dataOffset), static_cast<uint32_t>(dataSize), data);
		return *this;
	}

	Object<VkCommandBuffer>::Recorder& Object<VkCommandBuffer>::Recorder::pipelineBarrier(VkPipelineStageFlags sourceStage, VkPipelineStageFlags destinationStage, const VkImageMemoryBarrier& barrier) noexcept
	{
		vkCmdPipelineBarrier(mCmdBuffer.native(), sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
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

	bool Object<VkDeviceMemory>::write(std::span<const std::byte> data, size_t offset) const noexcept
	{
		assert(!mMappedMem);
		if (mMappedMem.has_value())
			return false;

		assert((data.size() + offset) <= mSpace.total);
		if ((data.size() + offset) > mSpace.total)
			return false;

		void* dataPtr;
		if (vkMapMemory(mDevice, mObj, static_cast<VkDeviceSize>(offset), static_cast<VkDeviceSize>(data.size()), 0, &dataPtr) != VK_SUCCESS)
			return false;

		std::memcpy(dataPtr, data.data(), data.size());
		vkUnmapMemory(mDevice, mObj);

		return true;
	}

	void* Object<VkDeviceMemory>::memMap(size_t size, size_t offset) noexcept
	{
		if (mMappedMem.has_value())
			return nullptr;

		void* dataPtr;
		if (vkMapMemory(mDevice, mObj, static_cast<VkDeviceSize>(offset), static_cast<VkDeviceSize>(size), 0, &dataPtr) != VK_SUCCESS)
			return nullptr;

		mMappedMem = dataPtr;
		return dataPtr;
	}

	void Object<VkDeviceMemory>::memUnmap() noexcept
	{
		if (!mMappedMem.has_value())
			return;

		vkUnmapMemory(mDevice, mObj);
		mMappedMem = std::nullopt;
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

	Object<VkDescriptorSetLayout>::Builder& Object<VkDescriptorSetLayout>::Builder::addUbo(uint32_t binding, VkShaderStageFlags shaderStageFlags)
	{
		VkDescriptorSetLayoutBinding uboLayoutBinding{};
		uboLayoutBinding.binding = binding;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.stageFlags = shaderStageFlags;
		uboLayoutBinding.pImmutableSamplers = nullptr;

		mBindings.push_back(std::move(uboLayoutBinding));

		return *this;
	}

	Object<VkDescriptorSetLayout>::Builder& Object<VkDescriptorSetLayout>::Builder::addSampler(uint32_t binding, VkShaderStageFlags shaderStageFlags)
	{
		VkDescriptorSetLayoutBinding samplerLayoutBinding{};
		samplerLayoutBinding.binding = binding;
		samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerLayoutBinding.descriptorCount = 1;
		samplerLayoutBinding.stageFlags = shaderStageFlags;
		samplerLayoutBinding.pImmutableSamplers = nullptr;

		mBindings.push_back(std::move(samplerLayoutBinding));

		return *this;
	}

	Object<VkDescriptorSetLayout> Object<VkDescriptorSetLayout>::Builder::build() const noexcept
	{
		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(mBindings.size());
		layoutInfo.pBindings = mBindings.data();

		VkDescriptorSetLayout descriptorSetLayout;
		if (vkCreateDescriptorSetLayout(mDevice, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
			return {};

		return Object(mDevice, descriptorSetLayout);
	}

	Object<VkDescriptorSetLayout>::~Object<VkDescriptorSetLayout>()
	{
		if (!BaseObject::operator bool())
			return;

		assert(mDevice != VK_NULL_HANDLE);

		vkDestroyDescriptorSetLayout(mDevice, mObj, nullptr);
		mObj = nullptr;
	}

	Object<VkDescriptorSet>::~Object<VkDescriptorSet>()
	{
		if (mImmortal)
		{
			mObj = nullptr; //otherwise BaseObject will complain
			return;
		}

		if (!BaseObject::operator bool())
			return;

		assert(mDevice != VK_NULL_HANDLE);
		assert(mDescriptorPool != VK_NULL_HANDLE);

		vkFreeDescriptorSets(mDevice, mDescriptorPool, 1, &mObj);
		mObj = nullptr;
	}

	Object<VkDescriptorSet>& Object<VkDescriptorSet>::updateUniformBuffer(uint32_t bindingIndex, VkBuffer buffer)
	{
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = buffer;
		bufferInfo.offset = 0;
		bufferInfo.range = VK_WHOLE_SIZE;
		mDescriptorBufferInfo.push_back(std::move(bufferInfo));

		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = mObj;
		descriptorWrite.dstBinding = bindingIndex;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pBufferInfo = nullptr; //update on save()
		mDescriptorWrites.push_back(std::move(descriptorWrite));

		return *this;
	}

	Object<VkDescriptorSet>& Object<VkDescriptorSet>::updateUniformBuffer(uint32_t bindingIndex, VkBuffer buffer, size_t bufferOffset, size_t bufferSize)
	{
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = buffer;
		bufferInfo.offset = static_cast<VkDeviceSize>(bufferOffset);
		bufferInfo.range = static_cast<VkDeviceSize>(bufferSize);
		mDescriptorBufferInfo.push_back(std::move(bufferInfo));

		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = mObj;
		descriptorWrite.dstBinding = bindingIndex;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pBufferInfo = nullptr; //update on save()
		mDescriptorWrites.push_back(std::move(descriptorWrite));

		return *this;
	}

	Object<VkDescriptorSet>& Object<VkDescriptorSet>::updateImageViewSampler(uint32_t bindingIndex, VkImageView imageView, VkSampler sampler)
	{
		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = imageView;
		imageInfo.sampler = sampler;
		mDescriptorImageInfo.push_back(std::move(imageInfo));

		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = mObj;
		descriptorWrite.dstBinding = bindingIndex;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pImageInfo = nullptr; //update on save()
		mDescriptorWrites.push_back(std::move(descriptorWrite));

		return *this;
	}

	void Object<VkDescriptorSet>::save()
	{
		size_t bufferIndex{0}, imageIndex{0};
		for (auto& desc : mDescriptorWrites)
		{
			switch (desc.descriptorType)
			{
				case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
					desc.pBufferInfo = &mDescriptorBufferInfo[bufferIndex];
					++bufferIndex;
					break;
				case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
					desc.pImageInfo = &mDescriptorImageInfo[imageIndex];
					++imageIndex;
					break;
				default:
					assert(!"Unknown descriptorType");
					break;
			}
		}
		assert(bufferIndex == mDescriptorBufferInfo.size());
		assert(imageIndex == mDescriptorImageInfo.size());

		vkUpdateDescriptorSets(mDevice, static_cast<uint32_t>(mDescriptorWrites.size()), mDescriptorWrites.data(), 0, nullptr);

		mDescriptorWrites.clear();
		mDescriptorBufferInfo.clear();
		mDescriptorImageInfo.clear();
	}

	Object<VkDescriptorPool>::Builder& Object<VkDescriptorPool>::Builder::addDescriptor(VkDescriptorType descriptorType, size_t descriptorCount)
	{
		VkDescriptorPoolSize poolSize{};
		poolSize.type = descriptorType;
		poolSize.descriptorCount = static_cast<uint32_t>(descriptorCount);

		mDescriptors.push_back(std::move(poolSize));

		return *this;
	}

	Object<VkDescriptorPool> Object<VkDescriptorPool>::Builder::build(bool immortalDescriptorSets, size_t maxDescriptorSets) const noexcept
	{
		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = static_cast<uint32_t>(mDescriptors.size());
		poolInfo.pPoolSizes = mDescriptors.data();
		poolInfo.maxSets = static_cast<uint32_t>(maxDescriptorSets);
		if (!immortalDescriptorSets)
			poolInfo.flags |= VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		
		VkDescriptorPool descriptorPool;
		if (vkCreateDescriptorPool(mDevice, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
			return {};
		
		return Object(mDevice, descriptorPool, immortalDescriptorSets);
	}

	Object<VkDescriptorPool>::~Object<VkDescriptorPool>()
	{
		if (!BaseObject::operator bool())
			return;

		assert(mDevice != VK_NULL_HANDLE);

		vkDestroyDescriptorPool(mDevice, mObj, nullptr);
		mObj = nullptr;
	}

	Object<VkDescriptorSet> Object<VkDescriptorPool>::allocateDescriptorSet(VkDescriptorSetLayout dsetLayout)
	{
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = mObj;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &dsetLayout;

		VkDescriptorSet descriptorSet;
		if (vkAllocateDescriptorSets(mDevice, &allocInfo, &descriptorSet) != VK_SUCCESS)
			return {};

		return Object<VkDescriptorSet>(mDevice, mObj, descriptorSet, mImmortalDescriptorSets);
	}
}