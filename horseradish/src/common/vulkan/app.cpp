#include "app.hpp"

#include <bit>
#include <set>
#include <array>
#include <vector>
#include <format>
#include <cassert>
#include <numeric>
#include <algorithm>

namespace hr::vulkan
{
	namespace
	{
		constexpr std::array<const char*, 3> requiredInstanceExts{"VK_KHR_surface", "VK_KHR_win32_surface", "VK_EXT_debug_utils"};
		constexpr std::array<const char*, 1> requiredDeviceExts{"VK_KHR_swapchain"};

		constexpr std::array<const char*, 1> requiredLayers{"VK_LAYER_KHRONOS_validation"};

		VKAPI_ATTR VkBool32 VKAPI_CALL vulkanDebugCb(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
		{
			assert(pUserData);
			auto app = static_cast<App*>(pUserData);

			App::DebugMessageSeverity severity;
			switch (messageSeverity)
			{
				case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
					severity = App::DebugMessageSeverity::Verbose;
					break;
				case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
					severity = App::DebugMessageSeverity::Info;
					break;
				case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
					severity = App::DebugMessageSeverity::Warning;
					break;
				case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
					severity = App::DebugMessageSeverity::Error;
					break;
				default:
					return VK_FALSE;
			}

			App::DebugMessageContext context{0};
			if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT)
				context |= App::DebugMessageContext::General;
			if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
				context |= App::DebugMessageContext::Validation;
			if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT)
				context |= App::DebugMessageContext::Performance;
			if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT)
				context |= App::DebugMessageContext::DeviceAddressBinding;

			app->invokeDebugCb(severity, context, pCallbackData->pMessageIdName, pCallbackData->pMessage);

