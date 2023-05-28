#pragma once

#include <vulkan/vulkan.h>

#include <span>
#include <array>
#include <cassert>
#include <optional>
#include <functional>
#include <filesystem>
#include <initializer_list>

namespace hr::vulkan
{
	namespace detail
	{
		template<class T>
		struct BaseObject
		{
		protected:
			T mObj{VK_NULL_HANDLE};

		protected:
			explicit BaseObject(T obj) noexcept
			  : mObj{obj}
			{
			}

			~BaseObject() noexcept
			{
				assert(mObj == VK_NULL_HANDLE);
			}

		public:
			BaseObject() noexcept = default;
			BaseObject(const BaseObject&) = delete;
			BaseObject& operator=(const BaseObject&) = delete;
			BaseObject(BaseObject&& o) noexcept
			{
				mObj = std::exchange(o.mObj, VK_NULL_HANDLE);
			}
			BaseObject& operator=(BaseObject&& o) noexcept
			{
				std::swap(mObj, o.mObj);
				return *this;
			}

			explicit operator bool() const noexcept
			{
				return (mObj != VK_NULL_HANDLE);
			}

			bool isValid() const noexcept
			{
				return operator bool();
			}

			T native() const noexcept
			{
				return mObj;
			}

			const T* nativePtr() const noexcept
			{
				return &mObj;
			}
		};
	}

	template<class T>
	class Object;

	using ImageView = Object<VkImageView>;
	using Image = Object<VkImage>;
	using Sampler = Object<VkSampler>;
	using ShaderModule = Object<VkShaderModule>;
	using Framebuffer = Object<VkFramebuffer>;
	using Pipeline = Object<VkPipeline>;
	using RenderPass = Object<VkRenderPass>;
	using CommandPool = Object<VkCommandPool>;
	using CommandBuffer = Object<VkCommandBuffer>;
	using Semaphore = Object<VkSemaphore>;
	using Fence = Object<VkFence>;
	using Memory = Object<VkDeviceMemory>;
	using Buffer = Object<VkBuffer>;
	using DescriptorSetLayout = Object<VkDescriptorSetLayout>;
	using DescriptorSet = Object<VkDescriptorSet>;
	using DescriptorPool = Object<VkDescriptorPool>;

	template<>
	class Object<VkImageView> final : public detail::BaseObject<VkImageView>
	{
		VkDevice mDevice{VK_NULL_HANDLE};

	private:
		explicit Object(VkDevice device, VkImageView imageView) noexcept
		  : BaseObject{imageView}
		  , mDevice{device}
		{
		}

	public:
		static Object gen2D(VkDevice device, VkImage sourceImg, size_t numMipLevels, VkFormat sourceFormat) noexcept;
		static Object genDepth(VkDevice device, VkImage sourceImg, VkFormat sourceFormat) noexcept;

	public:
		Object() noexcept = default;
		~Object() noexcept;

		Object(const Object&) = delete;
		Object& operator=(const Object&) = delete;
		Object(Object&&) noexcept = default;
		Object& operator=(Object&&) noexcept = default;
	};

	template<>
	class Object<VkImage> final : public detail::BaseObject<VkImage>
	{
		VkDevice mDevice{VK_NULL_HANDLE};
		VkMemoryRequirements mMemRequirements{};

	private:
		explicit Object(VkDevice device, VkImage image, VkMemoryRequirements memRequirements) noexcept
		  : BaseObject{image}
		  , mDevice{device}
		  , mMemRequirements{memRequirements}
		{
		}

	public:
		static size_t calculateNumMipMaps(size_t width)
		{
			return static_cast<size_t>(std::floor(std::log2(width))) + 1;
		}

		static size_t calculateNumMipMaps(size_t width, size_t height)
		{
			return calculateNumMipMaps(std::max(width, height));
		}

		static size_t calculateNumMipMaps(size_t width, size_t height, size_t depth)
		{
			return calculateNumMipMaps(std::max(std::max(width, height), depth));
		}

		static Object gen2D(VkDevice device, size_t width, size_t height, bool withMipMaps, VkFormat format) noexcept;
		static Object genDepth(VkDevice device, size_t width, size_t height, VkFormat format) noexcept;

	public:
		Object() noexcept = default;
		~Object() noexcept;

