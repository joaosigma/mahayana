#include "common\Platform.hpp"
#include "common\Common.hpp"
#include "common\UTF.hpp"
#include "common\Image.hpp"
#include "common\ImageFactory.hpp"
#include "common\opengl\openGL.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "winApp.hpp"

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
	WNDCLASSEXW windowClass;
	DWORD dwExStyle, dwStyle;
	RECT windowRect, desktopRect;
	DEVMODE	dmScreenSettings;
	wchar_t windowTitleWChar[256];

	if (this->isInitialized == true)
	{
		Window::MsgBoxErro("Window already initialized.\nApplication cannot proceed.");
		return false;
	}

	if ((windowTitle == nullptr) || (*windowTitle == '\0') || (winWidth == 0) || (winHeight == 0))
	{
		Window::MsgBoxErro("Incorrect data: cannot create window.\nApplication cannot proceed.");
		return false;
	}

	this->winWidth = winWidth;
	this->winHeight = winHeight;

	windowRect.left = windowRect.top = 0;
	windowRect.right = winWidth;
	windowRect.bottom = winHeight;

	HorseRadish::UTF::ConvertUTF8To("HorseRadish graphics engine...", HorseRadish::UTF::Windows, this->className, sizeof(this->className));
	HorseRadish::UTF::ConvertUTF8To(windowTitle, HorseRadish::UTF::Windows, windowTitleWChar, sizeof(windowTitleWChar));

	memset(&windowClass, 0, sizeof(WNDCLASSEXW));
	windowClass.cbSize = sizeof (WNDCLASSEXW);
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
		Window::MsgBoxErro("Unable to register window class.\nApplication cannot proceed.");
		return false;
	}

	this->originalDeviceMode.dmSize = sizeof(DEVMODE);
	this->originalDeviceMode.dmDriverExtra = 0;
	EnumDisplaySettingsEx(nullptr, ENUM_CURRENT_SETTINGS, &this->originalDeviceMode, 0);

	if (winFullscreen)
	{
		memset(&dmScreenSettings, 0, sizeof(DEVMODE));
		dmScreenSettings.dmSize = sizeof(DEVMODE);
		dmScreenSettings.dmPelsWidth = winWidth;
		dmScreenSettings.dmPelsHeight = winHeight;
		dmScreenSettings.dmBitsPerPel = 32;
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		if ((checkBestDisplayFrequency(&dmScreenSettings) == false) || (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN | CDS_RESET) != DISP_CHANGE_SUCCESSFUL))
		{
			Window::MsgBoxAviso("Unable to change to fullscreen.\nApplication will continue in window mode.");

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

	this->hWnd = CreateWindowEx(dwExStyle, this->className, windowTitleWChar, dwStyle, desktopRect.left + 5, desktopRect.top + 5, (windowRect.right - windowRect.left), (windowRect.bottom - windowRect.top), HWND_DESKTOP, nullptr, this->hInstance, nullptr);
	if (this->hWnd == nullptr)
	{
		Window::MsgBoxErro("Unable to create window!\nApplication cannot proceed.");

		UnregisterClass(this->className, this->hInstance);
		return false;
	}

	SetCursor(nullptr);

	this->isInitialized = true;
	return true;
}

