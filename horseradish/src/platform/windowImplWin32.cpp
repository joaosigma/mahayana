#include "windowImplWin32.hpp"

#if defined(HR_BUILD_WINDOWS)

#include "common/stringUtils.hpp"
#include "common/opengl/openGL.hpp"

#include <Windowsx.h>

namespace hr::platform
{
	static
	bool retrieveMonitorArea(RECT& monitorArea, bool secondaryIfAvailable, bool fullArea)
	{
		struct CallbackData
		{
			bool foundPrimary = false;
			RECT primaryAreaFull, primaryAreaWork;

			bool foundSecondary = false;
			RECT secondaryAreaFull, secondaryAreaWork;
		} cbData;

		EnumDisplayMonitors(nullptr, nullptr, [](HMONITOR hMonitor, HDC, LPRECT, LPARAM dwData) -> BOOL
		{
			MONITORINFO monitorInfo;
			memset(&monitorInfo, 0, sizeof(MONITORINFO));
			monitorInfo.cbSize = sizeof(MONITORINFO);

			if (!GetMonitorInfo(hMonitor, &monitorInfo))
				return TRUE;

			auto cbData = reinterpret_cast<CallbackData*>(dwData);

			if ((monitorInfo.dwFlags & MONITORINFOF_PRIMARY) == MONITORINFOF_PRIMARY)
			{
				cbData->foundPrimary = true;
				cbData->primaryAreaFull = monitorInfo.rcMonitor;
				cbData->primaryAreaWork = monitorInfo.rcWork;
			}
			else if (!cbData->foundSecondary && monitorInfo.rcMonitor.left >= 0)
			{
				cbData->foundSecondary = true;
				cbData->secondaryAreaFull = monitorInfo.rcMonitor;
				cbData->secondaryAreaWork = monitorInfo.rcWork;
			}

			return ((cbData->foundPrimary && cbData->foundSecondary) ? FALSE : TRUE);
		}, reinterpret_cast<LPARAM>(&cbData));

		if (secondaryIfAvailable && cbData.foundSecondary)
		{
			monitorArea = fullArea ? cbData.secondaryAreaFull : cbData.secondaryAreaWork;
			return true;
		}

		if (cbData.foundPrimary)
			monitorArea = fullArea ? cbData.primaryAreaFull : cbData.primaryAreaWork;

		return cbData.foundPrimary;
	}