		Object(const Object&) = delete;
		Object& operator=(const Object&) = delete;
		Object(Object&&) noexcept = default;
		Object& operator=(Object&&) noexcept = default;

		size_t hostRequiredSize() const noexcept
		{
			return static_cast<size_t>(mMemRequirements.size);
		}

		uint32_t hostRequiredMemoryType() const noexcept
		{
			return mMemRequirements.memoryTypeBits;
		}

		bool allocate(Object<VkDeviceMemory>& memory) const noexcept;
	};

	template<>
	class Object<VkSampler> final : public detail::BaseObject<VkSampler>
	{
		VkDevice mDevice{VK_NULL_HANDLE};

	private:
		static VkSamplerCreateInfo defaultSampler() noexcept;

	private:
		explicit Object(VkDevice device, VkSampler sampler) noexcept
		  : BaseObject{sampler}
		  , mDevice{device}
		{
		}

	public:
		static Object create(VkDevice device, VkFilter minFilter, VkFilter magFilter) noexcept;
		static Object createAnisotropic(VkDevice device, VkFilter minFilter, VkFilter magFilter, float maxAnisotropy) noexcept;

		static Object createMipMaps(VkDevice device, VkFilter minFilter, VkFilter magFilter, VkSamplerMipmapMode mipmapMode) noexcept;
		static Object createMipMapsAnisotropic(VkDevice device, VkFilter minFilter, VkFilter magFilter, VkSamplerMipmapMode mipmapMode, float maxAnisotropy) noexcept;

	public:
		Object() noexcept = default;
		~Object() noexcept;

		Object(const Object&) = delete;
		Object& operator=(const Object&) = delete;
		Object(Object&&) noexcept = default;
		Object& operator=(Object&&) noexcept = default;
	};

	template<>
	class Object<VkShaderModule> final : public detail::BaseObject<VkShaderModule>
	{
		VkDevice mDevice{VK_NULL_HANDLE};

	private:
		Object() noexcept = default;

		explicit Object(VkDevice device, VkShaderModule shaderModule) noexcept
		  : BaseObject{shaderModule}
		  , mDevice{device}
		{
		}

	public:
		enum class ShaderType
		{
			Vertex,
			Fragment,
			Compute
		};

		static bool compileShader(const std::filesystem::path& path,
		  const std::function<bool(const std::filesystem::path&, const std::filesystem::path&, ShaderType)>& compilerCb);

		static Object loadShader(VkDevice device, const std::filesystem::path& path);

	public:
		~Object() noexcept;

		Object(const Object&) = delete;
		Object& operator=(const Object&) = delete;
		Object(Object&&) noexcept = default;
		Object& operator=(Object&&) noexcept = default;
	};

	template<>
	class Object<VkFramebuffer> final : public detail::BaseObject<VkFramebuffer>
	{
		VkDevice mDevice{VK_NULL_HANDLE};

	private:
		Object() noexcept = default;

		explicit Object(VkDevice device, VkFramebuffer framebuffer) noexcept
		  : BaseObject{framebuffer}
		  , mDevice{device}
		{
		}

	public:
		static Object gen(VkDevice device, VkRenderPass renderPass, std::initializer_list<VkImageView> imageViews, uint32_t imageViewsWidth, uint32_t imageViewsHeight);
		static Object gen(VkDevice device, VkRenderPass renderPass, std::span<VkImageView> imageViews, uint32_t imageViewsWidth, uint32_t imageViewsHeight) noexcept;

	public:
		~Object() noexcept;

		Object(const Object&) = delete;
		Object& operator=(const Object&) = delete;
		Object(Object&&) noexcept = default;
		Object& operator=(Object&&) noexcept = default;
	};

	template<>
	class Object<VkPipeline> final : public detail::BaseObject<VkPipeline>
	{
		VkDevice mDevice{VK_NULL_HANDLE};
		VkPipelineLayout mPipelineLayout{VK_NULL_HANDLE};

	private:
		explicit Object(VkDevice device, VkPipeline pipeline, VkPipelineLayout pipelineLayout) noexcept
		  : BaseObject{pipeline}
		  , mDevice{device}
		  , mPipelineLayout{pipelineLayout}
		{
		}

	public:
		class Builder
		{
			VkDevice mDevice{VK_NULL_HANDLE};
			
