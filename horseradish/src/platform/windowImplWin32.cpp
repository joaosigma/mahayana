#include "windowImplWin32.hpp"

#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32)) && !defined(__CYGWIN__)

#include "common\stringUtils.hpp"
#include "common\opengl\openGL.hpp"

#include <Windowsx.h>

static
bool checkBestDisplayFrequency(DEVMODE * const deviceMode)
{
	DEVMODE deviceModeAux;
	DWORD bestModeIndex, bestModeFrequency;

	if (deviceMode == nullptr)
		return false;

	memset(&deviceModeAux, 0, sizeof(DEVMODE));
	deviceModeAux.dmSize = sizeof(DEVMODE);

	bestModeIndex = 0;
	bestModeFrequency = 0;

	for (DWORD modeIndex = 0; EnumDisplaySettingsEx(nullptr, modeIndex, &deviceModeAux, 0) != FALSE; modeIndex++)
	{
		if ((deviceModeAux.dmBitsPerPel != deviceMode->dmBitsPerPel) || (deviceModeAux.dmPelsHeight != deviceMode->dmPelsHeight) || (deviceModeAux.dmPelsWidth != deviceMode->dmPelsWidth))
			continue;

		if (deviceModeAux.dmDisplayFrequency < bestModeFrequency)
			continue;

		bestModeIndex = modeIndex;
		bestModeFrequency = deviceModeAux.dmDisplayFrequency;
	}

	if (bestModeFrequency == 0)
		return false;

	EnumDisplaySettingsEx(nullptr, bestModeIndex, deviceMode, 0);
	return true;
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

		SetWindowLongPtr(hWnd, GWL_USERDATA, (LONG_PTR)pCreate->lpCreateParams);
		return DefWindowProc(hWnd, messageID, wParam, lParam);
	}

	WindowImpl* window = reinterpret_cast<WindowImpl*>(GetWindowLongPtr(hWnd, GWL_USERDATA));
	if (window == nullptr)
		return DefWindowProc(hWnd, messageID, wParam, lParam);

	if (messageID == WM_CLOSE)
	{
		window->closeRequested = true;
		return 0;
	}

	if (messageID == WM_DESTROY)
	{
		PostQuitMessage(0);
		return 0;
	}

	if (messageID == WM_INPUT)
	{
		BYTE bufferAux[40];

		auto bufferSize = sizeof(bufferAux);
		GetRawInputData((HRAWINPUT)lParam, RID_INPUT, bufferAux, &bufferSize, sizeof(RAWINPUTHEADER));

		window->processRawInput(*(reinterpret_cast<RAWINPUT*>(bufferAux)));
		return 0;
	}
	
	if ((messageID == WM_CHAR) || (messageID == WM_KEYDOWN) || (messageID == WM_MOUSEWHEEL))
	{
		std::unique_lock<std::mutex> lock(window->mEvents.lock);

		if (window->mEvents.queueSize >= window->mEvents.queue.size())
		{
			//TODO: create warning
			return DefWindowProc(hWnd, messageID, wParam, lParam);
		}

		if ((messageID == WM_CHAR) || (messageID == WM_KEYDOWN))
		{
			HorseRadish::hSplitUInt32 flags(0);

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
			HorseRadish::hSplitUInt32 flags(0), params(0);

			flags.piecesShort.short0 = static_cast<signed short>(GET_WHEEL_DELTA_WPARAM(wParam) / 120);
			flags.piecesShort.short1 |= ((GET_KEYSTATE_WPARAM(wParam) & MK_CONTROL) != 0) ? static_cast<unsigned int>(Window::Message::MessageFlags::ControlKey) : 0;
			flags.piecesShort.short1 |= ((GET_KEYSTATE_WPARAM(wParam) & MK_SHIFT) != 0) ? static_cast<unsigned int>(Window::Message::MessageFlags::ShiftKey) : 0;

			params.piecesShort.short0 = GET_X_LPARAM(lParam);
			params.piecesShort.short1 = GET_Y_LPARAM(lParam);
			window->mEvents.queue[window->mEvents.queueSize++] = Window::Message(Window::Message::MessageType::MouseWheel, params.valueWord, flags);
		}

		return 0;
	}

	return DefWindowProc(hWnd, messageID, wParam, lParam);
}

