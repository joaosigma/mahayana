#pragma once

#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32)) && !defined(__CYGWIN__)

#include "window.hpp"

#include "../platform/platform.hpp"
#include "../common/vector.hpp"
#include "../common/opengl/objects.hpp"
#include "../engine/logger.hpp"

#include "wglext.h"

#include <array>
#include <mutex>

#include <windows.h>

namespace hr { namespace platform
{
	class WindowImpl
	{
		friend class OpenglContextImpl;

		HWND mHWnd = nullptr;
		HMODULE mHModule = nullptr;
		bool mIsInitialized = false;
		bool mCloseRequested = false;
		std::string mErrorMsg;
		std::wstring mClassName;
		DEVMODE mOriginalDeviceMode;
		hr::engine::Logger &mLogger;

		struct {
			size_t width = 0, height = 0;
			size_t resizeWidth = 0, resizeHeight = 0;
		} mDisplayInfo;

		struct {
			std::mutex lock;
			hr::Vector3f mouseSnapshot, mouseAccum;
			std::array<bool, 128> keysSnapshot, keysRealtime;
		} mRawInput;

		struct {
			std::mutex lock;
			size_t queueSize = 0;
			std::array<Window::Message, 1024> queue;
		} mEvents;
	
	private:
		static LRESULT CALLBACK wndProc(HWND hWnd, UINT messageID, WPARAM wParam, LPARAM lParam);
		static hr::hInt32 translateVirtualKeyCode(LPARAM nativeKeyCode);

		void processRawInput(const RAWINPUT &inputData);
	
	public:
		WindowImpl(hr::engine::Logger &logger);
		~WindowImpl();

		std::string getErrorMsg() const;

		bool windowInit(const std::string& windowTitle, Window::WindowStyle style, bool targetSecondaryDisplay, const unsigned int targetWidth, const unsigned int targeHeight);

		size_t getDisplayWidth() const;
		size_t getDisplayHeight() const;

		bool setWindowAlpha(const unsigned char &valorAlpha) const;
		bool sendMessageClose() const;
		bool setFocus() const;

		void rawInputSnapshot();
		bool rawInputGetKeyStatus(const unsigned int &vcode);
		bool rawInputGetKeyStatus(const Window::VirtualKeys &vcode);
		hr::Vector3f rawInputGetMouseStatus();
	
		int messageLoop(std::function<void()> closingCb);
		void processMessages(std::function<void(const Window::Message&)> cb, const bool resetQueue);

		static void MsgBoxInfo(const std::string& msg);
		static void MsgBoxInfo(const char * const msg);
		static void MsgBoxWarn(const std::string& msg);
		static void MsgBoxWarn(const char * const msg);
		static void MsgBoxError(const std::string& msg);
		static void MsgBoxError(const char * const msg);
	};

	class OpenglContextImpl
	{
		HDC mHDC;
		HGLRC mHRC;
		std::string mErrorMsg;
		const WindowImpl &mWindow;
		unsigned int mUsedPFD;

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

		bool isValid() const;
		std::string getErrorMsg() const;

		void setSwapInterval(const size_t &interval) const;
		bool swapBuffers(void) const;
	};

} }

#endif
