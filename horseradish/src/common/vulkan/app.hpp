#pragma once

#include "objects.hpp"

#include <string>
#include <optional>
#include <string_view>
#include <memory>
#include <functional>

#include <vulkan/vulkan.h>

namespace hr::vulkan
{
	class App
	{
	public:
		enum class DebugMessageSeverity
		{
			Verbose,
			Info,
			Warning,
			Error
		};

		enum class DebugMessageContext
		{
			None = 0,
			General = (1<<0),
			Validation = (1 << 1),
			Performance = (1 << 2),
			DeviceAddressBinding = (1 << 3)
		};

		using SetupSurface = std::function<VkSurfaceKHR(VkInstance)>;
		using DebugMessageCallback = std::function<void(DebugMessageSeverity, DebugMessageContext, std::string_view, std::string_view)>;

		class SwapChainImage
		{
			friend class App;

			App& mApp;
			std::optional<uint32_t> mImageIndex;

		private:
			SwapChainImage(App& app, uint32_t imageIndex) noexcept
			  : mApp{app}
			  , mImageIndex{imageIndex}
			{
			}

		public:
			~SwapChainImage() noexcept = default;

			SwapChainImage(const SwapChainImage&) = delete;
			SwapChainImage& operator=(const SwapChainImage&) = delete;
			SwapChainImage(SwapChainImage&& o) noexcept = default;
			SwapChainImage& operator=(SwapChainImage&& o) noexcept = default;

			bool present() noexcept;
			bool present(VkSemaphore toWaitFor) noexcept;
		};

	private:
		bool mInitialized{false};

		DebugMessageCallback mDebugCb;

		VkInstance mInstance{};
		VkSurfaceKHR mSurface{};

		VkPhysicalDevice mPhysicalDevice{};
		VkDevice mDevice{};
		VkQueue mGraphicsQueue{};
		uint32_t mGraphicsQueueIndex{0};

		VkSwapchainKHR mSwapChain{};
		VkSurfaceFormatKHR mSwapChainSurfaceFormat{};
		std::vector<VkImage> mSwapChainImages;
		std::vector<ImageView> mSwapChainImageViews;
		std::vector<Framebuffer> mSwapChainFramebuffers;
		VkExtent2D mSwapChainExtent{};

		VkDebugUtilsMessengerEXT mDebugMessenger{};

		std::string mErrorMsg;

	private:
		bool initDebug(VkInstanceCreateInfo &createInfo);
		bool setupDebug();
		bool cleanupDebug();

		bool initInstance();
		bool pickPhysicalDevice();
		bool initLogicalDevice();
		bool initSwapChain();

	public:
		App(DebugMessageCallback debugCb) noexcept
		  : mDebugCb{std::move(debugCb)}
		{
		}

		~App() noexcept;

		bool init(SetupSurface setupSurfaceCb);

		bool swapChainInit(VkRenderPass renderPass);

		CommandPool createCommandPool();

		void invokeDebugCb(App::DebugMessageSeverity severity, App::App::DebugMessageContext context, std::string_view, std::string_view msg) const noexcept;

		std::string_view lastError() const noexcept
		{
			return mErrorMsg;
		}

		VkDevice device() const noexcept
		{
			return mDevice;
		}

		VkQueue graphicsQueue() const noexcept
		{
			return mGraphicsQueue;
		}

		VkFormat swapChainImageFormat() const noexcept
		{
			return mSwapChainSurfaceFormat.format;
		}

		SwapChainImage swapChainAcquireImage(VkSemaphore whenImageReady) noexcept;
		VkFramebuffer swapChainFramebuffer(const SwapChainImage& swapChainImage) const noexcept;

		bool graphicsQueueSubmit(VkSemaphore waitFor, VkPipelineStageFlags waitForState, VkCommandBuffer commandBuffer, VkSemaphore doneCommandBuffer, VkFence doneQueue) const noexcept;

		void waitDeviceIdle() const noexcept;
	};

	inline App::DebugMessageContext operator|(App::DebugMessageContext a, App::DebugMessageContext b)
	{
		return static_cast<App::DebugMessageContext>(static_cast<std::underlying_type_t<App::DebugMessageContext>>(a) | static_cast<std::underlying_type_t<App::DebugMessageContext>>(b));
	}

	inline App::DebugMessageContext operator&(App::DebugMessageContext a, App::DebugMessageContext b)
	{
		return static_cast<App::DebugMessageContext>(static_cast<std::underlying_type_t<App::DebugMessageContext>>(a) & static_cast<std::underlying_type_t<App::DebugMessageContext>>(b));
	}

	inline App::DebugMessageContext& operator|=(App::DebugMessageContext& a, App::DebugMessageContext b)
	{
		a = a | b;
		return a;
	}
}