	static
	LRESULT CALLBACK auxWindowWGLExtProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		return DefWindowProc(hwnd, message, wParam, lParam);
	};

	LRESULT CALLBACK WindowImpl::wndProc(HWND hWnd, UINT messageID, WPARAM wParam, LPARAM lParam)
	{
		if (messageID == WM_NCCREATE)
		{
			CREATESTRUCT *pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);

			SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pCreate->lpCreateParams);
			return DefWindowProc(hWnd, messageID, wParam, lParam);
		}

		WindowImpl* window = reinterpret_cast<WindowImpl*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
		if (!window)
			return DefWindowProc(hWnd, messageID, wParam, lParam);

		if (messageID == WM_CLOSE)
		{
			window->mCloseRequested = true;
			return 0;
		}

		if (messageID == WM_DESTROY)
		{
			PostQuitMessage(0);
			return 0;
		}

		if (messageID == WM_INPUT)
		{
			BYTE bufferAux[sizeof(RAWINPUT)];
			UINT bufferSize = sizeof(RAWINPUT);
			GetRawInputData((HRAWINPUT)lParam, RID_INPUT, bufferAux, &bufferSize, sizeof(RAWINPUTHEADER));

			window->processRawInput(*(reinterpret_cast<RAWINPUT*>(bufferAux)));
			return 0;
		}
	
		if ((messageID == WM_CHAR) || (messageID == WM_KEYDOWN) || (messageID == WM_MOUSEWHEEL))
		{
			std::lock_guard<std::mutex> lock(window->mEvents.lock);

			if (window->mEvents.queueSize >= window->mEvents.queue.size())
			{
				//TODO: create warning
				return DefWindowProc(hWnd, messageID, wParam, lParam);
			}

			if ((messageID == WM_CHAR) || (messageID == WM_KEYDOWN))
			{
				hr::types::hSplitUInt32 flags(0);

				flags.piecesShort.short0 = (lParam & 0xFFFF);
				flags.piecesShort.short1 |= ((GetKeyState(VK_CONTROL) & 0x8000) != 0) ? static_cast<unsigned int>(Window::Message::MessageFlags::ControlKey) : 0;
				flags.piecesShort.short1 |= ((GetKeyState(VK_SHIFT) & 0x8000) != 0) ? static_cast<unsigned int>(Window::Message::MessageFlags::ShiftKey) : 0;

				if (messageID == WM_CHAR)
					window->mEvents.queue[window->mEvents.queueSize++] = Window::Message(Window::Message::MessageType::CharacterKey, wParam, flags);
				else
					window->mEvents.queue[window->mEvents.queueSize++] = Window::Message(Window::Message::MessageType::VirtualKey, WindowImpl::translateVirtualKeyCode(wParam), flags);
			}			
			else if (messageID == WM_MOUSEWHEEL)
			{
				hr::types::hSplitUInt32 flags(0), params(0);

				flags.piecesShort.short0 = static_cast<signed short>(GET_WHEEL_DELTA_WPARAM(wParam) / 120);
				flags.piecesShort.short1 |= ((GET_KEYSTATE_WPARAM(wParam) & MK_CONTROL) != 0) ? static_cast<unsigned int>(Window::Message::MessageFlags::ControlKey) : 0;
				flags.piecesShort.short1 |= ((GET_KEYSTATE_WPARAM(wParam) & MK_SHIFT) != 0) ? static_cast<unsigned int>(Window::Message::MessageFlags::ShiftKey) : 0;

				params.piecesShort.short0 = GET_X_LPARAM(lParam);
				params.piecesShort.short1 = GET_Y_LPARAM(lParam);
				window->mEvents.queue[window->mEvents.queueSize++] = Window::Message(Window::Message::MessageType::MouseWheel, params.valueWord, flags);
			}

			return 0;
		}

		if (messageID == WM_SIZE)
		{
			window->mDisplayInfo.resizeWidth = LOWORD(lParam);
			window->mDisplayInfo.resizeHeight = HIWORD(lParam);
			return 0;
		}

		if (messageID == WM_EXITSIZEMOVE)
		{
			if ((window->mDisplayInfo.resizeWidth != window->mDisplayInfo.width) || (window->mDisplayInfo.resizeHeight != window->mDisplayInfo.height))
			{
				hr::types::hSplitUInt32 params(0);
				params.piecesShort.short0 = static_cast<uint16_t>(window->mDisplayInfo.resizeWidth);
				params.piecesShort.short1 = static_cast<uint16_t>(window->mDisplayInfo.resizeHeight);

				window->mDisplayInfo.width = window->mDisplayInfo.resizeWidth;
				window->mDisplayInfo.height = window->mDisplayInfo.resizeHeight;

				window->mEvents.queue[window->mEvents.queueSize++] = Window::Message(Window::Message::MessageType::Resize, params.valueWord, 0);
			}

			return 0;
		}

		return DefWindowProc(hWnd, messageID, wParam, lParam);
	}

	int32_t WindowImpl::translateVirtualKeyCode(LPARAM nativeKeyCode)
	{
		Window::VirtualKeys virtuakKey;

		if ((nativeKeyCode >= 0x30) && (nativeKeyCode <= 0x39))
			return static_cast<int32_t>(nativeKeyCode); //0-9
		if ((nativeKeyCode >= 0x41) && (nativeKeyCode <= 0x5A))
			return static_cast<int32_t>(nativeKeyCode); //A-Z

		if ((nativeKeyCode >= 0x70) && (nativeKeyCode <= 0x7B))
			return (static_cast<int32_t>(Window::VirtualKeys::F1) + (nativeKeyCode - 0x70)); //F1-F12

		switch (nativeKeyCode)
		{
			case VK_LBUTTON:
				virtuakKey = Window::VirtualKeys::MouseLButton; break;
			case VK_RBUTTON:
				virtuakKey = Window::VirtualKeys::MouseRButton; break;
			case VK_MBUTTON:
				virtuakKey = Window::VirtualKeys::MouseMButton; break;
			case VK_TAB:
				virtuakKey = Window::VirtualKeys::Tab; break;
			case VK_BACK:
				virtuakKey = Window::VirtualKeys::Backspace; break;
			case VK_RETURN:
				virtuakKey = Window::VirtualKeys::Return; break;
			case VK_SHIFT:
				virtuakKey = Window::VirtualKeys::Shift; break;
			case VK_CONTROL:
				virtuakKey = Window::VirtualKeys::Control; break;
			case VK_ESCAPE:
				virtuakKey = Window::VirtualKeys::Escape; break;
			case VK_SPACE:
				virtuakKey = Window::VirtualKeys::Space; break;
			case VK_PRIOR:
				virtuakKey = Window::VirtualKeys::PageUp; break;
			case VK_NEXT:
				virtuakKey = Window::VirtualKeys::PageDown; break;
			case VK_END:
				virtuakKey = Window::VirtualKeys::End; break;
			case VK_HOME:
				virtuakKey = Window::VirtualKeys::Home; break;
			case VK_LEFT:
				virtuakKey = Window::VirtualKeys::Left; break;
			case VK_RIGHT:
				virtuakKey = Window::VirtualKeys::Right; break;
			case VK_UP:
				virtuakKey = Window::VirtualKeys::Up; break;
			case VK_DOWN:
				virtuakKey = Window::VirtualKeys::Down; break;
			case VK_DELETE:
				virtuakKey = Window::VirtualKeys::Delete; break;
			default:
				virtuakKey = Window::VirtualKeys::Invalid; break;
		}

		return static_cast<int32_t>(virtuakKey);
	}

	void WindowImpl::processRawInput(const RAWINPUT &inputData)
	{
		std::lock_guard<std::mutex> lock(mRawInput.lock);

		if (inputData.header.dwType == RIM_TYPEMOUSE)
		{
			mRawInput.mouseAccum[0] += inputData.data.mouse.lLastX;
			mRawInput.mouseAccum[1] += inputData.data.mouse.lLastY;
		}

		if (inputData.header.dwType == RIM_TYPEKEYBOARD)
		{
			bool keyDown = false;
			if (!(inputData.data.keyboard.Flags & RI_KEY_MAKE))
				keyDown = true;
			if (inputData.data.keyboard.Flags & RI_KEY_BREAK)
				keyDown = false;

			auto virtualKeyCode = static_cast<size_t>(WindowImpl::translateVirtualKeyCode(inputData.data.keyboard.VKey));
			if (virtualKeyCode < mRawInput.keysRealtime.size())
				mRawInput.keysRealtime[virtualKeyCode] = keyDown;
		}
	}

	void WindowImpl::MsgBoxInfo(std::string_view msg)
	{
		auto msgWChar = hr::StringUtils::conv2Native(msg);

		MessageBox(nullptr, msgWChar.c_str(), L"Info", MB_OK | MB_ICONINFORMATION);
	}

	void WindowImpl::MsgBoxWarn(std::string_view msg)
	{
		auto msgWChar = hr::StringUtils::conv2Native(msg);

		MessageBox(nullptr, msgWChar.c_str(), L"Warning", MB_OK | MB_ICONWARNING);
	}

	void WindowImpl::MsgBoxError(std::string_view msg)
	{
		auto msgWChar = hr::StringUtils::conv2Native(msg);

		MessageBox(nullptr, msgWChar.c_str(), L"Error", MB_OK | MB_ICONERROR);
	}

	WindowImpl::WindowImpl(hr::engine::Logger &logger)
		: mLogger(logger)
	{
		memset(&mOriginalDeviceMode, 0, sizeof(DEVMODE));

		mRawInput.mouseAccum = mRawInput.mouseSnapshot = hr::Vector3f(0.0f);
		mRawInput.keysSnapshot.fill(false);
		mRawInput.keysRealtime.fill(false);
	}

	WindowImpl::~WindowImpl()
	{
		if (!mOriginalDeviceMode.dmSize)
			return;

		ChangeDisplaySettings(nullptr, 0);
		memset(&mOriginalDeviceMode, 0, sizeof(DEVMODE));
	}

	std::string WindowImpl::getErrorMsg() const
	{
		return mErrorMsg;
	}

	bool WindowImpl::windowInit(std::string_view windowTitle, Window::WindowStyle style, bool targetSecondaryDisplay, const unsigned int targetWidth, const unsigned int targeHeight)
	{
		if (mIsInitialized)
		{
			mErrorMsg = "window already initialized";
			return false;
		}

		if (windowTitle.empty() || ((style == Window::WindowStyle::StyleWindow) && ((targetWidth == 0) || (targeHeight == 0))))
		{
			mErrorMsg = "incorrect data to properly create a window";
			return false;
		}

		mClassName = hr::StringUtils::conv2Native("HorseRadish graphics engine...");
		mHModule = GetModuleHandle(NULL); //safe since this is not a DLL

		{
			WNDCLASSEXW windowClass;

			memset(&windowClass, 0, sizeof(WNDCLASSEXW));
			windowClass.cbSize = sizeof(WNDCLASSEXW);
			windowClass.style = CS_OWNDC;
			windowClass.lpfnWndProc = (WNDPROC)&WindowImpl::wndProc;
			windowClass.cbClsExtra = 0;
			windowClass.cbWndExtra = 0;
			windowClass.hInstance = mHModule;
			windowClass.hIcon = nullptr;
			windowClass.hCursor = nullptr;
			windowClass.hbrBackground = nullptr;
			windowClass.lpszMenuName = nullptr;
			windowClass.lpszClassName = mClassName.c_str();
			if (RegisterClassEx(&windowClass) == 0)
			{
				mErrorMsg = "unable to register window class";
				return false;
			}
		}

		if (style == Window::WindowStyle::StyleFullscreen)
		{
			mOriginalDeviceMode.dmSize = sizeof(DEVMODE);
			mOriginalDeviceMode.dmDriverExtra = 0;
			EnumDisplaySettingsEx(nullptr, ENUM_CURRENT_SETTINGS, &mOriginalDeviceMode, 0);

			if (ChangeDisplaySettings(&mOriginalDeviceMode, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
			{
				memset(&mOriginalDeviceMode, 0, sizeof(DEVMODE));

				mErrorMsg = "unable to change to fullscreen";
				return false;
			}

			DWORD dwExStyle = 0;
			DWORD dwStyle = WS_POPUP | WS_VISIBLE;

			auto windowTitleWChar = hr::StringUtils::conv2Native(windowTitle);

			mHWnd = CreateWindowEx(dwExStyle, mClassName.c_str(), windowTitleWChar.c_str(), dwStyle,
				0, 0, mOriginalDeviceMode.dmPelsWidth, mOriginalDeviceMode.dmPelsHeight,
				HWND_DESKTOP, nullptr, mHModule,
				this);

			mDisplayInfo.resizeWidth = mOriginalDeviceMode.dmPelsWidth;
			mDisplayInfo.resizeHeight = mOriginalDeviceMode.dmPelsHeight;
		}
		else if (style == Window::WindowStyle::StyleFullscreenWindow)
		{
			RECT monitorRect;
			if (!retrieveMonitorArea(monitorRect, targetSecondaryDisplay, true))
			{
				mErrorMsg = "unable to retrieve display device";

				UnregisterClass(mClassName.c_str(), mHModule);
				return false;
			}

			DWORD dwExStyle = 0;
			DWORD dwStyle = WS_POPUP | WS_VISIBLE;
		
			auto windowTitleWChar = hr::StringUtils::conv2Native(windowTitle);

			mHWnd = CreateWindowEx(dwExStyle, mClassName.c_str(), windowTitleWChar.c_str(), dwStyle,
				monitorRect.left, monitorRect.top, monitorRect.right - monitorRect.left, monitorRect.bottom - monitorRect.top,
				HWND_DESKTOP, nullptr, mHModule,
				this);

			mDisplayInfo.resizeWidth = monitorRect.right - monitorRect.left;
			mDisplayInfo.resizeHeight = monitorRect.bottom - monitorRect.top;
		}
		else if(style == Window::WindowStyle::StyleWindow)
		{
			RECT monitorRect;
			if (!retrieveMonitorArea(monitorRect, targetSecondaryDisplay, targetSecondaryDisplay))
			{
				mErrorMsg = "unable to retrieve display device";

				UnregisterClass(mClassName.c_str(), mHModule);
				return false;
			}

			DWORD dwExStyle = 0;
			DWORD dwStyle = WS_CAPTION | WS_VISIBLE;

			RECT windowRect;
			windowRect.left = windowRect.top = 0;
			windowRect.right = targetWidth;
			windowRect.bottom = targeHeight;
			AdjustWindowRectEx(&windowRect, dwStyle, false, dwExStyle);

			if (((windowRect.right - windowRect.left) * (windowRect.bottom - windowRect.top)) > ((monitorRect.right - monitorRect.left) * (monitorRect.bottom - monitorRect.top)))
				windowRect = monitorRect;

			auto windowTitleWChar = hr::StringUtils::conv2Native(windowTitle);

			mHWnd = CreateWindowEx(dwExStyle, mClassName.c_str(), windowTitleWChar.c_str(), dwStyle,
				monitorRect.left, monitorRect.top, (windowRect.right - windowRect.left), (windowRect.bottom - windowRect.top),
				HWND_DESKTOP, nullptr, mHModule,
				this);

			mDisplayInfo.resizeWidth = (windowRect.right - windowRect.left);
			mDisplayInfo.resizeHeight = (windowRect.bottom - windowRect.top);
		}

		if (mHWnd == nullptr)
		{
			mErrorMsg = "unable to create window";
		
			UnregisterClass(mClassName.c_str(), mHModule);
			return false;
		}

		{
			RAWINPUTDEVICE rawInputDevice[2];

			rawInputDevice[0].usUsagePage = 0x01; //HID_USAGE_PAGE_GENERIC
			rawInputDevice[0].usUsage = 0x02; //HID_USAGE_GENERIC_MOUSE
			rawInputDevice[0].dwFlags = 0;
			rawInputDevice[0].hwndTarget = mHWnd;
		
			rawInputDevice[1].usUsagePage = 0x01; //HID_USAGE_PAGE_GENERIC
			rawInputDevice[1].usUsage = 0x06; //HID_USAGE_GENERIC_KEYBOARD
			rawInputDevice[1].dwFlags = 0;
			rawInputDevice[1].hwndTarget = mHWnd;

			if (RegisterRawInputDevices(rawInputDevice, 2, sizeof(rawInputDevice[0])) != TRUE)
				mLogger.logError(hr::engine::Logger::ModuleType::Graphics, "unable to register raw input devices");
		}

		SetCursor(nullptr);

		mIsInitialized = true;
		return true;
	}

	size_t WindowImpl::getDisplayWidth() const
	{
		return mDisplayInfo.resizeWidth;
	}

	size_t WindowImpl::getDisplayHeight() const
	{
		return mDisplayInfo.resizeHeight;
	}

	bool WindowImpl::setWindowAlpha(const unsigned char &valorAlpha) const
	{
		SetWindowLong(mHWnd, GWL_EXSTYLE, GetWindowLong(mHWnd, GWL_EXSTYLE) | WS_EX_LAYERED);

		return (SetLayeredWindowAttributes(mHWnd, RGB(0,0,0), (valorAlpha < 10) ? 10 : valorAlpha, LWA_ALPHA) == TRUE);
	}

	bool WindowImpl::sendMessageClose() const
	{
		return (SendNotifyMessage(mHWnd, WM_CLOSE, 0, 0) == TRUE);
	}

	bool WindowImpl::setFocus() const
	{
		return (::SetFocus(mHWnd) != nullptr);
	}

	void WindowImpl::rawInputSnapshot()
	{
		std::lock_guard<std::mutex> lock(mRawInput.lock);

		mRawInput.mouseSnapshot = mRawInput.mouseAccum;
		mRawInput.mouseAccum.set(0.0f);

		mRawInput.keysSnapshot = mRawInput.keysRealtime;
	}

	bool WindowImpl::rawInputGetKeyStatus(const unsigned int &vcode)
	{
		std::lock_guard<std::mutex> lock(mRawInput.lock);

		if (vcode >= mRawInput.keysSnapshot.size())
			return false;
		return mRawInput.keysSnapshot[vcode];
	}

	bool WindowImpl::rawInputGetKeyStatus(const Window::VirtualKeys &vcode)
	{
		return WindowImpl::rawInputGetKeyStatus(static_cast<unsigned int>(vcode));
	}

	hr::Vector3f WindowImpl::rawInputGetMouseStatus()
	{
		std::lock_guard<std::mutex> lock(mRawInput.lock);

		return mRawInput.mouseSnapshot;
	}

	int WindowImpl::messageLoop(const std::function<void()>& closingCb)
	{
		MSG msg;
		BOOL returnCode;

		if (!mIsInitialized)
			return -1;

		//main loop
		mCloseRequested = false;
		while ((returnCode = GetMessage(&msg, NULL, 0, 0)) != 0)
		{
			if (returnCode == -1)
			{
				mLogger.logError(hr::engine::Logger::ModuleType::Graphics, "Error in window message loop");
				continue;
			}

			if (mCloseRequested)
			{
				if (closingCb)
					closingCb();

				if (mHWnd != nullptr)
				{
					if (DestroyWindow(mHWnd) == FALSE)
						mLogger.logError(hr::engine::Logger::ModuleType::Graphics, "Unable to delete window handle");

					mHWnd = nullptr;
				}

				mCloseRequested = false;
			}

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		//cleanup
		if (UnregisterClass(mClassName.c_str(), mHModule) == FALSE)
			mLogger.logError(hr::engine::Logger::ModuleType::Graphics, "Unable to unregister window class");

		mIsInitialized = false;

		//return exit code
		return msg.wParam;
	}

	void WindowImpl::processMessages(const std::function<void(const Window::Message&)>& cb, const bool resetQueue)
	{
		std::lock_guard<std::mutex> lock(mEvents.lock);

		for (size_t i = 0; i < mEvents.queueSize; i++)
			cb(mEvents.queue[i]);

		if (resetQueue)
			mEvents.queueSize = 0;
	}

	void OpenglContextImpl::loadWGLFunctions(HMODULE openglModule)
	{
		typedef PROC(APIENTRY *PFNWGLGETPROCADDRESSPROC)(LPCSTR lpcstr);

		auto ptrWGlGetProcAddress = (PFNWGLGETPROCADDRESSPROC)GetProcAddress(openglModule, "wglGetProcAddress");
		if (ptrWGlGetProcAddress == nullptr)
			return;

	#if defined(WGL_ARB_create_context)
		mWGL.createContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)ptrWGlGetProcAddress("wglCreateContextAttribsARB");
	#endif

	#if defined(WGL_ARB_extensions_string)
		mWGL.getExtensionsStringARB = (PFNWGLGETEXTENSIONSSTRINGARBPROC)ptrWGlGetProcAddress("wglGetExtensionsStringARB");
	#endif

	#if defined(WGL_ARB_pixel_format)
		mWGL.getPixelFormatAttribivARB = (PFNWGLGETPIXELFORMATATTRIBIVARBPROC)ptrWGlGetProcAddress("wglGetPixelFormatAttribivARB");
		mWGL.getPixelFormatAttribfvARB = (PFNWGLGETPIXELFORMATATTRIBFVARBPROC)ptrWGlGetProcAddress("wglGetPixelFormatAttribfvARB");
		mWGL.choosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)ptrWGlGetProcAddress("wglChoosePixelFormatARB");
	#endif

	#if defined(WGL_EXT_swap_control)
		mWGL.swapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)ptrWGlGetProcAddress("wglSwapIntervalEXT");
		mWGL.getSwapIntervalEXT = (PFNWGLGETSWAPINTERVALEXTPROC)ptrWGlGetProcAddress("wglGetSwapIntervalEXT");
	#endif
	}

	bool OpenglContextImpl::auxWindowWGLExt(HINSTANCE hInstance, HMODULE openglModule)
	{
		auto TempAuxWindowName = L"gl aux window";

		{
			WNDCLASS winClassAux;

			memset(&winClassAux, 0, sizeof(WNDCLASS));
			winClassAux.hInstance = hInstance;
			winClassAux.lpszClassName = TempAuxWindowName;
			winClassAux.lpfnWndProc = auxWindowWGLExtProc;
			if (RegisterClass(&winClassAux) == 0)
				return false;
		}

		HWND hWndAux = CreateWindow(TempAuxWindowName, TempAuxWindowName, WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, 0, 0, 8, 8, HWND_DESKTOP, nullptr, hInstance, nullptr);
		if (hWndAux == nullptr)
		{
			UnregisterClass(TempAuxWindowName, hInstance);
			return false;
		}

		{
			PIXELFORMATDESCRIPTOR pfFormatD;
			memset(&pfFormatD, 0, sizeof(PIXELFORMATDESCRIPTOR));
			pfFormatD.nSize = sizeof(PIXELFORMATDESCRIPTOR);
			pfFormatD.nVersion = 1;
			pfFormatD.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
			pfFormatD.iPixelType = PFD_TYPE_RGBA;
			pfFormatD.cColorBits = 32;
			pfFormatD.cDepthBits = 24;
			pfFormatD.iLayerType = PFD_MAIN_PLANE;

			HDC hDCAux = GetDC(hWndAux);
			SetPixelFormat(hDCAux, ChoosePixelFormat(hDCAux, &pfFormatD), &pfFormatD);

			HGLRC hRCAux = mWGL.createContext(hDCAux);
			mWGL.makeCurrent(hDCAux, hRCAux);

			this->loadWGLFunctions(openglModule);

			mWGL.makeCurrent(nullptr, nullptr);
			mWGL.deleteContext(hRCAux);

			ReleaseDC(hWndAux, hDCAux);
		}

		DestroyWindow(hWndAux);
		UnregisterClass(TempAuxWindowName, hInstance);

		return true;
	}

	OpenglContextImpl::OpenglContextImpl(const WindowImpl &window, std::string_view openGLModuleName, int contextMajorVersion, int contextMinorVersion, bool contextDebug, bool contextForwardCompatible)
		: mWindow{ window }
	{
		HMODULE openglModule;

		if ((window.mHWnd == nullptr) || openGLModuleName.empty() || (contextMajorVersion < 3) || (contextMinorVersion < 0))
		{
			mErrorMsg = "incorrect data to properly create a OpenGL context";
			return;
		}

		{
			auto openGLModuleNameWChar = hr::StringUtils::conv2Native(openGLModuleName);

			openglModule = GetModuleHandle(openGLModuleNameWChar.c_str());
			if (openglModule == nullptr)
			{
				mErrorMsg = "incorrect OpenGL module name";
				return;
			}
		}

		mWGL.createContext = (HGLRC (APIENTRY *)(HDC hdc))GetProcAddress(openglModule, "wglCreateContext");
		mWGL.deleteContext = (BOOL (APIENTRY *)(HGLRC hglrc))GetProcAddress(openglModule, "wglDeleteContext");
		mWGL.makeCurrent = (BOOL (APIENTRY *)(HDC hdc, HGLRC hglrc))GetProcAddress(openglModule, "wglMakeCurrent");
		mWGL.swapBuffers = (BOOL (APIENTRY *)(HDC hdc))GetProcAddress(openglModule, "wglSwapBuffers");
		if (!mWGL.createContext || !mWGL.deleteContext || !mWGL.makeCurrent || !mWGL.swapBuffers)
			return;

		auxWindowWGLExt(window.mHModule, openglModule);

		if (!mWGL.choosePixelFormatARB || !mWGL.getExtensionsStringARB || !mWGL.createContextAttribsARB)
			return;

		mHDC = GetDC(window.mHWnd);
		if (mHDC == nullptr)
		{
			mErrorMsg = "unable to retrieve device context";
			return;
		}

		auto wglExt = mWGL.getExtensionsStringARB(mHDC);
		if (!wglExt || (std::strstr(wglExt, "WGL_ARB_create_context_profile") == nullptr))
		{
			mErrorMsg = "extension WGL_ARB_create_context_profile not supported";
			return;
		}
		if (!wglExt || (std::strstr(wglExt, "WGL_ARB_create_context_profile") == nullptr))
		{
			mErrorMsg = "extension WGL_ARB_create_context_profile not supported";
			return;
		}

		{
			int pixelFormat;
			UINT numFormats;

			float fAttributes[] = {0,0};
			int iAttributes[] = {
				WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
				WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
				WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
				WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
				WGL_COLOR_BITS_ARB, 24,
				WGL_ALPHA_BITS_ARB, 8,
				WGL_DEPTH_BITS_ARB, 0,
				WGL_STENCIL_BITS_ARB, 0,
				WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
				WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB, GL_TRUE,
				0, 0 };

			mUsedPFD = static_cast<unsigned int>(-1);
			if (mWGL.choosePixelFormatARB(mHDC, iAttributes, fAttributes, 1, &pixelFormat, &numFormats) == TRUE)
				mUsedPFD = pixelFormat;
		}

		if ((mUsedPFD == 0) || !SetPixelFormat(mHDC, mUsedPFD, nullptr))
		{
			mErrorMsg = "cannot find a useful pixel format";
			return;
		}

		{
			int contextAttrib[] = {
				WGL_CONTEXT_MAJOR_VERSION_ARB, contextMajorVersion,
				WGL_CONTEXT_MINOR_VERSION_ARB, contextMinorVersion,
				WGL_CONTEXT_FLAGS_ARB, (contextDebug ? WGL_CONTEXT_DEBUG_BIT_ARB : 0) | (contextForwardCompatible ? WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB : 0),
				WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
				0, 0 };

			mHRC = mWGL.createContextAttribsARB(mHDC, 0, contextAttrib);
			if (!mHRC)
			{
				mErrorMsg = "unable to create a rendering context";
				return;
			}
		}

		if (mWGL.makeCurrent(mHDC, mHRC) == FALSE)
		{
			mErrorMsg = "unable to activate a rendering context";
			return;
		}

		loadWGLFunctions(openglModule);
	}

	OpenglContextImpl::~OpenglContextImpl()
	{
		if (mHRC != nullptr)
		{
			if (mWGL.makeCurrent(mHDC, nullptr) == FALSE)
				mWindow.mLogger.logError(hr::engine::Logger::ModuleType::Graphics, "Unable to release rendering context.");

			if (mWGL.deleteContext(mHRC) == FALSE)
				mWindow.mLogger.logError(hr::engine::Logger::ModuleType::Graphics, "Unable to delete rendering context.");

			mHRC = nullptr;
		}

		if (mHDC != nullptr)
		{
			if (ReleaseDC(mWindow.mHWnd, mHDC) == 0)
				mWindow.mLogger.logError(hr::engine::Logger::ModuleType::Graphics, "Unable to release device context.");

			mHDC = nullptr;
		}
	}

	bool OpenglContextImpl::isValid() const
	{
		return mErrorMsg.empty();
	}

	std::string OpenglContextImpl::getErrorMsg() const
	{
		return mErrorMsg;
	}

	void OpenglContextImpl::setSwapInterval(const size_t &interval) const
	{
		if (mWGL.swapIntervalEXT)
			mWGL.swapIntervalEXT(interval);
	}

	bool OpenglContextImpl::swapBuffers() const
	{
		return ((mWGL.swapBuffers != nullptr) && (mWGL.swapBuffers(this->mHDC) != FALSE));
	}
}

#endif
