#include "window.hpp"

#include "windowImplWin32.hpp"

Window::Window(HorseRadish::Engine::Logger &logger)
	: mImpl(new WindowImpl(logger))
{
}

Window::~Window()
{
}

std::string Window::GetErrorMsg() const
{
	return mImpl->GetErrorMsg();
}

bool Window::WindowInit(const std::string& windowTitle, const unsigned int winWidth, const unsigned int winHeight, const bool winFullscreen)
{
	return mImpl->WindowInit(windowTitle, winWidth, winHeight, winFullscreen);
}

bool Window::WindowEditorInit(const std::string& windowTitle, const unsigned int winWidth, const unsigned int winHeight, const HWND handleWindowParent)
{
	return mImpl->WindowEditorInit(windowTitle, winWidth, winHeight, handleWindowParent);
}

bool Window::SetWindowAlpha(const unsigned char &valorAlpha) const
{
	return mImpl->SetWindowAlpha(valorAlpha);
}

bool Window::SendMessageClose() const
{
	return mImpl->SendMessageClose();
}

bool Window::SetFocus() const
{
	return mImpl->SetFocus();
}

void Window::RawInputSnapshot()
{
	mImpl->RawInputSnapshot();
}

bool Window::RawInputGetKeyStatus(const unsigned int &vcode)
{
	return mImpl->RawInputGetKeyStatus(vcode);
}

bool Window::RawInputGetKeyStatus(const Window::VirtualKeys &vcode)
{
	return mImpl->RawInputGetKeyStatus(vcode);
}

HorseRadish::Vector3f Window::RawInputGetMouseStatus()
{
	return mImpl->RawInputGetMouseStatus();
}

int Window::MessageLoop(std::function<void()> closingCb)
{
	return mImpl->MessageLoop(closingCb);
}

void Window::ProcessMessages(std::function<void(const Message&)> cb, const bool resetQueue)
{
	mImpl->ProcessMessages(cb, resetQueue);
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
	: mIsValid(false)
	, mImpl(new OpenglContextImpl(*window.mImpl, openGLModuleName, contextMajorVersion, contextMinorVersion, contextDebug, contextForwardCompatible))
{
	mIsValid = initContext();
}

bool OpenglContext::isValid() const
{
	return (mImpl->IsValid() & mIsValid);
}

std::string OpenglContext::getErrorMsg() const
{
	return mImpl->GetErrorMsg();
}

void OpenglContext::setSwapInterval(const unsigned int &interval) const
{
	mImpl->SetSwapInterval(interval);
}

bool OpenglContext::swapBuffers() const
{
	return mImpl->SwapBuffers();
}