HorseRadish::hInt32 WindowImpl::translateVirtualKeyCode(LPARAM nativeKeyCode)
{
	Window::VirtualKeys virtuakKey;

	if ((nativeKeyCode >= 0x30) && (nativeKeyCode <= 0x39))
		return static_cast<HorseRadish::hInt32>(nativeKeyCode); //0-9
	if ((nativeKeyCode >= 0x41) && (nativeKeyCode <= 0x5A))
		return static_cast<HorseRadish::hInt32>(nativeKeyCode); //A-Z

	if ((nativeKeyCode >= 0x70) && (nativeKeyCode <= 0x7B))
		return (static_cast<HorseRadish::hInt32>(Window::VirtualKeys::F1) + (nativeKeyCode - 0x70)); //F1-F12

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

	return static_cast<HorseRadish::hInt32>(virtuakKey);
}

void WindowImpl::processRawInput(const RAWINPUT &inputData)
{
	std::lock_guard<std::mutex> lock(mRawInput.lock);

	if (inputData.header.dwType == RIM_TYPEMOUSE)
	{
		mRawInput.mouseAccum.x += inputData.data.mouse.lLastX;
		mRawInput.mouseAccum.y += inputData.data.mouse.lLastY;
	}

	if (inputData.header.dwType == RIM_TYPEKEYBOARD)
	{
		bool keyDown;
		if (!(inputData.data.keyboard.Flags & RI_KEY_MAKE))
			keyDown = true;
		if (inputData.data.keyboard.Flags & RI_KEY_BREAK)
			keyDown = false;

		auto virtualKeyCode = WindowImpl::translateVirtualKeyCode(inputData.data.keyboard.VKey);
		if (virtualKeyCode < mRawInput.keysRealtime.size())
			mRawInput.keysRealtime[virtualKeyCode] = keyDown;
	}
}

WindowImpl::WindowImpl(HorseRadish::Engine::Logger &logger)
	: hWnd(nullptr)
	, hModule(nullptr)
	, mLogger(logger)
	, isFullscreen(false)
	, isInitialized(false)
{
	mEvents.queueSize = 0;

	mRawInput.mouseAccum = mRawInput.mouseSnapshot = HorseRadish::Vector(0.0f);
	mRawInput.keysSnapshot.fill(false);
	mRawInput.keysRealtime.fill(false);

	memset(&this->originalDeviceMode, 0, sizeof(this->originalDeviceMode));
	mClassName.clear();
}

WindowImpl::~WindowImpl()
{
}

std::string WindowImpl::GetErrorMsg() const
{
	return errorMsg;
}

