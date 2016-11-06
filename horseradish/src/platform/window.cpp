#include "window.hpp"

#include "windowImplWin32.hpp"

namespace HorseRadish { namespace platform
{
	Window::Window(HorseRadish::Engine::Logger &logger)
		: mImpl(new WindowImpl(logger))
	{ }

	Window::~Window()
	{ }

	std::string Window::getErrorMsg() const
	{
		return mImpl->getErrorMsg();
	}

	bool Window::windowInit(const std::string& windowTitle, WindowStyle style, bool targetSecondaryDisplay, const size_t targetWidth, const size_t targeHeight)
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

	HorseRadish::Vector3f Window::rawInputGetMouseStatus()
	{
		return mImpl->rawInputGetMouseStatus();
	}

	int Window::messageLoop(std::function<void()> closingCb)
	{
		return mImpl->messageLoop(closingCb);
	}

	void Window::processMessages(std::function<void(const Message&)> cb, const bool resetQueue)
	{
		mImpl->processMessages(cb, resetQueue);
	}

	void Window::MsgBoxInfo(const std::string& msg)
	{
		WindowImpl::MsgBoxInfo(msg);
	}

	void Window::MsgBoxInfo(const char * const msg)
	{
		WindowImpl::MsgBoxInfo(msg);
	}

	void Window::MsgBoxWarn(const std::string& msg)
	{
		WindowImpl::MsgBoxWarn(msg);
	}

	void Window::MsgBoxWarn(const char * const msg)
	{
		WindowImpl::MsgBoxWarn(msg);
	}

	void Window::MsgBoxError(const std::string& msg)
	{
		WindowImpl::MsgBoxError(msg);
	}

	void Window::MsgBoxError(const char * const msg)
	{
		WindowImpl::MsgBoxError(msg);
	}

	OpenglContext::OpenglContext(const Window &window, const std::string& openGLModuleName, int contextMajorVersion, int contextMinorVersion, bool contextDebug, bool contextForwardCompatible)
		: mImpl(new OpenglContextImpl(*window.mImpl, openGLModuleName, contextMajorVersion, contextMinorVersion, contextDebug, contextForwardCompatible))
	{
		mIsValid = initContext();
	}

	bool OpenglContext::isValid() const
	{
		return (mImpl->isValid() & mIsValid);
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

}}