bool Window::WindowEditorInit(WNDPROC procFunc, const HorseRadish::hChar *windowTitle, const unsigned int winWidth, const unsigned int winHeight, const HWND handleWindowParent)
{
	/*WNDCLASSEXW wc;
	DWORD dwExStyle, dwStyle;
	wchar_t windowTitleWChar[256];

	//se já iniciei
	if (this->isInitialized == true)
	{
		Window::MsgBoxErro("Window already initialized.\nApplication cannot proceed.");
		return false;
	}

	//verificar parametros e dados destes
	if (title==nullptr || winWidth==0 || winHeight==0 || OpenGLDllName==nullptr || *OpenGLDllName=='\0')
	{
		Window::MsgBoxErro("Incorrect data! Cannot create window.\nApplication cannot proceed.");
		return false;
	}

	//guardo isto
	this->winWidth = winWidth;
	this->winHeight = winHeight;

	//tenho de buscar os ponteiros para isto
	this->wglCreateContext=(HGLRC (APIENTRY *)(HDC hdc))GetProcAddress(GetModuleHandle(OpenGLDllName), "wglCreateContext");
	this->wglDeleteContext=(BOOL (APIENTRY *)(HGLRC hglrc))GetProcAddress(GetModuleHandle(OpenGLDllName), "wglDeleteContext");
	this->wglMakeCurrent=(BOOL (APIENTRY *)(HDC hdc, HGLRC hglrc))GetProcAddress(GetModuleHandle(OpenGLDllName), "wglMakeCurrent");
	this->wglSwapBuffers=(BOOL (APIENTRY *)(HDC hdc))GetProcAddress(GetModuleHandle(OpenGLDllName), "wglSwapBuffers");
	if (this->wglCreateContext==nullptr || this->wglDeleteContext==nullptr || this->wglMakeCurrent==nullptr || this->wglSwapBuffers==nullptr)
		return false;

	//guardo já o nome da classe e também transformo o titulo da janela
	HorseRadish::UTF::ConvertUTF8To("SIGMA Graphics Engine...", HorseRadish::UTF::Windows, this->className, sizeof(this->className));
	HorseRadish::UTF::ConvertUTF8To(windowTitle, HorseRadish::UTF::Windows, windowTitleWChar, sizeof(windowTitleWChar));

	//tento registar a classe da janela
	memset(&wc,0,sizeof(WNDCLASSEXW));
	wc.cbSize			= sizeof (WNDCLASSEXW);
	wc.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc		= (WNDPROC)procFunc;
	wc.cbClsExtra		= 0;
	wc.cbWndExtra		= 0;
	wc.hInstance		= this->hInstance;
	wc.hIcon			= nullptr;
	wc.hCursor			= nullptr;
	wc.hbrBackground	= nullptr;
	wc.lpszMenuName		= nullptr;
	wc.lpszClassName	= this->className;
	if (!RegisterClassEx(&wc))
	{
		Window::MsgBoxErro("Unable to register window class!\nApplication cannot proceed.");
		return false;
	}

	//sou filho de uma outra janela (o meu pai é o editor)
	dwExStyle = 0;
	dwStyle = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

	//crio a janela
	this->hWnd=CreateWindowEx(dwExStyle, this->className, windowTitleWChar, dwStyle, 0, 0, winWidth, winHeight, handleWindowParent, nullptr, this->hInstance, nullptr);
	if (this->hWnd==nullptr)
	{
		this->WindowKill();
		Window::MsgBoxErro("Unable to create window!\nApplication cannot proceed.");
		return false;
	}

	//obtenho o device context
	this->hDC=GetDC(this->hWnd);
	if (this->hDC==nullptr)
	{
		this->WindowKill();
		Window::MsgBoxErro("Unable to device context!\nApplication cannot proceed.");
		return false;
	}

	//procuro o melhor, tendo em conta o WGL
	this->usedPFD=checkBestPFD(this->hDC);

	//carrego o pixel format
	if(this->usedPFD==0 || SetPixelFormat(this->hDC,this->usedPFD,nullptr)==false)
	{
		this->WindowKill();
		Window::MsgBoxErro("Cannot find a useful pixel format!\nApplication cannot proceed.");
		return false;
	}

	//crio o rendering context
	if (!(this->hRC=this->wglCreateContext(this->hDC)))
	{
		this->WindowKill();
		Window::MsgBoxErro("Unable to create a rendering context!\nApplication cannot proceed.");
		return false;
	}

	//activo o rendering context
	if(!this->wglMakeCurrent(this->hDC,this->hRC))
	{
		this->WindowKill();
		Window::MsgBoxErro("Unable to activate a rendering context!\nApplication cannot proceed.");
		return false;
	}

	//carrego por fim o WGL
	HorseRadish::OpenGL::Windows::ExtensionsLoad(OpenGLDllName);

	//correu tudo bem
	this->isInitialized = true;
	return true;*/

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
			Window::MsgBoxErro("Unable to delete window handle.");

		this->hWnd = nullptr;
	}

	if (UnregisterClass(this->className, this->hInstance) == FALSE)
		Window::MsgBoxErro("Unable to unregister window class.");

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