bool WindowImpl::WindowInit(const std::string& windowTitle, const unsigned int winWidth, const unsigned int winHeight, const bool winFullscreen)
{
	DWORD dwExStyle, dwStyle;
	RECT windowRect, desktopRect;

	if (this->isInitialized)
	{
		errorMsg = "window already initialized";
		return false;
	}

	if (windowTitle.empty() || (winWidth == 0) || (winHeight == 0))
	{
		errorMsg = "incorrect data to properly create a window";
		return false;
	}

	windowRect.left = windowRect.top = 0;
	windowRect.right = winWidth;
	windowRect.bottom = winHeight;

	mClassName = HorseRadish::StringUtils::conv2UTF16("HorseRadish graphics engine...");

	this->hModule = GetModuleHandle(NULL); //safe since this is not a DLL

	{
		WNDCLASSEXW windowClass;

		memset(&windowClass, 0, sizeof(WNDCLASSEXW));
		windowClass.cbSize = sizeof(WNDCLASSEXW);
		windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
		windowClass.lpfnWndProc = (WNDPROC)&WindowImpl::wndProc;
		windowClass.cbClsExtra = 0;
		windowClass.cbWndExtra = 0;
		windowClass.hInstance = this->hModule;
		windowClass.hIcon = nullptr;
		windowClass.hCursor = nullptr;
		windowClass.hbrBackground = nullptr;
		windowClass.lpszMenuName = nullptr;
		windowClass.lpszClassName = mClassName.c_str();
		if (RegisterClassEx(&windowClass) == 0)
		{
			errorMsg = "unable to register window class";
			return false;
		}
	}

	this->originalDeviceMode.dmSize = sizeof(DEVMODE);
	this->originalDeviceMode.dmDriverExtra = 0;
	EnumDisplaySettingsEx(nullptr, ENUM_CURRENT_SETTINGS, &this->originalDeviceMode, 0);

	if (winFullscreen)
	{
		DEVMODE	dmScreenSettings;

		memset(&dmScreenSettings, 0, sizeof(DEVMODE));
		dmScreenSettings.dmSize = sizeof(DEVMODE);
		dmScreenSettings.dmPelsWidth = winWidth;
		dmScreenSettings.dmPelsHeight = winHeight;
		dmScreenSettings.dmBitsPerPel = 32;
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		if ((checkBestDisplayFrequency(&dmScreenSettings) == false) || (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN | CDS_RESET) != DISP_CHANGE_SUCCESSFUL))
		{
			mLogger.AddWarning(HorseRadish::Engine::Logger::ModuleType::Graphics, "Unable to change to fullscreen");

			dwExStyle = WS_EX_APPWINDOW;
			dwStyle = WS_CAPTION | WS_VISIBLE;
			AdjustWindowRectEx(&windowRect, dwStyle, false, dwExStyle);
		}
		else
		{
			dwExStyle = WS_EX_APPWINDOW;
			dwStyle = WS_POPUP | WS_VISIBLE;

			this->isFullscreen = true;
		}
	}
	else
	{
		dwExStyle = WS_EX_APPWINDOW;
		dwStyle = WS_CAPTION | WS_VISIBLE;
		AdjustWindowRectEx(&windowRect, dwStyle, false, dwExStyle);
	}

	if (SystemParametersInfo(SPI_GETWORKAREA, 0, &desktopRect, 0) != TRUE)
		desktopRect.bottom = desktopRect.left = desktopRect.right = desktopRect.top = 0;

	{
		auto windowTitleWChar = HorseRadish::StringUtils::conv2UTF16(windowTitle);

		this->hWnd = CreateWindowEx(dwExStyle, mClassName.c_str(), windowTitleWChar.c_str(), dwStyle,
			desktopRect.left + 5, desktopRect.top + 5, (windowRect.right - windowRect.left), (windowRect.bottom - windowRect.top),
			HWND_DESKTOP, nullptr, this->hModule,
			this);

		if (this->hWnd == nullptr)
		{
			errorMsg = "unable to create window";

			UnregisterClass(mClassName.c_str(), this->hModule);
			return false;
		}
	}

	{
		RAWINPUTDEVICE rawInputDevice[2];

		rawInputDevice[0].usUsagePage = 0x01; //HID_USAGE_PAGE_GENERIC
		rawInputDevice[0].usUsage = 0x02; //HID_USAGE_GENERIC_MOUSE
		rawInputDevice[0].dwFlags = 0;
		rawInputDevice[0].hwndTarget = hWnd;
		
		rawInputDevice[1].usUsagePage = 0x01; //HID_USAGE_PAGE_GENERIC
		rawInputDevice[1].usUsage = 0x06; //HID_USAGE_GENERIC_KEYBOARD
		rawInputDevice[1].dwFlags = 0;
		rawInputDevice[1].hwndTarget = hWnd;

		if (RegisterRawInputDevices(rawInputDevice, 2, sizeof(rawInputDevice[0])) != TRUE)
			mLogger.AddError(HorseRadish::Engine::Logger::ModuleType::Graphics, "unable to register raw input devices");
	}

	SetCursor(nullptr);

	this->isInitialized = true;
	return true;
}

bool WindowImpl::WindowEditorInit(const std::string& windowTitle, const unsigned int winWidth, const unsigned int winHeight, const HWND handleWindowParent)
{
	return false;
}