			struct
			{
				std::optional<ShaderModule> vertexShaderModule;
				std::optional<ShaderModule> fragmentShaderModule;
			} mStageShaders;
						
			struct
			{
				std::optional<VkViewport> viewport;
				std::optional<VkRect2D> scissor;
			} mStageViewport;
			VkPipelineViewportStateCreateInfo mViewportState;

			struct
			{
				size_t numBindings{0};
				std::array<VkVertexInputBindingDescription, 8> bindings{};
				size_t numAttribs{0};
				std::array<VkVertexInputAttributeDescription, 8> attribs{};
			} mStageVertexInput;
			VkPipelineVertexInputStateCreateInfo mVertexInputInfo;
			VkPipelineInputAssemblyStateCreateInfo mInputAssembly;

			std::vector<VkDescriptorSetLayout> mDSetLayouts;
			std::vector<VkPushConstantRange> mPushConstants;

			VkPipelineRasterizationStateCreateInfo mRasterizer;
			VkPipelineMultisampleStateCreateInfo mMultisampling;

			VkPipelineColorBlendAttachmentState mColorBlendAttachment;

			std::optional<VkPipelineDepthStencilStateCreateInfo> mDepthStencil;

		public:
			Builder(VkDevice device) noexcept;

			Builder(const Builder&) = delete;
			Builder& operator=(const Builder&) = delete;
			Builder(Builder&&) noexcept = delete;
			Builder& operator=(Builder&&) noexcept = delete;

			Builder& setupShader(ShaderModule module, ShaderModule::ShaderType shaderType);
			Builder& addDescriptorSetLayout(VkDescriptorSetLayout dsetLayout);
			Builder& addPushConstant(size_t offset, size_t size, VkShaderStageFlags stageFlags);

			Builder& addVertexBinding(size_t bindingIndex, size_t stride);
			Builder& addVertexAttribute(size_t bindingIndex, size_t locationIndex, size_t offset, VkFormat format);
			Builder& setupInputAssembly(VkPrimitiveTopology topology, bool primitiveRestart);

			Builder& setupViewport(float width, float height, bool flipY);
			Builder& setupViewport(float x, float y, float width, float height);
			Builder& setupViewport(float x, float y, float width, float height, float minDepth, float maxDepth);
			Builder& setupScissor(int32_t x, int32_t y, uint32_t width, uint32_t height);

			Builder& setupDepth(bool enableDepthTest, bool enableDepthWrite);

			Object build(VkRenderPass renderPass);
		};

	public:
		Object() noexcept = default;
		~Object() noexcept;

		Object(const Object&) = delete;
		Object& operator=(const Object&) = delete;
		Object(Object&&) noexcept = default;
		Object& operator=(Object&&) noexcept = default;

		VkPipelineLayout layout() const noexcept
		{
			return mPipelineLayout;
		}
	};

	template<>
	class Object<VkRenderPass> final : public detail::BaseObject<VkRenderPass>
	{
		VkDevice mDevice{VK_NULL_HANDLE};

	private:
		explicit Object(VkDevice device, VkRenderPass renderPass) noexcept
		  : BaseObject{renderPass}
		  , mDevice{device}
		{
		}

	public:
		class Builder
		{
			VkDevice mDevice{VK_NULL_HANDLE};
			std::vector<VkAttachmentDescription> mAttachments;
			std::vector<VkAttachmentReference> mAttachmentColorRefs;
			std::optional<VkAttachmentReference> mAttachmentDepthRef;

			std::vector<VkSubpassDescription> mSubpasses;
			std::vector<VkSubpassDependency> mSubpassDependencies;

		public:
			Builder(VkDevice device) noexcept;

			Builder(const Builder&) = delete;
			Builder& operator=(const Builder&) = delete;
			Builder(Builder&&) noexcept = delete;
			Builder& operator=(Builder&&) noexcept = delete;

			Builder& addAttachment(VkFormat format);
			Builder& addAttachmentDepth(VkFormat format);
			Builder& addSubpass();
			Builder& addSubpassDependency(bool hasColor, bool hasDepth);

			Object build() const noexcept;
		};

	public:
		Object() noexcept = default;
		~Object() noexcept;

		Object(const Object&) = delete;
		Object& operator=(const Object&) = delete;
		Object(Object&&) noexcept = default;
		Object& operator=(Object&&) noexcept = default;
	};