			return VK_FALSE;
		}
	}

	bool App::initDebug(VkInstanceCreateInfo& createInfo)
	{
		createInfo.enabledLayerCount = 0;
		createInfo.ppEnabledLayerNames = nullptr;

		std::vector<VkLayerProperties> layers;
		{
			uint32_t layerCount;
			vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

			layers.resize(layerCount);
			vkEnumerateInstanceLayerProperties(&layerCount, layers.data());
		}

		for (auto requiredLayer : requiredLayers)
		{
			std::string_view requiredLayerView{requiredLayer};

			if (std::any_of(layers.begin(), layers.end(), [&requiredLayerView](const auto& ext) { return std::string_view{ext.layerName} == requiredLayerView; }))
				continue;

			mErrorMsg = std::format("Required layer '{}' isn't supported.", requiredLayer);
			return false;
		}

		createInfo.enabledLayerCount = static_cast<uint32_t>(requiredLayers.size());
		createInfo.ppEnabledLayerNames = requiredLayers.data();

		return true;
	}

	bool App::setupDebug()
	{
		if (!mDebugCb)
			return true;

		VkDebugUtilsMessengerCreateInfoEXT createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_FLAG_BITS_MAX_ENUM_EXT;
		createInfo.pfnUserCallback = vulkanDebugCb;
		createInfo.pUserData = this;

		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(mInstance, "vkCreateDebugUtilsMessengerEXT");
		if (!func)
		{
			mErrorMsg = "Unable to load function 'vkCreateDebugUtilsMessengerEXT' to set up the debug messenger.";
			return false;
		}

		if (func(mInstance, &createInfo, nullptr, &mDebugMessenger) != VK_SUCCESS)
		{
			mErrorMsg = "Failed to set up debug messenger.";
			return false;
		}

		return true;
	}

	bool App::cleanupDebug()
	{
		if (!mDebugMessenger)
			return true;

		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(mInstance, "vkDestroyDebugUtilsMessengerEXT");
		if (!func)
		{
			mErrorMsg = "Unable to load function 'vkDestroyDebugUtilsMessengerEXT' to clean up the debug messenger.";
			return false;
		}

		func(mInstance, mDebugMessenger, nullptr);

		return true;
	}

	bool App::initInstance()
	{
		//make sure that the required instance extensions are present
		{
			uint32_t extCount = 0;
			vkEnumerateInstanceExtensionProperties(nullptr, &extCount, nullptr);

			std::vector<VkExtensionProperties> exts(extCount);
			vkEnumerateInstanceExtensionProperties(nullptr, &extCount, exts.data());

			for (auto requiredExt : requiredInstanceExts)
			{
				std::string_view requiredExtView{requiredExt};

				if (std::any_of(exts.begin(), exts.end(), [&requiredExtView](const auto& ext) { return std::string_view{ext.extensionName} == requiredExtView; }))
					continue;

				mErrorMsg = std::format("Required extension '{}' isn't supported.", requiredExtView);
				return false;
			}
		}

		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Horseradish";
		appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
		appInfo.pEngineName = "horseradish";
		appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;
		createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredInstanceExts.size());
		createInfo.ppEnabledExtensionNames = requiredInstanceExts.data();
		
		if (!initDebug(createInfo))
			return false;

		if (vkCreateInstance(&createInfo, nullptr, &mInstance) != VK_SUCCESS)
			return false;

		if (!setupDebug())
			return false;

		return true;
	}

	bool App::pickPhysicalDevice()
	{
		std::vector<VkPhysicalDevice> devices;
		{
			uint32_t deviceCount = 0;
			vkEnumeratePhysicalDevices(mInstance, &deviceCount, nullptr);
			if (deviceCount <= 0)
			{
				mErrorMsg = "Unable to find GPUs with Vulkan support.";
				return false;
			}

			devices.resize(deviceCount);
			vkEnumeratePhysicalDevices(mInstance, &deviceCount, devices.data());
		}

		auto rankDevice = [this](VkPhysicalDevice device) -> uint32_t
		{
			uint32_t rank{0};

			//check properties
			{
				VkPhysicalDeviceProperties deviceProperties;
				vkGetPhysicalDeviceProperties(device, &deviceProperties);

				if ((VK_API_VERSION_MAJOR(deviceProperties.apiVersion) != 1) || (VK_API_VERSION_MINOR(deviceProperties.apiVersion) != 3))
					return 0;
				if (deviceProperties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
					return 0;
			}

			//check features
			{
				VkPhysicalDeviceFeatures deviceFeatures;
				vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

				if (deviceFeatures.samplerAnisotropy != VK_TRUE)
					return 0;
				if (deviceFeatures.textureCompressionBC != VK_TRUE)
					return 0;
			}

			//check extensions
			{
				uint32_t extensionCount;
				vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

				std::vector<VkExtensionProperties> exts(extensionCount);
				vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, exts.data());

				for (auto requiredExt : requiredDeviceExts)
				{
					std::string_view requiredExtView{requiredExt};
					if (std::any_of(exts.begin(), exts.end(), [&requiredExtView](const auto& ext) { return std::string_view{ext.extensionName} == requiredExtView; }))
						continue;

					return 0;
				}
			}

			//check queues
			{
				struct QueueFamilyData
				{
					uint32_t index;
					VkQueueFamilyProperties qFamily;
				};

				std::vector<QueueFamilyData> queueFamilies;
				{
					uint32_t queueFamilyCount{0};
					vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
					if (queueFamilyCount <= 0)
						return 0;

					std::vector<VkQueueFamilyProperties> queue(queueFamilyCount);
					vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queue.data());

					queueFamilies.reserve(queueFamilyCount);
					for (uint32_t curIndex = 0; curIndex < queue.size(); ++curIndex)
						queueFamilies.push_back(QueueFamilyData{.index = curIndex, .qFamily = queue[curIndex]});
				}

				//eliminate families that don't have the necessary requirements
				std::erase_if(queueFamilies, [this, device](const auto& family)
				{
					VkBool32 presentSupport = false;
					vkGetPhysicalDeviceSurfaceSupportKHR(device, family.index, mSurface, &presentSupport);
					return (presentSupport == VK_FALSE);
				});
				if (queueFamilies.empty())
					return 0;
				
				//order families by: supporting graphics and then by number of flags
				assert(!queueFamilies.empty());
				std::sort(queueFamilies.begin(), queueFamilies.end(), [](const auto& familyA, const auto& familyB)
				{
					if ((familyA.qFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) && ((familyB.qFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0))
						return true;
					if (std::popcount(familyA.qFamily.queueFlags) > std::popcount(familyB.qFamily.queueFlags))
						return true;
					
					return false;
				});

				//need to have graphics
				if ((queueFamilies.front().qFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0)
					return 0;

				rank = std::accumulate(queueFamilies.begin(), queueFamilies.end(), rank, [](const auto rank, const auto& family)
				{
					return (rank + std::popcount(family.qFamily.queueFlags));
				});
			}

			//return accumulated rank
			return rank;
		};

		struct DeviceRank
		{
			uint32_t rank;
			VkPhysicalDevice device;

			std::strong_ordering operator<=>(const DeviceRank& o) const noexcept
			{
				return rank <=> o.rank;
			}
		};
		std::set<DeviceRank> finalDevices;

		for (const auto& device : devices)
		{
			auto rank = rankDevice(device);
			if (rank <= 0)
				continue; //discard device

			finalDevices.emplace(DeviceRank{.rank = rank, .device = device });
		}

		//just use the last element (with the highest rank)
		if (!finalDevices.empty())
			mPhysicalDevice = finalDevices.rbegin()->device;

		if (mPhysicalDevice == VK_NULL_HANDLE)
		{
			mErrorMsg = "Unable to find a suitable GPU.";
			return false;
		}

		return true;
	}

	bool App::initLogicalDevice()
	{
		std::vector<VkQueueFamilyProperties> queueFamilies;
		{
			uint32_t queueFamilyCount{0};
			vkGetPhysicalDeviceQueueFamilyProperties(mPhysicalDevice, &queueFamilyCount, nullptr);
			if (queueFamilyCount <= 0)
				return 0;

			queueFamilies.resize(queueFamilyCount);
			vkGetPhysicalDeviceQueueFamilyProperties(mPhysicalDevice, &queueFamilyCount, queueFamilies.data());
		}

		uint32_t bestIndex = 0;
		for (uint32_t curIndex = 1; curIndex < queueFamilies.size(); curIndex++)
		{
			if ((queueFamilies[bestIndex].queueFlags & VK_QUEUE_GRAPHICS_BIT) && ((queueFamilies[curIndex].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0))
				continue;

			if (std::popcount(queueFamilies[bestIndex].queueFlags) > std::popcount(queueFamilies[curIndex].queueFlags))
				continue;

			bestIndex = curIndex; //current is better
		}

		std::array<float, 1> queuePriorities{1.0f};

		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = bestIndex;
		queueCreateInfo.queueCount = static_cast<uint32_t>(queuePriorities.size());
		queueCreateInfo.pQueuePriorities = queuePriorities.data();

		VkPhysicalDeviceFeatures deviceFeatures{};
		deviceFeatures.samplerAnisotropy = VK_TRUE;
		deviceFeatures.textureCompressionBC = VK_TRUE;

		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pQueueCreateInfos = &queueCreateInfo;
		createInfo.queueCreateInfoCount = 1;
		createInfo.pEnabledFeatures = &deviceFeatures;
		createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredDeviceExts.size());
		createInfo.ppEnabledExtensionNames = requiredDeviceExts.data();

		if (vkCreateDevice(mPhysicalDevice, &createInfo, nullptr, &mDevice) != VK_SUCCESS)
		{
			mErrorMsg = "Failed to create logical device.";
			return false;
		}

		mGraphicsQueueIndex = bestIndex;
		vkGetDeviceQueue(mDevice, mGraphicsQueueIndex, 0, &mGraphicsQueue);

		return true;
	}

	bool App::initSwapChain()
	{
		VkSurfaceCapabilitiesKHR capabilities;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(mPhysicalDevice, mSurface, &capabilities);

		{
			uint32_t count;
			vkGetPhysicalDeviceSurfaceFormatsKHR(mPhysicalDevice, mSurface, &count, nullptr);

			if (count <= 0)
			{
				mErrorMsg = "No suitable surface formats available in the device.";
				return false;
			}

			std::vector<VkSurfaceFormatKHR> formats(count);
			vkGetPhysicalDeviceSurfaceFormatsKHR(mPhysicalDevice, mSurface, &count, formats.data());

			VkSurfaceFormatKHR expected{.format = VK_FORMAT_B8G8R8A8_SRGB, .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
			if (std::find_if(formats.begin(), formats.end(), [&expected](const auto& format){ return ((format.format == expected.format) && (format.colorSpace == expected.colorSpace)); }) == formats.end())
			{
				mErrorMsg = "No suitable surface formats available in the device.";
				return false;
			}

			mSwapChainSurfaceFormat = expected;
		}

		VkPresentModeKHR presentMode{VK_PRESENT_MODE_FIFO_KHR};
		{
			uint32_t count;
			vkGetPhysicalDeviceSurfacePresentModesKHR(mPhysicalDevice, mSurface, &count, nullptr);

			if (count > 0)
			{
				std::vector<VkPresentModeKHR> presentModes(count);
				vkGetPhysicalDeviceSurfacePresentModesKHR(mPhysicalDevice, mSurface, &count, presentModes.data());

				if (std::find(presentModes.begin(), presentModes.end(), VK_PRESENT_MODE_MAILBOX_KHR) != presentModes.end())
					presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
			}
		}

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = mSurface;
		createInfo.minImageCount = std::min(capabilities.minImageCount + 1, capabilities.maxImageCount);
		createInfo.imageFormat = mSwapChainSurfaceFormat.format;
		createInfo.imageColorSpace = mSwapChainSurfaceFormat.colorSpace;
		createInfo.imageExtent = capabilities.maxImageExtent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;
		createInfo.preTransform = capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = VK_NULL_HANDLE;

		if (vkCreateSwapchainKHR(mDevice, &createInfo, nullptr, &mSwapChain) != VK_SUCCESS)
		{
			mErrorMsg = "Error creating swap chain.";
			return false;
		}

		mSwapChainExtent = capabilities.maxImageExtent;

		{
			uint32_t count;
			vkGetSwapchainImagesKHR(mDevice, mSwapChain, &count, nullptr);

			mSwapChainImages.resize(count);
			vkGetSwapchainImagesKHR(mDevice, mSwapChain, &count, mSwapChainImages.data());

			mSwapChainImageViews.reserve(count);
			for (uint32_t i = 0; i < count; i++)
			{
				mSwapChainImageViews.emplace_back(ImageView::gen2D(mDevice, mSwapChainImages[i], mSwapChainSurfaceFormat.format));
				if (!mSwapChainImageViews.back().isValid())
				{
					mErrorMsg = "Error creating swap chain image view.";
					return false;
				}
			}
		}

		return true;
	}

	App::~App() noexcept
	{
		if (!mInitialized)
			return;

		mSwapChainFramebuffers.clear();
		mSwapChainImageViews.clear();
		vkDestroySwapchainKHR(mDevice, mSwapChain, nullptr);
		vkDestroySurfaceKHR(mInstance, mSurface, nullptr);
		vkDestroyDevice(mDevice, nullptr);
		vkDestroyInstance(mInstance, nullptr);
	}

	bool App::init(SetupSurface setupSurfaceCb)
	{
		if (std::exchange(mInitialized, true))
			return true;

		if (!initInstance())
			return false;

		//this part depends on the platform, so we have to outsource it
		{
			assert(setupSurfaceCb);
			VkSurfaceKHR surface = setupSurfaceCb(mInstance);
			if (surface == VK_NULL_HANDLE)
				return false;

			mSurface = surface;
		}

		if (!pickPhysicalDevice())
			return false;

		if (!initLogicalDevice())
			return false;

		if (!initSwapChain())
			return false;

		return true;
	}

	bool App::swapChainInit(VkRenderPass renderPass)
	{
		if (mSwapChainImageViews.empty())
			return false;

		if (!mSwapChainFramebuffers.empty())
			return false;

		mSwapChainFramebuffers.reserve(mSwapChainImageViews.size());
		for (const auto& imageView : mSwapChainImageViews)
		{
			auto framebuffer = Framebuffer::gen(mDevice, renderPass, {imageView.native()}, mSwapChainExtent.width, mSwapChainExtent.height);
			if (!framebuffer)
			{
				mSwapChainFramebuffers.clear();
				return false;
			}

			mSwapChainFramebuffers.push_back(std::move(framebuffer));
		}

		return true;
	}

	CommandPool App::createCommandPool()
	{
		return CommandPool::gen(mDevice, mGraphicsQueueIndex);
	}

	void App::invokeDebugCb(App::DebugMessageSeverity severity, App::App::DebugMessageContext context, std::string_view source, std::string_view msg) const noexcept
	{
		assert(mDebugCb);
		mDebugCb(severity, context, source, msg);
	}
}