bool WindowImpl::SetWindowAlpha(const unsigned char &valorAlpha) const
{
	if (this->isFullscreen)
		return false;

	SetWindowLong(this->hWnd, GWL_EXSTYLE, GetWindowLong(this->hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);

	return (SetLayeredWindowAttributes(this->hWnd, RGB(0,0,0), (valorAlpha < 10) ? 10 : valorAlpha, LWA_ALPHA) == TRUE);
}

bool WindowImpl::SendMessageClose() const
{
	return (SendNotifyMessage(this->hWnd, WM_CLOSE, 0, 0) == TRUE);
}

bool WindowImpl::SetFocus() const
{
	return (::SetFocus(this->hWnd) != nullptr);
}

void WindowImpl::RawInputSnapshot()
{
	std::lock_guard<std::mutex> lock(mRawInput.lock);

	mRawInput.mouseSnapshot = mRawInput.mouseAccum;
	mRawInput.mouseAccum.Set(0.0f);

	mRawInput.keysSnapshot = mRawInput.keysRealtime;
}

bool WindowImpl::RawInputGetKeyStatus(const unsigned int &vcode)
{
	std::lock_guard<std::mutex> lock(mRawInput.lock);

	if (vcode >= mRawInput.keysSnapshot.size())
		return false;
	return mRawInput.keysSnapshot[vcode];
}

bool WindowImpl::RawInputGetKeyStatus(const Window::VirtualKeys &vcode)
{
	return WindowImpl::RawInputGetKeyStatus(static_cast<unsigned int>(vcode));
}

HorseRadish::Vector WindowImpl::RawInputGetMouseStatus()
{
	std::lock_guard<std::mutex> lock(mRawInput.lock);

	return mRawInput.mouseSnapshot;
}

int WindowImpl::MessageLoop(std::function<void()> closingCb)
{
	MSG msg;
	BOOL returnCode;

	if (!this->isInitialized)
		return -1;

	//main loop
	closeRequested = false;
	while ((returnCode = GetMessage(&msg, NULL, 0, 0)) != 0)
	{
		if (returnCode == -1)
		{
			mLogger.AddError(HorseRadish::Engine::Logger::ModuleType::Graphics, "Error in window message loop");
			continue;
		}

		if (closeRequested)
		{
			if (closingCb)
				closingCb();

			if (this->isFullscreen)
				ChangeDisplaySettings(&this->originalDeviceMode, CDS_RESET | CDS_UPDATEREGISTRY);
			this->isFullscreen = false;

			if (this->hWnd != nullptr)
			{
				if (DestroyWindow(this->hWnd) == FALSE)
					mLogger.AddError(HorseRadish::Engine::Logger::ModuleType::Graphics, "Unable to delete window handle");

				this->hWnd = nullptr;
			}

			closeRequested = false;
		}

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	//cleanup
	if (UnregisterClass(mClassName.c_str(), this->hModule) == FALSE)
		mLogger.AddError(HorseRadish::Engine::Logger::ModuleType::Graphics, "Unable to unregister window class");

	this->isInitialized = false;

	//return exit code
	return msg.wParam;
}

void WindowImpl::ProcessMessages(std::function<void(const Window::Message&)> cb, const bool resetQueue)
{
	std::unique_lock<std::mutex> lock(mEvents.lock);

	for (int i = 0; i < mEvents.queueSize; i++)
		cb(mEvents.queue[i]);

	if (resetQueue)
		mEvents.queueSize = 0;
}

void WindowImpl::MsgBoxInfo(const std::string& msg)
{
	auto msgWChar = HorseRadish::StringUtils::conv2UTF16(msg);

	MessageBox(nullptr, msgWChar.c_str(), L"Info", MB_OK | MB_ICONINFORMATION);
}

void WindowImpl::MsgBoxInfo(const char * const msg)
{
	auto msgWChar = HorseRadish::StringUtils::conv2UTF16(msg);

	MessageBox(nullptr, msgWChar.c_str(), L"Info", MB_OK | MB_ICONINFORMATION);
}

void WindowImpl::MsgBoxWarn(const std::string& msg)
{
	auto msgWChar = HorseRadish::StringUtils::conv2UTF16(msg);

	MessageBox(nullptr, msgWChar.c_str(), L"Warning", MB_OK | MB_ICONWARNING);
}

void WindowImpl::MsgBoxWarn(const char * const msg)
{
	auto msgWChar = HorseRadish::StringUtils::conv2UTF16(msg);

	MessageBox(nullptr, msgWChar.c_str(), L"Warning", MB_OK | MB_ICONWARNING);
}

void WindowImpl::MsgBoxError(const std::string& msg)
{
	auto msgWChar = HorseRadish::StringUtils::conv2UTF16(msg);

	MessageBox(nullptr, msgWChar.c_str(), L"Error", MB_OK | MB_ICONERROR);
}

void WindowImpl::MsgBoxError(const char * const msg)
{
	auto msgWChar = HorseRadish::StringUtils::conv2UTF16(msg);
	
	MessageBox(nullptr, msgWChar.c_str(), L"Error", MB_OK | MB_ICONERROR);
}

void OpenglContextImpl::loadWGLFunctions(HMODULE openglModule)
{
	typedef PROC(APIENTRY *PFNWGLGETPROCADDRESSPROC)(LPCSTR lpcstr);

	auto ptrWGlGetProcAddress = (PFNWGLGETPROCADDRESSPROC)GetProcAddress(openglModule, "wglGetProcAddress");
	if (ptrWGlGetProcAddress == nullptr)
		return;

#ifdef WGL_ARB_create_context
	this->wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)ptrWGlGetProcAddress("wglCreateContextAttribsARB");
#endif

#ifdef WGL_ARB_extensions_string
	this->wglGetExtensionsStringARB = (PFNWGLGETEXTENSIONSSTRINGARBPROC)ptrWGlGetProcAddress("wglGetExtensionsStringARB");
#endif

#ifdef WGL_ARB_pixel_format
	this->wglGetPixelFormatAttribivARB = (PFNWGLGETPIXELFORMATATTRIBIVARBPROC)ptrWGlGetProcAddress("wglGetPixelFormatAttribivARB");
	this->wglGetPixelFormatAttribfvARB = (PFNWGLGETPIXELFORMATATTRIBFVARBPROC)ptrWGlGetProcAddress("wglGetPixelFormatAttribfvARB");
	this->wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)ptrWGlGetProcAddress("wglChoosePixelFormatARB");
#endif

#ifdef WGL_EXT_swap_control
	this->wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)ptrWGlGetProcAddress("wglSwapIntervalEXT");
	this->wglGetSwapIntervalEXT = (PFNWGLGETSWAPINTERVALEXTPROC)ptrWGlGetProcAddress("wglGetSwapIntervalEXT");
