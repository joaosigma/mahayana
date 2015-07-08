#pragma once

#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32)) && !defined(__CYGWIN__)

#include "window.hpp"

#include "common\Vector.hpp"
#include "common\Platform.hpp"
#include "common\opengl\objects.hpp"
#include "..\engine\logger.hpp"

#include "wglext.h"

#include <array>
#include <mutex>

#include <windows.h>

class WindowImpl
{
	friend class OpenglContextImpl;

private:
	HWND hWnd;
	HMODULE hModule;
	bool closeRequested;
	std::string errorMsg;
	std::wstring mClassName;
	DEVMODE originalDeviceMode;
	bool isFullscreen, isInitialized;
	HorseRadish::Engine::Logger &mLogger;

	struct {
		std::mutex lock;
		HorseRadish::Vector3f mouseSnapshot, mouseAccum;
		std::array<bool, 128> keysSnapshot, keysRealtime;
	} mRawInput;

	struct {
		std::mutex lock;
		unsigned int queueSize;
		std::array<Window::Message, 1024> queue;
	} mEvents;
	
private:
	static LRESULT CALLBACK wndProc(HWND hWnd, UINT messageID, WPARAM wParam, LPARAM lParam);
	static HorseRadish::hInt32 translateVirtualKeyCode(LPARAM nativeKeyCode);

	void processRawInput(const RAWINPUT &inputData);
	
public:
	WindowImpl(HorseRadish::Engine::Logger &logger);
	~WindowImpl();

	std::string GetErrorMsg() const;

	bool WindowInit(const std::string& windowTitle, const unsigned int winWidth, const unsigned int winHeight, const bool winFullscreen);
	bool WindowEditorInit(const std::string& windowTitle, const unsigned int winWidth, const unsigned int winHeight, const HWND handleWindowParent);

	bool SetWindowAlpha(const unsigned char &valorAlpha) const;
	bool SendMessageClose() const;
	bool SetFocus() const;

	void RawInputSnapshot();
	bool RawInputGetKeyStatus(const unsigned int &vcode);
	bool RawInputGetKeyStatus(const Window::VirtualKeys &vcode);
	HorseRadish::Vector3f RawInputGetMouseStatus();
	
	int MessageLoop(std::function<void()> closingCb);
	void ProcessMessages(std::function<void(const Window::Message&)> cb, const bool resetQueue);

	static void MsgBoxInfo(const std::string& msg);
	static void MsgBoxInfo(const char * const msg);
	static void MsgBoxWarn(const std::string& msg);
	static void MsgBoxWarn(const char * const msg);
	static void MsgBoxError(const std::string& msg);
	static void MsgBoxError(const char * const msg);
};

class OpenglContextImpl
{
	HDC hDC;
	HGLRC hRC;
	std::string errorMsg;
	const WindowImpl &window;
	unsigned int usedPFD;

	HGLRC (APIENTRY *wglCreateContext)	(HDC hdc);
	BOOL  (APIENTRY *wglMakeCurrent)	(HDC hdc, HGLRC hglrc);
	BOOL  (APIENTRY *wglDeleteContext)	(HGLRC hglrc);
	BOOL  (APIENTRY *wglSwapBuffers)	(HDC hdc);

#ifdef WGL_ARB_create_context
	PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB;
#endif

#ifdef WGL_ARB_extensions_string
	PFNWGLGETEXTENSIONSSTRINGARBPROC wglGetExtensionsStringARB;
#endif

#ifdef WGL_ARB_pixel_format
	PFNWGLGETPIXELFORMATATTRIBIVARBPROC wglGetPixelFormatAttribivARB;
	PFNWGLGETPIXELFORMATATTRIBFVARBPROC wglGetPixelFormatAttribfvARB;
	PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB;
#endif

#ifdef WGL_EXT_swap_control
	PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT;
	PFNWGLGETSWAPINTERVALEXTPROC wglGetSwapIntervalEXT;
#endif
	
	void loadWGLFunctions(HMODULE openglModule);
	bool auxWindowWGLExt(HINSTANCE hInstance, HMODULE openglModule);

public:
	OpenglContextImpl(const WindowImpl &window, const std::string& openGLModuleName, int contextMajorVersion, int contextMinorVersion, bool contextDebug, bool contextForwardCompatible);
	~OpenglContextImpl();

	bool IsValid() const;
	std::string GetErrorMsg() const;

	void SetSwapInterval(const unsigned int &interval) const;
	bool SwapBuffers(void) const;
};

#endif