	template<>
	class Object<VkCommandPool> final : public detail::BaseObject<VkCommandPool>
	{
		VkDevice mDevice{VK_NULL_HANDLE};

	private:
		Object() noexcept = default;

		explicit Object(VkDevice device, VkCommandPool commandPool) noexcept
		  : BaseObject{commandPool}
		  , mDevice{device}
		{
		}

	public:
		static Object gen(VkDevice device, uint32_t queueFamilyIndex) noexcept;

	public:
		~Object() noexcept;

		Object(const Object&) = delete;
		Object& operator=(const Object&) = delete;
		Object(Object&&) noexcept = default;
		Object& operator=(Object&&) noexcept = default;

	public:
		CommandBuffer allocateBuffer(bool destroyWithPool) noexcept;
	};

	template<>
	class Object<VkCommandBuffer> final : public detail::BaseObject<VkCommandBuffer>
	{
		VkDevice mDevice{VK_NULL_HANDLE};
		VkCommandPool mCommandPool{VK_NULL_HANDLE};
		bool mDestroyWithPool{true};

	private:
		Object() noexcept = default;

		explicit Object(VkDevice device, VkCommandPool commandPool, VkCommandBuffer commandBuffer, bool destroyWithPool) noexcept
		  : BaseObject{commandBuffer}
		  , mDevice{device}
		  , mCommandPool{commandPool}
		  , mDestroyWithPool{destroyWithPool}
		{
		}

	public:
		static Object gen(VkDevice device, VkCommandPool commandPool, bool destroyWithPool) noexcept;

		class Recorder
		{
			friend class Object;

			Object& mCmdBuffer;
			bool mFinished{false};

		private:
			Recorder(Object& cmdBuffer) noexcept
			  : mCmdBuffer{cmdBuffer}
			{
			}

		public:
			~Recorder() noexcept = default;
			Recorder(const Recorder&) = delete;
			Recorder& operator=(const Recorder&) = delete;
			Recorder(Recorder&&) noexcept = default;
			Recorder& operator=(Recorder&&) noexcept = default;

			bool doRenderPass(VkRenderPass renderPass, VkFramebuffer framebuffer, uint32_t renderWidth, uint32_t renderHeight, std::span<const float, 4> clearColor, float clearDepth,
			  uint32_t clearStencil, const std::function<void(Recorder&)>& cb);

			Recorder& copyBuffer(VkBuffer dest, VkBuffer source, size_t size) noexcept;
			Recorder& copyBuffer(VkBuffer dest, size_t destOffset, VkBuffer source, size_t sourceOffset, size_t size) noexcept;

			Recorder& copyBufferToImage(VkImage dest, VkBuffer source, size_t width, size_t height, size_t mipLevel) noexcept;
			Recorder& copyBufferToImage(VkImage dest, VkBuffer source, size_t width, size_t height) noexcept
			{
				return copyBufferToImage(dest, source, width, height, 0);
			}

			Recorder& bindVertexBuffer(uint32_t bindingIndex, VkBuffer buffer, uint32_t bufferOffset) noexcept;
			Recorder& bindIndexBuffer(VkBuffer buffer, uint32_t bufferOffset, VkIndexType indexType) noexcept;

			Recorder& bindPipeline(VkPipeline pipeline) noexcept;
			Recorder& setViewport(float x, float y, float width, float height, float minDepth, float maxDepth) noexcept;
			Recorder& setScissor(int32_t x, int32_t y, uint32_t width, uint32_t height) noexcept;

			Recorder& bindDescriptorSets(VkPipelineLayout pipelineLayout, VkDescriptorSet descriptorSet);
			Recorder& pushConstants(VkPipelineLayout pipelineLayout, VkShaderStageFlags stageFlags, const void* data, size_t dataSize, size_t dataOffset) noexcept;

			Recorder& pipelineBarrier(VkPipelineStageFlags sourceStage, VkPipelineStageFlags destinationStage, const VkImageMemoryBarrier& barrier) noexcept;

			Recorder& draw(uint32_t numIndices) noexcept;

			bool finish() noexcept;
		};

	public:
		~Object() noexcept;

		Object(const Object&) = delete;
		Object& operator=(const Object&) = delete;
		Object(Object&&) noexcept = default;
		Object& operator=(Object&&) noexcept = default;