#endif
}

bool OpenglContextImpl::auxWindowWGLExt(HINSTANCE hInstance, HMODULE openglModule)
{
	HWND hWndAux;
	HDC hDCAux;
	HGLRC hRCAux;
	PIXELFORMATDESCRIPTOR pfFormatD;

	{
		WNDCLASS winClassAux;

		memset(&winClassAux, 0, sizeof(WNDCLASS));
		winClassAux.hInstance = hInstance;
		winClassAux.lpszClassName = L"gl aux window";
		winClassAux.lpfnWndProc = auxWindowWGLExtProc;
		if (RegisterClass(&winClassAux) == 0)
			return false;
	}

	hWndAux = CreateWindow(L"gl aux window", L"gl aux window", WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, 0, 0, 8, 8, HWND_DESKTOP, nullptr, hInstance, nullptr);
	if (hWndAux == nullptr)
	{
		UnregisterClass(L"gl aux window", hInstance);
		return false;
	}

	memset(&pfFormatD, 0, sizeof(PIXELFORMATDESCRIPTOR));
	pfFormatD.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfFormatD.nVersion = 1;
	pfFormatD.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfFormatD.iPixelType = PFD_TYPE_RGBA;
	pfFormatD.cColorBits = 32;
	pfFormatD.cDepthBits = 24;
	pfFormatD.iLayerType = PFD_MAIN_PLANE;
	
	hDCAux = GetDC(hWndAux);
	SetPixelFormat(hDCAux, ChoosePixelFormat(hDCAux, &pfFormatD), &pfFormatD);
	hRCAux = this->wglCreateContext(hDCAux);
	this->wglMakeCurrent(hDCAux, hRCAux);

	this->loadWGLFunctions(openglModule);

	this->wglMakeCurrent(nullptr, nullptr);
	this->wglDeleteContext(hRCAux);
	ReleaseDC(hWndAux, hDCAux);
	DestroyWindow(hWndAux);
	UnregisterClass(L"gl aux window", hInstance);
	return true;
}

