#include "winApp.hpp"

#include "common\Common.hpp"
#include "common\UTF.hpp"
#include "common\opengl\openGL.hpp"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

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

Window::Window(HINSTANCE hInst)
	: hWnd(nullptr)
	, hInstance(hInst)
	, isFullscreen(false)
	, isInitialized(false)
	, winWidth(0)
	, winHeight(0)
{
	memset(&this->originalDeviceMode, 0, sizeof(this->originalDeviceMode));
	memset(this->className, 0, sizeof(this->className));
}

Window::~Window()
{
	if (this->isInitialized)
		this->WindowKill();
}

bool Window::WindowInit(WNDPROC procFunc, const HorseRadish::hChar *windowTitle, const unsigned int winWidth, const unsigned int winHeight, const bool winFullscreen)
{
	DWORD dwExStyle, dwStyle;
	RECT windowRect, desktopRect;

	if (this->isInitialized == true)
	{
		Window::MsgBoxError("Window already initialized.\nApplication cannot proceed.");
		return false;
	}

	if ((windowTitle == nullptr) || (*windowTitle == '\0') || (winWidth == 0) || (winHeight == 0))
	{
		Window::MsgBoxError("Incorrect data: cannot create window.\nApplication cannot proceed.");
		return false;
	}

	this->winWidth = winWidth;
	this->winHeight = winHeight;

	windowRect.left = windowRect.top = 0;
	windowRect.right = winWidth;
	windowRect.bottom = winHeight;

	HorseRadish::UTF::ConvertUTF8To("HorseRadish graphics engine...", HorseRadish::UTF::Windows, this->className, sizeof(this->className));

	{
		WNDCLASSEXW windowClass;

		memset(&windowClass, 0, sizeof(WNDCLASSEXW));
		windowClass.cbSize = sizeof(WNDCLASSEXW);
		windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
		windowClass.lpfnWndProc = (WNDPROC)procFunc;
		windowClass.cbClsExtra = 0;
		windowClass.cbWndExtra = 0;
		windowClass.hInstance = this->hInstance;
		windowClass.hIcon = nullptr;
		windowClass.hCursor = nullptr;
		windowClass.hbrBackground = nullptr;
		windowClass.lpszMenuName = nullptr;
		windowClass.lpszClassName = this->className;
		if (RegisterClassEx(&windowClass) == 0)
		{
			Window::MsgBoxError("Unable to register window class.\nApplication cannot proceed.");
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
			Window::MsgBoxWarn("Unable to change to fullscreen.\nApplication will continue in window mode.");

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
		wchar_t windowTitleWChar[256];
		HorseRadish::UTF::ConvertUTF8To(windowTitle, HorseRadish::UTF::Windows, windowTitleWChar, sizeof(windowTitleWChar));

		this->hWnd = CreateWindowEx(dwExStyle, this->className, windowTitleWChar, dwStyle, desktopRect.left + 5, desktopRect.top + 5, (windowRect.right - windowRect.left), (windowRect.bottom - windowRect.top), HWND_DESKTOP, nullptr, this->hInstance, nullptr);
		if (this->hWnd == nullptr)
		{
			Window::MsgBoxError("Unable to create window!\nApplication cannot proceed.");

			UnregisterClass(this->className, this->hInstance);
			return false;
		}
	}

	SetCursor(nullptr);

	this->isInitialized = true;
	return true;
}

bool Window::WindowEditorInit(WNDPROC procFunc, const HorseRadish::hChar *windowTitle, const unsigned int winWidth, const unsigned int winHeight, const HWND handleWindowParent)
{
	return false;
}

void Window::WindowKill()
{
	if (!this->isInitialized)
		return;

	if (this->isFullscreen)
		ChangeDisplaySettings(&this->originalDeviceMode, CDS_RESET | CDS_UPDATEREGISTRY);
	this->isFullscreen = false;

	if (this->hWnd != nullptr)
	{
		if (DestroyWindow(this->hWnd) == FALSE)
			Window::MsgBoxError("Unable to delete window handle.");

		this->hWnd = nullptr;
	}

	if (UnregisterClass(this->className, this->hInstance) == FALSE)
		Window::MsgBoxError("Unable to unregister window class.");

	this->isInitialized = false;
}

bool Window::SetWindowAlpha(const unsigned char &valorAlpha) const
{
	if (this->isFullscreen)
		return false;

	SetWindowLong(this->hWnd, GWL_EXSTYLE, GetWindowLong(this->hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);

	return (SetLayeredWindowAttributes(this->hWnd, RGB(0,0,0), (valorAlpha < 10) ? 10 : valorAlpha, LWA_ALPHA) == TRUE);
}

bool Window::SendMessageClose() const
{
	return (SendNotifyMessage(this->hWnd, WM_CLOSE, 0, 0) == TRUE);
}

bool Window::SetFocus() const
{
	return (::SetFocus(this->hWnd) != nullptr);
}

void Window::PeekMessageAndDispatch() const
{
	MSG msg;

	while (PeekMessage(&msg, this->hWnd, 0, 0, PM_REMOVE) == TRUE)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

void Window::MsgBoxInfo(const HorseRadish::String &msg)
{
	wchar_t msgConverted[512];

	msg.Convert(HorseRadish::String::Windows, msgConverted, sizeof(msgConverted));
	MessageBox(nullptr, msgConverted, L"Info", MB_OK | MB_ICONINFORMATION);
}

void Window::MsgBoxInfo(const char * const msg)
{
	HorseRadish::String msgConvertida;

	msgConvertida.Set(HorseRadish::String::UTF8, msg);
	Window::MsgBoxInfo(msgConvertida);
}

void Window::MsgBoxWarn(const HorseRadish::String &msg)
{
	wchar_t msgConverted[512];

	msg.Convert(HorseRadish::String::Windows, msgConverted, sizeof(msgConverted));
	MessageBox(nullptr, msgConverted, L"Warning", MB_OK | MB_ICONWARNING);
}

void Window::MsgBoxWarn(const char * const msg)
{
	HorseRadish::String msgConverted;

	msgConverted.Set(HorseRadish::String::UTF8, msg);
	Window::MsgBoxWarn(msgConverted);
}

void Window::MsgBoxError(const HorseRadish::String &msg)
{
	wchar_t msgConverted[512];

	msg.Convert(HorseRadish::String::Windows, msgConverted, sizeof(msgConverted));
	MessageBox(nullptr, msgConverted, L"Error", MB_OK | MB_ICONERROR);
}

void Window::MsgBoxError(const char * const msg)
{
	HorseRadish::String msgConverted;

	msgConverted.Set(HorseRadish::String::UTF8, msg);
	Window::MsgBoxError(msgConverted);
}

bool Window::CommandLineHasParam(PWSTR cmdLine, const HorseRadish::hChar * const parameterName)
{
	HorseRadish::String commandLine, curToken;

	if ((cmdLine == nullptr) || (parameterName == nullptr))
		return false;

	commandLine.Set(HorseRadish::String::Windows, cmdLine);

	for(HorseRadish::String::Tokenizer tok(commandLine, ' '); tok.IsLast() == false; )
	{
		tok.Read(curToken);
		if (curToken.GetSizeBytes() == 0)
			break;

		curToken.Trim();

		if (curToken == parameterName)
			return true;
	}

	return false;
}

bool Window::CommandLineGetParam(PWSTR cmdLine, const HorseRadish::hChar * const parameterName, HorseRadish::String &parameterValue)
{
	HorseRadish::String commandLine, curToken;

	parameterValue.SetEmpty();

	if ((cmdLine == nullptr) || (parameterName == nullptr))
		return false;

	commandLine.Set(HorseRadish::String::Windows, cmdLine);

	for(HorseRadish::String::Tokenizer tok(commandLine, ' '); tok.IsLast() == false; )
	{
		tok.Read(curToken);
		if (curToken.GetSizeBytes() == 0)
			break;

		curToken.Trim();

		if (curToken != parameterName)
			continue;

		if (tok.IsLast() == true)
			return false;

		tok.Read(parameterValue);
		return true;
	}

	return false;
}

bool Window::CommandLineGetParam(PWSTR cmdLine, const HorseRadish::hChar * const parameterName, int &parameterValue)
{
	HorseRadish::String paramValueString;

	if (Window::CommandLineGetParam(cmdLine, parameterName, paramValueString) == false)
		return false;

	parameterValue = paramValueString.ToInt();
	return true;
}

void OpenglContext::loadWGLFunctions(HMODULE openglModule)
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

bool OpenglContext::auxWindowWGLExt(HINSTANCE hInstance, HMODULE openglModule)
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

OpenglContext::OpenglContext(const Window * const window, const HorseRadish::hChar *openGLModuleName, int contextMajorVersion, int contextMinorVersion, bool contextDebug, bool contextForwardCompatible)
{
	HMODULE openglModule;

	this->window = window;

	if ( (this->window == nullptr) || (this->window->hWnd == nullptr) || (openGLModuleName == nullptr) || (*openGLModuleName == '\0') || (contextMajorVersion < 3) || (contextMinorVersion < 0))
	{
		Window::MsgBoxError("Incorrect data! Cannot create window.\nApplication cannot proceed.");
		return;
	}

	{
		wchar_t openGLModuleNameWChar[128];
		HorseRadish::UTF::ConvertUTF8To(openGLModuleName, HorseRadish::UTF::Windows, openGLModuleNameWChar, sizeof(openGLModuleNameWChar));

		openglModule = GetModuleHandle(openGLModuleNameWChar);
		if (openglModule == nullptr)
		{
			Window::MsgBoxError("Incorrect OpenGL module name.\nApplication cannot proceed.");
			return;
		}
	}

	this->wglCreateContext = (HGLRC (APIENTRY *)(HDC hdc))GetProcAddress(openglModule, "wglCreateContext");
	this->wglDeleteContext = (BOOL (APIENTRY *)(HGLRC hglrc))GetProcAddress(openglModule, "wglDeleteContext");
	this->wglMakeCurrent = (BOOL (APIENTRY *)(HDC hdc, HGLRC hglrc))GetProcAddress(openglModule, "wglMakeCurrent");
	this->wglSwapBuffers = (BOOL (APIENTRY *)(HDC hdc))GetProcAddress(openglModule, "wglSwapBuffers");
	if ((this->wglCreateContext == nullptr) || (this->wglDeleteContext == nullptr) || (this->wglMakeCurrent == nullptr) || (this->wglSwapBuffers == nullptr))
		return;

	auxWindowWGLExt(this->window->hInstance, openglModule);

	if ((this->wglChoosePixelFormatARB == nullptr) || (this->wglGetExtensionsStringARB == nullptr) || (this->wglCreateContextAttribsARB == nullptr))
		return;

	this->hDC = GetDC(this->window->hWnd);
	if (this->hDC == nullptr)
	{
		Window::MsgBoxError("Unable to device context!\nApplication cannot proceed.");
		return;
	}

	auto wglExt = this->wglGetExtensionsStringARB(this->hDC);
	if ((wglExt == nullptr) || (strstr(wglExt, "WGL_ARB_create_context_profile") == nullptr))
	{
		Window::MsgBoxError("Extensions WGL_ARB_create_context_profile not supported!\nApplication cannot proceed.");
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
		Window::MsgBoxError("Cannot find a useful pixel format!\nApplication cannot proceed.");
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
			Window::MsgBoxError("Unable to create a rendering context!\nApplication cannot proceed.");
			return;
		}
	}

	if (this->wglMakeCurrent(this->hDC, this->hRC) == FALSE)
	{
		Window::MsgBoxError("Unable to activate a rendering context!\nApplication cannot proceed.");
		return;
	}

	this->contextCreated = true;

	this->loadWGLFunctions(openglModule);
}

OpenglContext::~OpenglContext()
{
	if (this->hRC != nullptr)
	{
		if (this->wglMakeCurrent(this->hDC, nullptr) == FALSE)
			Window::MsgBoxError("Unable to release rendering context.");

		if (this->wglDeleteContext(this->hRC) == FALSE)
			Window::MsgBoxError("Unable to delete rendering context.");

		this->hRC = nullptr;
	}

	if (this->hDC != nullptr)
	{
		if (ReleaseDC(this->window->hWnd, this->hDC) == 0)
			Window::MsgBoxError("Unable to release device context.");

		this->hDC = nullptr;
	}
}

void OpenglContext::SetSwapInterval(const unsigned int &interval) const
{
	if (this->wglSwapIntervalEXT != nullptr)
		this->wglSwapIntervalEXT(interval);
}

bool OpenglContext::SwapBuffers() const
{
	return ((this->wglSwapBuffers != nullptr) && (this->wglSwapBuffers(this->hDC) != FALSE));
}

RawInput::RawInput(const Window * const window)
	: numMaxKeyStrokes(256)
{
	RAWINPUTDEVICE rawInputDevice[1];

	memset(mouseSnapshot, 0, sizeof(mouseSnapshot));
	memset(mousePosAccum, 0, sizeof(mousePosAccum));

	keysSnapshot = new bool[numMaxKeyStrokes];
	keysRealtime = new bool[numMaxKeyStrokes];

	memset(keysSnapshot, 0, sizeof(bool) * numMaxKeyStrokes);
	memset(keysRealtime, 0, sizeof(bool) * numMaxKeyStrokes);

    rawInputDevice[0].usUsagePage = 0x01;
    rawInputDevice[0].usUsage = 0x02; //mouse
    rawInputDevice[0].dwFlags = 0;
	rawInputDevice[0].hwndTarget = window->hWnd;
    RegisterRawInputDevices(rawInputDevice, 1, sizeof(rawInputDevice[0]));
}

RawInput::~RawInput()
{
	memset(mouseSnapshot, 0, sizeof(mouseSnapshot));
	memset(mousePosAccum, 0, sizeof(mousePosAccum));

	delete[] keysSnapshot;
	delete[] keysRealtime;
	keysSnapshot = nullptr;
	keysRealtime = nullptr;
}

void RawInput::ProcessRawInput(const RAWINPUT * const rawInputData)
{
	if ((rawInputData == nullptr) || (rawInputData->header.dwType != RIM_TYPEMOUSE))
		return;

	std::lock_guard<std::mutex> lock(mutex);

	if (rawInputData->header.dwType == RIM_TYPEMOUSE) 
	{
		mousePosAccum[0] += rawInputData->data.mouse.lLastX;
		mousePosAccum[1] += rawInputData->data.mouse.lLastY;
	}
}

void RawInput::ProcessKey(bool keyDown, WPARAM wParam, LPARAM lParam)
{
	int virtualKeyCode;

	virtualKeyCode = wParam;
	if (virtualKeyCode >= numMaxKeyStrokes)
		return;

	std::lock_guard<std::mutex> lock(mutex);

	keysRealtime[virtualKeyCode] = keyDown;
}

void RawInput::Snapshot()
{
	std::lock_guard<std::mutex> lock(mutex);

	mouseSnapshot[0] = mousePosAccum[0];
	mouseSnapshot[1] = mousePosAccum[1];
	mouseSnapshot[2] = mousePosAccum[2];
	mousePosAccum[0] = mousePosAccum[1] = mousePosAccum[2] = 0.0f;

	memcpy(keysSnapshot, keysRealtime, sizeof(bool) * numMaxKeyStrokes);
}

bool RawInput::KStatus(const int &vcode) const	
{
	if (vcode >= numMaxKeyStrokes)
		return false;
	return keysSnapshot[vcode];
}

float RawInput::MStatusPosX() const
{
	return mouseSnapshot[0];
}

float RawInput::MStatusPosY() const
{
	return mouseSnapshot[1];
}

float RawInput::MStatusPosZ() const
{
	return mouseSnapshot[2];
}