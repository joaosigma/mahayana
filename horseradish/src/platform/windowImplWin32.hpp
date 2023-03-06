#pragma once

#include "../platform/platform.hpp"

#if defined(HR_BUILD_WINDOWS)

#include "window.hpp"

#include "../common/vector.hpp"
#include "../common/opengl/objects.hpp"
#include "../engine/logger.hpp"

#include "wglext.h"

#include <array>
#include <mutex>

#include <windows.h>

namespace hr::platform
{
	class WindowImpl
	{
		friend class OpenglContextImpl;

		HWND mHWnd{ nullptr };
		HMODULE mHModule{ nullptr };
		bool mIsInitialized{ false };
		bool mCloseRequested{ false };
		std::string mErrorMsg;
		std::wstring mClassName;
		DEVMODE mOriginalDeviceMode;
		hr::engine::Logger &mLogger;

		struct {
			size_t width{ 0 }, height{ 0 };
			size_t resizeWidth{ 0 }, resizeHeight{ 0 };
		} mDisplayInfo;

		struct {
			std::mutex lock;
			hr::Vector3f mouseSnapshot, mouseAccum;
			std::array<bool, 128> keysSnapshot, keysRealtime;
		} mRawInput;

		struct {
			std::mutex lock;
			size_t queueSize{ 0 };
			std::array<Window::Message, 1024> queue;
		} mEvents;

	private:
		static LRESULT CALLBACK wndProc(HWND hWnd, UINT messageID, WPARAM wParam, LPARAM lParam);
		static int32_t translateVirtualKeyCode(LPARAM nativeKeyCode);

		void processRawInput(const RAWINPUT &inputData);

	public:
		static void MsgBoxInfo(std::string_view msg);
		static void MsgBoxWarn(std::string_view msg);
		static void MsgBoxError(std::string_view msg);

	public:
		WindowImpl(hr::engine::Logger &logger);
		~WindowImpl();

		std::string getErrorMsg() const;

		bool windowInit(std::string_view windowTitle, Window::WindowStyle style, bool targetSecondaryDisplay, const unsigned int targetWidth, const unsigned int targeHeight);

		size_t getDisplayWidth() const;
		size_t getDisplayHeight() const;

		bool setWindowAlpha(const unsigned char &valorAlpha) const;
		bool sendMessageClose() const;
		bool setFocus() const;

		void rawInputSnapshot();
		bool rawInputGetKeyStatus(const unsigned int &vcode);
		bool rawInputGetKeyStatus(const Window::VirtualKeys &vcode);
		hr::Vector3f rawInputGetMouseStatus();

		int messageLoop(const std::function<void()>& closingCb);
		void processMessages(const std::function<void(const Window::Message&)>& cb, const bool resetQueue);

		VkSurfaceKHR setupVulkanSurface(VkInstance vulkanInstance);
	};

	class OpenglContextImpl
	{
		HDC mHDC;
		HGLRC mHRC;
		std::string mErrorMsg;
		const WindowImpl &mWindow;
		unsigned int mUsedPFD;

		struct
		{
			HGLRC(APIENTRY *createContext)	(HDC hdc) { nullptr };
			BOOL(APIENTRY *makeCurrent)		(HDC hdc, HGLRC hglrc) { nullptr };
			BOOL(APIENTRY *deleteContext)	(HGLRC hglrc) { nullptr };
			BOOL(APIENTRY *swapBuffers)		(HDC hdc) { nullptr };

#ifdef WGL_ARB_create_context
			PFNWGLCREATECONTEXTATTRIBSARBPROC createContextAttribsARB{ nullptr };
#endif

#ifdef WGL_ARB_extensions_string
			PFNWGLGETEXTENSIONSSTRINGARBPROC getExtensionsStringARB{ nullptr };
#endif

#ifdef WGL_ARB_pixel_format
			PFNWGLGETPIXELFORMATATTRIBIVARBPROC getPixelFormatAttribivARB{ nullptr };
			PFNWGLGETPIXELFORMATATTRIBFVARBPROC getPixelFormatAttribfvARB{ nullptr };
			PFNWGLCHOOSEPIXELFORMATARBPROC choosePixelFormatARB{ nullptr };
#endif

#ifdef WGL_EXT_swap_control
			PFNWGLSWAPINTERVALEXTPROC swapIntervalEXT{ nullptr };
			PFNWGLGETSWAPINTERVALEXTPROC getSwapIntervalEXT{ nullptr };
#endif
		} mWGL;

		void loadWGLFunctions(HMODULE openglModule);
		bool auxWindowWGLExt(HINSTANCE hInstance, HMODULE openglModule);

	public:
		OpenglContextImpl(const WindowImpl &window, std::string_view openGLModuleName, int contextMajorVersion, int contextMinorVersion, bool contextDebug, bool contextForwardCompatible);
		~OpenglContextImpl();

		bool isValid() const;
		std::string getErrorMsg() const;

		void setSwapInterval(const size_t &interval) const;
		bool swapBuffers(void) const;
	};
}

#endif