OpenglContextImpl::OpenglContextImpl(const WindowImpl &window, const std::string& openGLModuleName, int contextMajorVersion, int contextMinorVersion, bool contextDebug, bool contextForwardCompatible)
	: window(window)
{
	HMODULE openglModule;

	if ( (this->window.hWnd == nullptr) || openGLModuleName.empty() || (contextMajorVersion < 3) || (contextMinorVersion < 0))
	{
		errorMsg = "incorrect data to properly create a OpenGL context";
		return;
	}

	{
		auto openGLModuleNameWChar = HorseRadish::StringUtils::conv2UTF16(openGLModuleName);

		openglModule = GetModuleHandle(openGLModuleNameWChar.c_str());
		if (openglModule == nullptr)
		{
			errorMsg = "incorrect OpenGL module name";
			return;
		}
	}

	this->wglCreateContext = (HGLRC (APIENTRY *)(HDC hdc))GetProcAddress(openglModule, "wglCreateContext");
	this->wglDeleteContext = (BOOL (APIENTRY *)(HGLRC hglrc))GetProcAddress(openglModule, "wglDeleteContext");
	this->wglMakeCurrent = (BOOL (APIENTRY *)(HDC hdc, HGLRC hglrc))GetProcAddress(openglModule, "wglMakeCurrent");
	this->wglSwapBuffers = (BOOL (APIENTRY *)(HDC hdc))GetProcAddress(openglModule, "wglSwapBuffers");
	if ((this->wglCreateContext == nullptr) || (this->wglDeleteContext == nullptr) || (this->wglMakeCurrent == nullptr) || (this->wglSwapBuffers == nullptr))
		return;

	auxWindowWGLExt(this->window.hModule, openglModule);

	if ((this->wglChoosePixelFormatARB == nullptr) || (this->wglGetExtensionsStringARB == nullptr) || (this->wglCreateContextAttribsARB == nullptr))
		return;

	this->hDC = GetDC(this->window.hWnd);
	if (this->hDC == nullptr)
	{
		errorMsg = "unable to retrieve device context";
		return;
	}

	auto wglExt = this->wglGetExtensionsStringARB(this->hDC);
	if ((wglExt == nullptr) || (strstr(wglExt, "WGL_ARB_create_context_profile") == nullptr))
	{
		errorMsg = "extension WGL_ARB_create_context_profile not supported";
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
			0, 0 };

		this->usedPFD = -1;
		if (this->wglChoosePixelFormatARB(this->hDC, iAttributes, fAttributes, 1, &pixelFormat, &numFormats) == TRUE)
			this->usedPFD = pixelFormat;
	}

	if ((this->usedPFD == 0) || (SetPixelFormat(this->hDC,this->usedPFD,nullptr) == false))
	{
		errorMsg = "cannot find a useful pixel format";
		return;
	}

	{
		int contextAttrib[] = {
			WGL_CONTEXT_MAJOR_VERSION_ARB, contextMajorVersion,
			WGL_CONTEXT_MINOR_VERSION_ARB, contextMinorVersion,
			WGL_CONTEXT_FLAGS_ARB, (contextDebug ? WGL_CONTEXT_DEBUG_BIT_ARB : 0) | (contextForwardCompatible ? WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB : 0),
			WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
			0, 0 };

		this->hRC = this->wglCreateContextAttribsARB(this->hDC, 0, contextAttrib);
		if (this->hRC == nullptr)
		{
			errorMsg = "unable to create a rendering context";
			return;
		}
	}

	if (this->wglMakeCurrent(this->hDC, this->hRC) == FALSE)
	{
		errorMsg = "unable to activate a rendering context";
		return;
	}

	this->loadWGLFunctions(openglModule);
}

OpenglContextImpl::~OpenglContextImpl()
{
	if (this->hRC != nullptr)
	{
		if (this->wglMakeCurrent(this->hDC, nullptr) == FALSE)
			this->window.mLogger.AddError(HorseRadish::Engine::Logger::ModuleType::Graphics, "Unable to release rendering context.");

		if (this->wglDeleteContext(this->hRC) == FALSE)
			this->window.mLogger.AddError(HorseRadish::Engine::Logger::ModuleType::Graphics, "Unable to delete rendering context.");

		this->hRC = nullptr;
	}

	if (this->hDC != nullptr)
	{
		if (ReleaseDC(this->window.hWnd, this->hDC) == 0)
			this->window.mLogger.AddError(HorseRadish::Engine::Logger::ModuleType::Graphics, "Unable to release device context.");

		this->hDC = nullptr;
	}
}

bool OpenglContextImpl::IsValid() const
{
	return errorMsg.empty();
}

std::string OpenglContextImpl::GetErrorMsg() const
{
	return errorMsg;
}

void OpenglContextImpl::SetSwapInterval(const unsigned int &interval) const
{
	if (this->wglSwapIntervalEXT != nullptr)
		this->wglSwapIntervalEXT(interval);
}

bool OpenglContextImpl::SwapBuffers() const
{
	return ((this->wglSwapBuffers != nullptr) && (this->wglSwapBuffers(this->hDC) != FALSE));
}

#endif