void Window::MsgBoxAviso(const HorseRadish::String &msg)
{
	wchar_t msgConverted[512];

	msg.Convert(HorseRadish::String::Windows, msgConverted, sizeof(msgConverted));
	MessageBox(nullptr, msgConverted, L"Warning", MB_OK | MB_ICONWARNING);
}

void Window::MsgBoxAviso(const char * const msg)
{
	HorseRadish::String msgConvertida;

	msgConvertida.Set(HorseRadish::String::UTF8, msg);
	Window::MsgBoxAviso(msgConvertida);
}

void Window::MsgBoxErro(const HorseRadish::String &msg)
{
	wchar_t msgConverted[512];

	msg.Convert(HorseRadish::String::Windows, msgConverted, sizeof(msgConverted));
	MessageBox(nullptr, msgConverted, L"Error", MB_OK | MB_ICONERROR);
}

void Window::MsgBoxErro(const char * const msg)
{
	HorseRadish::String msgConvertida;

	msgConvertida.Set(HorseRadish::String::UTF8, msg);
	Window::MsgBoxErro(msgConvertida);
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

#define WGL_CONTEXT_DEBUG_BIT_ARB      0x00000001
#define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB 0x00000002
#define WGL_CONTEXT_MAJOR_VERSION_ARB  0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB  0x2092
#define WGL_CONTEXT_LAYER_PLANE_ARB    0x2093
#define WGL_CONTEXT_FLAGS_ARB          0x2094
#define ERROR_INVALID_VERSION_ARB      0x209

#define WGL_CONTEXT_PROFILE_MASK_ARB   0x9126
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB 0x00000001
#define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x00000002
#define ERROR_INVALID_PROFILE_ARB      0x2096

#define WGL_NUMBER_PIXEL_FORMATS_ARB   0x2000
#define WGL_DRAW_TO_WINDOW_ARB         0x2001
#define WGL_DRAW_TO_BITMAP_ARB         0x2002
#define WGL_ACCELERATION_ARB           0x2003
#define WGL_NEED_PALETTE_ARB           0x2004
#define WGL_NEED_SYSTEM_PALETTE_ARB    0x2005
#define WGL_SWAP_LAYER_BUFFERS_ARB     0x2006
#define WGL_SWAP_METHOD_ARB            0x2007
#define WGL_NUMBER_OVERLAYS_ARB        0x2008
#define WGL_NUMBER_UNDERLAYS_ARB       0x2009
#define WGL_TRANSPARENT_ARB            0x200A
#define WGL_TRANSPARENT_RED_VALUE_ARB  0x2037
#define WGL_TRANSPARENT_GREEN_VALUE_ARB 0x2038
#define WGL_TRANSPARENT_BLUE_VALUE_ARB 0x2039
#define WGL_TRANSPARENT_ALPHA_VALUE_ARB 0x203A
#define WGL_TRANSPARENT_INDEX_VALUE_ARB 0x203B
#define WGL_SHARE_DEPTH_ARB            0x200C
#define WGL_SHARE_STENCIL_ARB          0x200D
#define WGL_SHARE_ACCUM_ARB            0x200E
#define WGL_SUPPORT_GDI_ARB            0x200F
#define WGL_SUPPORT_OPENGL_ARB         0x2010
#define WGL_DOUBLE_BUFFER_ARB          0x2011
#define WGL_STEREO_ARB                 0x2012
#define WGL_PIXEL_TYPE_ARB             0x2013
#define WGL_COLOR_BITS_ARB             0x2014
#define WGL_RED_BITS_ARB               0x2015
#define WGL_RED_SHIFT_ARB              0x2016
#define WGL_GREEN_BITS_ARB             0x2017
#define WGL_GREEN_SHIFT_ARB            0x2018
#define WGL_BLUE_BITS_ARB              0x2019
#define WGL_BLUE_SHIFT_ARB             0x201A
#define WGL_ALPHA_BITS_ARB             0x201B
#define WGL_ALPHA_SHIFT_ARB            0x201C
#define WGL_ACCUM_BITS_ARB             0x201D
#define WGL_ACCUM_RED_BITS_ARB         0x201E
#define WGL_ACCUM_GREEN_BITS_ARB       0x201F
#define WGL_ACCUM_BLUE_BITS_ARB        0x2020
#define WGL_ACCUM_ALPHA_BITS_ARB       0x2021
#define WGL_DEPTH_BITS_ARB             0x2022
#define WGL_STENCIL_BITS_ARB           0x2023
#define WGL_AUX_BUFFERS_ARB            0x2024
#define WGL_NO_ACCELERATION_ARB        0x2025
#define WGL_GENERIC_ACCELERATION_ARB   0x2026
#define WGL_FULL_ACCELERATION_ARB      0x2027
#define WGL_SWAP_EXCHANGE_ARB          0x2028
#define WGL_SWAP_COPY_ARB              0x2029
#define WGL_SWAP_UNDEFINED_ARB         0x202A
#define WGL_TYPE_RGBA_ARB              0x202B
#define WGL_TYPE_COLORINDEX_ARB        0x202C

#define WGL_TYPE_RGBA_FLOAT_ARB		0x21A0

void OpenglContext::loadWGLFunctions(HMODULE openglModule)
{
	PROC (APIENTRY *ptrWGlGetProcAddress)(LPCSTR lpcstr);

	ptrWGlGetProcAddress = (PROC (APIENTRY *)(LPCSTR lpcstr))GetProcAddress(openglModule, "wglGetProcAddress");
	if (ptrWGlGetProcAddress == nullptr)
		return;

	this->wglCreateContextAttribsARB = (OpenglContext::PFNWGLCREATECONTEXTATTRIBSARBPROC)ptrWGlGetProcAddress("wglCreateContextAttribsARB");
	this->wglGetExtensionsStringARB = (OpenglContext::PFNWGLGETEXTENSIONSSTRINGARBPROC)ptrWGlGetProcAddress("wglGetExtensionsStringARB");
	this->wglGetPixelFormatAttribivARB = (OpenglContext::PFNWGLGETPIXELFORMATATTRIBIVARBPROC)ptrWGlGetProcAddress("wglGetPixelFormatAttribivARB");
	this->wglGetPixelFormatAttribfvARB = (OpenglContext::PFNWGLGETPIXELFORMATATTRIBFVARBPROC)ptrWGlGetProcAddress("wglGetPixelFormatAttribfvARB");
	this->wglChoosePixelFormatARB = (OpenglContext::PFNWGLCHOOSEPIXELFORMATARBPROC)ptrWGlGetProcAddress("wglChoosePixelFormatARB");
	this->wglSwapIntervalEXT = (OpenglContext::PFNWGLSWAPINTERVALEXTPROC)ptrWGlGetProcAddress("wglSwapIntervalEXT");
	this->wglGetSwapIntervalEXT = (OpenglContext::PFNWGLGETSWAPINTERVALEXTPROC)ptrWGlGetProcAddress("wglGetSwapIntervalEXT");
}

bool OpenglContext::auxWindowWGLExt(HINSTANCE hInstance, HMODULE openglModule)
{
	WNDCLASS winClassAux;
	HWND hWndAux;
	HDC hDCAux;
	HGLRC hRCAux;
	PIXELFORMATDESCRIPTOR pfFormatD;
	
	memset(&winClassAux, 0, sizeof(WNDCLASS));
	winClassAux.hInstance = hInstance;
	winClassAux.lpszClassName = L"gl aux window";
	winClassAux.lpfnWndProc = auxWindowWGLExtProc;
	if (RegisterClass(&winClassAux) == 0)
		return false;

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
	int contextAttrib[10];
	const char *wglExt;
	HMODULE openglModule;
	wchar_t openGLModuleNameWChar[128];

	this->window = window;

	if ( (this->window == nullptr) || (this->window->hWnd == nullptr) || (openGLModuleName == nullptr) || (*openGLModuleName == '\0') || (contextMajorVersion < 3) || (contextMinorVersion < 0))
	{
		Window::MsgBoxErro("Incorrect data! Cannot create window.\nApplication cannot proceed.");
		return;
	}

	HorseRadish::UTF::ConvertUTF8To(openGLModuleName, HorseRadish::UTF::Windows, openGLModuleNameWChar, sizeof(openGLModuleNameWChar));

	openglModule = GetModuleHandle(openGLModuleNameWChar);
	if (openglModule == nullptr)
	{
		Window::MsgBoxErro("Incorrect OpenGL module name.\nApplication cannot proceed.");
		return;
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
		Window::MsgBoxErro("Unable to device context!\nApplication cannot proceed.");
		return;
	}

	wglExt = this->wglGetExtensionsStringARB(this->hDC);
	if ((wglExt == nullptr) || (strstr(wglExt, "WGL_ARB_create_context_profile") == nullptr))
	{
		Window::MsgBoxErro("Extensions WGL_ARB_create_context_profile not supported!\nApplication cannot proceed.");
		return;
	}

	{
		int pixelFormat;
		UINT numFormats;

		float fAttributes[] = {0,0};
		int iAttributes[] = {
			WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
			WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
			WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
			WGL_COLOR_BITS_ARB, 24,
			WGL_ALPHA_BITS_ARB, 8,
			WGL_DEPTH_BITS_ARB, 0,
			WGL_STENCIL_BITS_ARB, 0,
			WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
			0, 0};

		this->usedPFD = -1;
		if (this->wglChoosePixelFormatARB(this->hDC, iAttributes, fAttributes, 1, &pixelFormat, &numFormats) == TRUE)
			this->usedPFD = pixelFormat;
	}

	if ((this->usedPFD == 0) || (SetPixelFormat(this->hDC,this->usedPFD,nullptr) == false))
	{
		Window::MsgBoxErro("Cannot find a useful pixel format!\nApplication cannot proceed.");
		return;
	}

	contextAttrib[0] = WGL_CONTEXT_MAJOR_VERSION_ARB;
	contextAttrib[1] = contextMajorVersion;
	contextAttrib[2] = WGL_CONTEXT_MINOR_VERSION_ARB;
	contextAttrib[3] = contextMinorVersion;
	contextAttrib[4] = WGL_CONTEXT_FLAGS_ARB;
	contextAttrib[5] = 0;
	contextAttrib[5] |= contextDebug ? WGL_CONTEXT_DEBUG_BIT_ARB : 0;
	contextAttrib[5] |= contextForwardCompatible ? WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB : 0;
	contextAttrib[6] = WGL_CONTEXT_PROFILE_MASK_ARB;
	contextAttrib[7] = WGL_CONTEXT_CORE_PROFILE_BIT_ARB;
	contextAttrib[8] = 0;
	contextAttrib[9] = 0;

	this->hRC = this->wglCreateContextAttribsARB(this->hDC, 0, contextAttrib);
	if (this->hRC == nullptr)
	{
		Window::MsgBoxErro("Unable to create a rendering context!\nApplication cannot proceed.");
		return;
	}

	if (this->wglMakeCurrent(this->hDC, this->hRC) == FALSE)
	{
		Window::MsgBoxErro("Unable to activate a rendering context!\nApplication cannot proceed.");
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
			Window::MsgBoxErro("Unable to release rendering context.");

		if (this->wglDeleteContext(this->hRC) == FALSE)
			Window::MsgBoxErro("Unable to delete rendering context.");

		this->hRC = nullptr;
	}

	if (this->hDC != nullptr)
	{
		if (ReleaseDC(this->window->hWnd, this->hDC) == 0)
			Window::MsgBoxErro("Unable to release device context.");

		this->hDC = nullptr;
	}
}

bool OpenglContext::TakeScreenshot(HorseRadish::Streams::FileStream &fileStream) const
{
	HorseRadish::Imaging::Image *imgWrite;
	unsigned char *pixels;

	pixels = new unsigned char[this->window->winWidth * this->window->winHeight * 3];

	HorseRadish::OpenGL::glReadPixels(0, 0, this->window->winWidth, this->window->winHeight, GL_BGR, GL_UNSIGNED_BYTE, pixels);
	
	imgWrite = new HorseRadish::Imaging::Image(this->window->winWidth, this->window->winHeight, HorseRadish::Imaging::Image::UByte, HorseRadish::Imaging::Image::BGR, pixels, true);

	HorseRadish::Imaging::Factory::SaveBMP(&HorseRadish::Streams::StreamWriter(&fileStream), imgWrite);

	delete imgWrite;

	return true;
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
{
	RAWINPUTDEVICE rawInputDevice[1];

	memset(ratoSnapshot, 0, sizeof(ratoSnapshot));
	memset(ratoPosAccum, 0, sizeof(ratoPosAccum));

	numMaxTeclas = 256;
	teclasSnapshot = new bool[numMaxTeclas];
	teclasTempoReal = new bool[numMaxTeclas];

	memset(teclasSnapshot, 0, sizeof(bool) * numMaxTeclas);
	memset(teclasTempoReal, 0, sizeof(bool) * numMaxTeclas);

    rawInputDevice[0].usUsagePage = 0x01;
    rawInputDevice[0].usUsage = 0x02;	//mouse
    rawInputDevice[0].dwFlags = 0;
	rawInputDevice[0].hwndTarget = window->hWnd;
    RegisterRawInputDevices(rawInputDevice, 1, sizeof(rawInputDevice[0]));
}

RawInput::~RawInput()
{
	memset(ratoSnapshot, 0, sizeof(ratoSnapshot));
	memset(ratoPosAccum, 0, sizeof(ratoPosAccum));

	delete[] teclasSnapshot;
	delete[] teclasTempoReal;
	teclasSnapshot = nullptr;
	teclasTempoReal = nullptr;
}

void RawInput::ProcessRawInput(const RAWINPUT * const rawInputData)
{
	if ((rawInputData == nullptr) || (rawInputData->header.dwType != RIM_TYPEMOUSE))
		return;

	std::lock_guard<std::mutex> lock(mutex);

	if (rawInputData->header.dwType == RIM_TYPEMOUSE) 
	{
		ratoPosAccum[0] += rawInputData->data.mouse.lLastX;
		ratoPosAccum[1] += rawInputData->data.mouse.lLastY;
	}
}

void RawInput::ProcessKey(bool keyDown, WPARAM wParam, LPARAM lParam)
{
	int virtualKeyCode;

	virtualKeyCode = wParam;
	if (virtualKeyCode >= numMaxTeclas)
		return;

	std::lock_guard<std::mutex> lock(mutex);

	teclasTempoReal[virtualKeyCode] = keyDown;
}

void RawInput::Snapshot()
{
	std::lock_guard<std::mutex> lock(mutex);

	ratoSnapshot[0] = ratoPosAccum[0];
	ratoSnapshot[1] = ratoPosAccum[1];
	ratoSnapshot[2] = ratoPosAccum[2];
	ratoPosAccum[0] = ratoPosAccum[1] = ratoPosAccum[2] = 0.0f;

	memcpy(teclasSnapshot, teclasTempoReal, sizeof(bool) * numMaxTeclas);
}

bool RawInput::KStatus(const int &vcode) const	
{
	if (vcode >= numMaxTeclas)
		return false;
	return teclasSnapshot[vcode];
}

float RawInput::MStatusPosX() const
{
	return ratoSnapshot[0];
}

float RawInput::MStatusPosY() const
{
	return ratoSnapshot[1];
}

float RawInput::MStatusPosZ() const
{
	return ratoSnapshot[2];
}