	public:
		void reset() noexcept;
		std::optional<Recorder> record(bool oneTimeOnly) noexcept;
	};

	template<>
	class Object<VkSemaphore> final : public detail::BaseObject<VkSemaphore>
	{
		VkDevice mDevice{VK_NULL_HANDLE};

	private:
		Object() noexcept = default;

		explicit Object(VkDevice device, VkSemaphore semaphore) noexcept
		  : BaseObject{semaphore}
		  , mDevice{device}
		{
		}

	public:
		static Object gen(VkDevice device) noexcept;

	public:
		~Object() noexcept;

		Object(const Object&) = delete;
		Object& operator=(const Object&) = delete;
		Object(Object&&) noexcept = default;
		Object& operator=(Object&&) noexcept = default;
	};

	template<>
	class Object<VkFence> final : public detail::BaseObject<VkFence>
	{
		VkDevice mDevice{VK_NULL_HANDLE};

	private:
		Object() noexcept = default;

		explicit Object(VkDevice device, VkFence fence) noexcept
		  : BaseObject{fence}
		  , mDevice{device}
		{
		}

	public:
		static Object gen(VkDevice device, bool signaled = false) noexcept;

		static void waitAll(VkDevice device, std::initializer_list<Object> fences) noexcept;

	public:
		~Object() noexcept;

		Object(const Object&) = delete;
		Object& operator=(const Object&) = delete;
		Object(Object&&) noexcept = default;
		Object& operator=(Object&&) noexcept = default;

	public:
		Object& wait() noexcept;
		Object& reset() noexcept;
	};

	template<>
	class Object<VkDeviceMemory> final : public detail::BaseObject<VkDeviceMemory>
	{
		VkDevice mDevice{VK_NULL_HANDLE};
		struct
		{
			size_t total{0};
			size_t used{0};
		} mSpace;
		std::optional<void*> mMappedMem;
		

	private:
		explicit Object(VkDevice device, VkDeviceMemory memory, size_t size) noexcept
		  : BaseObject{memory}
		  , mDevice{device}
		{
			mSpace.total = size;
		}

	public:
		static Object gen(VkDevice device, VkDeviceMemory memory, size_t size) noexcept;

	public:
		Object() noexcept = default;
		~Object() noexcept;

		Object(const Object&) = delete;
		Object& operator=(const Object&) = delete;
		Object(Object&&) noexcept = default;
		Object& operator=(Object&&) noexcept = default;

		size_t sizeUsed() const noexcept
		{
			return mSpace.used;
		}

		std::optional<size_t> reserve(size_t size) noexcept;
		std::optional<size_t> reserve(size_t size, size_t alignment) noexcept;

		bool write(std::span<const std::byte> data, size_t offset) const noexcept;

		void* memMap(size_t size, size_t offset) noexcept;
		void memUnmap() noexcept;

		void* memMappedPtr() const noexcept
		{
			return mMappedMem.value_or(nullptr);
		}
	};

	template<>
	class Object<VkBuffer> final : public detail::BaseObject<VkBuffer>
	{
		VkDevice mDevice{VK_NULL_HANDLE};
		size_t mSize{0};
		VkMemoryRequirements mMemRequirements{};

	private:
		explicit Object(VkDevice device, VkBuffer buffer, size_t size, VkMemoryRequirements memRequirements) noexcept
		  : BaseObject{buffer}
		  , mDevice{device}
		  , mSize{size}
		  , mMemRequirements{memRequirements}
		{
		}

	public:
		static Object gen(VkDevice device, size_t size, VkBufferUsageFlags usage) noexcept;

	public:
		Object() noexcept = default;
		~Object() noexcept;

		Object(const Object&) = delete;
		Object& operator=(const Object&) = delete;
		Object(Object&&) noexcept = default;
		Object& operator=(Object&&) noexcept = default;

		size_t size() const noexcept
		{
			return mSize;
		}

		size_t hostRequiredSize() const noexcept
		{
			return static_cast<size_t>(mMemRequirements.size);
		}

		uint32_t hostRequiredMemoryType() const noexcept
		{
			return mMemRequirements.memoryTypeBits;
		}

		bool allocate(Object<VkDeviceMemory>& memory) const noexcept;
	};

