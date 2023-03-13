#pragma once

#include <vulkan/vulkan.h>

#include <span>
#include <array>
#include <cassert>
#include <optional>
#include <functional>
#include <filesystem>

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
		};
	}

	template<class T>
	class Object;

	using ImageView = Object<VkImageView>;
	using ShaderModule = Object<VkShaderModule>;
	using Framebuffer = Object<VkFramebuffer>;
	using Pipeline = Object<VkPipeline>;
	using RenderPass = Object<VkRenderPass>;
	using CommandPool = Object<VkCommandPool>;
	using CommandBuffer = Object<VkCommandBuffer>;
	using Semaphore = Object<VkSemaphore>;
	using Fence = Object<VkFence>;

	template<>
	class Object<VkImageView> final : public detail::BaseObject<VkImageView>
	{
		VkDevice mDevice{VK_NULL_HANDLE};

	private:
		Object() noexcept = default;

		explicit Object(VkDevice device, VkImageView imageView) noexcept
		  : BaseObject{imageView}
		  , mDevice{device}
		{
		}

	public:
		static Object gen2D(VkDevice device, VkImage sourceImg, VkFormat sourceFormat) noexcept;

	public:
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

			VkPipelineVertexInputStateCreateInfo mVertexInputInfo;
			VkPipelineInputAssemblyStateCreateInfo mInputAssembly;

			VkPipelineRasterizationStateCreateInfo mRasterizer;
			VkPipelineMultisampleStateCreateInfo mMultisampling;

			VkPipelineColorBlendAttachmentState mColorBlendAttachment;

		public:
			Builder(VkDevice device) noexcept;

			Builder(const Builder&) = delete;
			Builder& operator=(const Builder&) = delete;
			Builder(Builder&&) noexcept = delete;
			Builder& operator=(Builder&&) noexcept = delete;

			Builder& setupShader(ShaderModule module, ShaderModule::ShaderType shaderType);
			Builder& setupVertexInput();
			Builder& setupInputAssembly(VkPrimitiveTopology topology, bool primitiveRestart);
			Builder& setupViewport(float x, float y, float width, float height, float minDepth, float maxDepth);
			Builder& setupScissor(int32_t x, int32_t y, uint32_t width, uint32_t height);

			Object build(VkRenderPass renderPass);
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
			std::vector<VkSubpassDescription> mSubpasses;
			std::vector<VkSubpassDependency> mSubpassDependencies;

		public:
			Builder(VkDevice device) noexcept;

			Builder(const Builder&) = delete;
			Builder& operator=(const Builder&) = delete;
			Builder(Builder&&) noexcept = delete;
			Builder& operator=(Builder&&) noexcept = delete;

			Builder& addAttachment(VkFormat format);
			Builder& addSubpass();
			Builder& addSubpassDependency();

			Object build();
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
		CommandBuffer allocateBuffer() noexcept;
	};

	template<>
	class Object<VkCommandBuffer> final : public detail::BaseObject<VkCommandBuffer>
	{
		VkDevice mDevice{VK_NULL_HANDLE};

	private:
		Object() noexcept = default;

		explicit Object(VkDevice device, VkCommandBuffer commandBuffer) noexcept
		  : BaseObject{commandBuffer}
		  , mDevice{device}
		{
		}

	public:
		static Object gen(VkDevice device, VkCommandPool commandPool) noexcept;

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

			bool doRenderPass(VkRenderPass renderPass, VkFramebuffer framebuffer, uint32_t renderWidth, uint32_t renderHeight, const std::function<void(Recorder&)>& cb);

			Recorder& bindPipeline(VkPipeline pipeline) noexcept;
			Recorder& setViewport(float x, float y, float width, float height, float minDepth, float maxDepth) noexcept;
			Recorder& setScissor(int32_t x, int32_t y, uint32_t width, uint32_t height) noexcept;

			Recorder& draw(uint32_t vertexCount) noexcept;

			bool finish() noexcept;
		};

	public:
		~Object() noexcept
		{
			//commands are destroyed through the command pool
			mObj = VK_NULL_HANDLE;
		}

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
}
