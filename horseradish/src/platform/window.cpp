#include "window.hpp"

#include "windowImplWin32.hpp"

namespace hr::platform
{
	void Window::MsgBoxInfo(std::string_view msg)
	{
		WindowImpl::MsgBoxInfo(msg);
	}

	void Window::MsgBoxWarn(std::string_view msg)
	{
		WindowImpl::MsgBoxWarn(msg);
	}

	void Window::MsgBoxError(std::string_view msg)
	{
		WindowImpl::MsgBoxError(msg);
	}

	Window::Window(hr::engine::Logger &logger)
		: mImpl{ new WindowImpl(logger) }
	{ }

	Window::~Window()
	{ }

	std::string Window::getErrorMsg() const
	{
		return mImpl->getErrorMsg();
	}

	bool Window::windowInit(std::string_view windowTitle, WindowStyle style, bool targetSecondaryDisplay, const size_t targetWidth, const size_t targeHeight)
	{
		return mImpl->windowInit(windowTitle, style, targetSecondaryDisplay, targetWidth, targeHeight);
	}

	size_t Window::getDisplayWidth() const
	{
		return mImpl->getDisplayWidth();
	}

	size_t Window::getDisplayHeight() const
	{
		return mImpl->getDisplayHeight();
	}

	bool Window::setWindowAlpha(const unsigned char &valorAlpha) const
	{
		return mImpl->setWindowAlpha(valorAlpha);
	}

	bool Window::sendMessageClose() const
	{
		return mImpl->sendMessageClose();
	}

	bool Window::setFocus() const
	{
		return mImpl->setFocus();
	}

	void Window::rawInputSnapshot()
	{
		mImpl->rawInputSnapshot();
	}

	bool Window::rawInputGetKeyStatus(const unsigned int &vcode)
	{
		return mImpl->rawInputGetKeyStatus(vcode);
	}

	bool Window::rawInputGetKeyStatus(const Window::VirtualKeys &vcode)
	{
		return mImpl->rawInputGetKeyStatus(vcode);
	}

	hr::Vector3f Window::rawInputGetMouseStatus()
	{
		return mImpl->rawInputGetMouseStatus();
	}

	int Window::messageLoop(const std::function<void()>& closingCb)
	{
		return mImpl->messageLoop(closingCb);
	}

	void Window::processMessages(const std::function<void(const Message&)>& cb, const bool resetQueue)
	{
		mImpl->processMessages(cb, resetQueue);
	}

	VkSurfaceKHR Window::setupVulkanSurface(VkInstance vulkanInstance)
	{
		return mImpl->setupVulkanSurface(vulkanInstance);
	}

	OpenglContext::OpenglContext(const Window &window, std::string_view openGLModuleName, int contextMajorVersion, int contextMinorVersion, bool contextDebug, bool contextForwardCompatible)
		: mImpl(new OpenglContextImpl(*window.mImpl, openGLModuleName, contextMajorVersion, contextMinorVersion, contextDebug, contextForwardCompatible))
	{
		mIsValid = initContext();
	}

	bool OpenglContext::isValid() const
	{
		return (mImpl->isValid() && mIsValid);
	}

	std::string OpenglContext::getErrorMsg() const
	{
		return mImpl->getErrorMsg();
	}

	void OpenglContext::setSwapInterval(size_t interval) const
	{
		mImpl->setSwapInterval(interval);
	}

	bool OpenglContext::swapBuffers() const
	{
		return mImpl->swapBuffers();
	}
}