	template<>
	class Object<VkDescriptorSetLayout> final : public detail::BaseObject<VkDescriptorSetLayout>
	{
		VkDevice mDevice{VK_NULL_HANDLE};

	private:
		explicit Object(VkDevice device, VkDescriptorSetLayout dsetLayout) noexcept
		  : BaseObject{dsetLayout}
		  , mDevice{device}
		{
		}

	public:
		class Builder
		{
			VkDevice mDevice{VK_NULL_HANDLE};

			std::vector<VkDescriptorSetLayoutBinding> mBindings;

		public:
			Builder(VkDevice device) noexcept
			  : mDevice{device}
			{
			}

			Builder(const Builder&) = delete;
			Builder& operator=(const Builder&) = delete;
			Builder(Builder&&) noexcept = delete;
			Builder& operator=(Builder&&) noexcept = delete;

			Builder& addUbo(uint32_t binding, VkShaderStageFlags shaderStageFlags);
			Builder& addSampler(uint32_t binding, VkShaderStageFlags shaderStageFlags);

			Object build() const noexcept;
		};

	public:
		Object() noexcept = default;
		~Object() noexcept;

		Object(const Object&) = delete;
		Object& operator=(const Object&) = delete;
		Object(Object&&) noexcept = default;
		Object& operator=(Object&&) noexcept = default;
	};

	template<>
	class Object<VkDescriptorSet> final : public detail::BaseObject<VkDescriptorSet>
	{
		friend class Object<VkDescriptorPool>;

		VkDevice mDevice{VK_NULL_HANDLE};
		VkDescriptorPool mDescriptorPool{VK_NULL_HANDLE};
		bool mImmortal{false};

		std::vector<VkWriteDescriptorSet> mDescriptorWrites;
		std::vector<VkDescriptorBufferInfo> mDescriptorBufferInfo;
		std::vector<VkDescriptorImageInfo> mDescriptorImageInfo;

	private:
		explicit Object(VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorSet descriptorSet, bool immortal) noexcept
		  : BaseObject{descriptorSet}
		  , mDevice{device}
		  , mDescriptorPool{descriptorPool}
		  , mImmortal{immortal}
		{
		}

	public:
		Object() noexcept = default;
		~Object() noexcept;

		Object(const Object&) = delete;
		Object& operator=(const Object&) = delete;
		Object(Object&&) noexcept = default;
		Object& operator=(Object&&) noexcept = default;

	public:
		Object& updateUniformBuffer(uint32_t bindingIndex, VkBuffer buffer);
		Object& updateUniformBuffer(uint32_t bindingIndex, VkBuffer buffer, size_t bufferOffset, size_t bufferSize);
		Object& updateImageViewSampler(uint32_t bindingIndex, VkImageView imageView, VkSampler sampler);
		void save();
	};

	template<>
	class Object<VkDescriptorPool> final : public detail::BaseObject<VkDescriptorPool>
	{
		VkDevice mDevice{VK_NULL_HANDLE};
		bool mImmortalDescriptorSets{false};

	private:
		Object() noexcept = default;

		explicit Object(VkDevice device, VkDescriptorPool descriptorPool, bool immortalDescriptorSets) noexcept
		  : BaseObject{descriptorPool}
		  , mDevice{device}
		  , mImmortalDescriptorSets{immortalDescriptorSets}
		{
		}

	public:
		class Builder
		{
			VkDevice mDevice{VK_NULL_HANDLE};

			std::vector<VkDescriptorPoolSize> mDescriptors;

		public:
			Builder(VkDevice device) noexcept
			  : mDevice{device}
			{
			}

			Builder(const Builder&) = delete;
			Builder& operator=(const Builder&) = delete;
			Builder(Builder&&) noexcept = delete;
			Builder& operator=(Builder&&) noexcept = delete;

			Builder& addDescriptor(VkDescriptorType descriptorType, size_t descriptorCount);

			Object build(bool immortalDescriptorSets, size_t maxDescriptorSets) const noexcept;
		};

	public:
		~Object() noexcept;

		Object(const Object&) = delete;
		Object& operator=(const Object&) = delete;
		Object(Object&&) noexcept = default;
		Object& operator=(Object&&) noexcept = default;

	public:
		Object<VkDescriptorSet> allocateDescriptorSet(VkDescriptorSetLayout dsetLayout);
	};
}
