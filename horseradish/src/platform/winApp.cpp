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

//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§   funções locais auxiliares	§§§§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
static
bool checkBestDisplayFrequency(DEVMODE * const deviceMode)
{
	DEVMODE deviceModeAux;
	DWORD bestModeIndex, bestModeFrequency;

	//verificar isto
	if (deviceMode == nullptr)
		return false;

	//limpo isto
	memset(&deviceModeAux, 0, sizeof(DEVMODE));
	deviceModeAux.dmSize = sizeof(DEVMODE);

	//por omissão
	bestModeIndex = 0;
	bestModeFrequency = 0;

	//percorro todos os modos possíveis para o display
	for (DWORD modeIndex = 0; EnumDisplaySettingsEx(nullptr, modeIndex, &deviceModeAux, 0) != FALSE; modeIndex++)
	{
		//ignoro aqueles que não têm as características do que é preciso
		if ((deviceModeAux.dmBitsPerPel != deviceMode->dmBitsPerPel) || (deviceModeAux.dmPelsHeight != deviceMode->dmPelsHeight) || (deviceModeAux.dmPelsWidth != deviceMode->dmPelsWidth))
			continue;

		//se não for melhor do que já tenho
		if (deviceModeAux.dmDisplayFrequency < bestModeFrequency)
			continue;

		//este passa a ser o melhor
		bestModeIndex = modeIndex;
		bestModeFrequency = deviceModeAux.dmDisplayFrequency;
	}

	//se não há melhor
	if (bestModeFrequency == 0)
		return false;

	//leio o melhor que encontrei e pronto
	EnumDisplaySettingsEx(nullptr, bestModeIndex, deviceMode, 0);
	return true;
}

static
LRESULT CALLBACK auxWindowWGLExtProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return DefWindowProc(hwnd, message, wParam, lParam);
};

//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§   Classe Window	§§§§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
Window::Window(HINSTANCE hInst)
{
	//guardo isto
	this->hInstance = hInst;

	//limpo isto tudo
	this->hWnd = nullptr;
	this->isFullscreen = false;
	this->isInitialized = false;

	//e também estes buffers
	memset(&this->originalDeviceMode, 0, sizeof(this->originalDeviceMode));
	memset(this->className, 0, sizeof(this->className));
}

Window::~Window()
{
	//se ainda não fechei tudo como deve de ser, faço-o antes de terminar
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

	//se já iniciei
	if (this->isInitialized == true)
	{
		Window::MsgBoxErro("Window already initialized.\nApplication cannot proceed.");
		return false;
	}

	//verificar parametros e dados destes
	if ((windowTitle == nullptr) || (*windowTitle == '\0') || (winWidth == 0) || (winHeight == 0))
	{
		Window::MsgBoxErro("Incorrect data: cannot create window.\nApplication cannot proceed.");
		return false;
	}

	//guardo isto
	this->winWidth = winWidth;
	this->winHeight = winHeight;

	//o tamanho da janela que quero
	windowRect.left = windowRect.top = 0;
	windowRect.right = winWidth;
	windowRect.bottom = winHeight;

	//guardo já o nome da classe e também transformo o titulo da janela
	HorseRadish::UTF::ConvertUTF8To("HorseRadish graphics engine...", HorseRadish::UTF::Windows, this->className, sizeof(this->className));
	HorseRadish::UTF::ConvertUTF8To(windowTitle, HorseRadish::UTF::Windows, windowTitleWChar, sizeof(windowTitleWChar));

	//tento registar a classe da janela
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

	//guardo o estado actual o ecran (caso o mude para fullscreen)
	this->originalDeviceMode.dmSize = sizeof(DEVMODE);
	this->originalDeviceMode.dmDriverExtra = 0;
	EnumDisplaySettingsEx(nullptr, ENUM_CURRENT_SETTINGS, &this->originalDeviceMode, 0);

	//se ponho ou não em ecran inteiro
	if (winFullscreen == true)
	{
		//qual o tamanho com que quero ficar
		memset(&dmScreenSettings, 0, sizeof(DEVMODE));
		dmScreenSettings.dmSize = sizeof(DEVMODE);
		dmScreenSettings.dmPelsWidth = winWidth;
		dmScreenSettings.dmPelsHeight = winHeight;
		dmScreenSettings.dmBitsPerPel = 32;
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		//tento achar o melhor e tento colocá-lo
		if ((checkBestDisplayFrequency(&dmScreenSettings) == false) || (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN | CDS_RESET) != DISP_CHANGE_SUCCESSFUL))
		{
			//aviso que falhou
			Window::MsgBoxAviso("Unable to change to fullscreen.\nApplication will continue in window mode.");

			//não consegui, meto isto como se tivesse em janela
			dwExStyle = WS_EX_APPWINDOW;
			dwStyle = WS_CAPTION | WS_VISIBLE;
			AdjustWindowRectEx(&windowRect, dwStyle, false, dwExStyle);
		}
		else
		{
			//estou em fullscreen
			dwExStyle = WS_EX_APPWINDOW;
			dwStyle = WS_POPUP | WS_VISIBLE;

			//tenho de guardar isto
			this->isFullscreen = true;
		}
	}
	else
	{
		//não estou em fullscreen
		dwExStyle = WS_EX_APPWINDOW;
		dwStyle = WS_CAPTION | WS_VISIBLE;
		AdjustWindowRectEx(&windowRect, dwStyle, false, dwExStyle);
	}

	//get desktop area
	if (SystemParametersInfo(SPI_GETWORKAREA, 0, &desktopRect, 0) != TRUE)
		desktopRect.bottom = desktopRect.left = desktopRect.right = desktopRect.top = 0;

	//crio a janela propriamente dita
	this->hWnd = CreateWindowEx(dwExStyle, this->className, windowTitleWChar, dwStyle, desktopRect.left + 5, desktopRect.top + 5, (windowRect.right - windowRect.left), (windowRect.bottom - windowRect.top), HWND_DESKTOP, nullptr, this->hInstance, nullptr);
	if (this->hWnd == nullptr)
	{
		//aviso
		Window::MsgBoxErro("Unable to create window!\nApplication cannot proceed.");

		//para evitar problemas, apago o registo da classe da janela
		UnregisterClass(this->className, this->hInstance);
		return false;
	}

	//não quero cursor
	SetCursor(nullptr);

	//correu tudo bem
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
	//se já fechei tudo, não faço nada
	if (this->isInitialized == false)
		return;

	//se tenho de sair de fullscreen
	if (this->isFullscreen)
	{
		ChangeDisplaySettings(&this->originalDeviceMode, CDS_RESET | CDS_UPDATEREGISTRY);
		this->isFullscreen = false;
	}

	//se tenho uma janela
	if (this->hWnd != nullptr)
	{
		//destruo a janela
		if (DestroyWindow(this->hWnd) == FALSE)
			Window::MsgBoxErro("Unable to delete window handle.");

		//não a quero para nada
		this->hWnd = nullptr;
	}

	//apago o registo da classe
	if (UnregisterClass(this->className, this->hInstance) == FALSE)
		Window::MsgBoxErro("Unable to unregister window class.");

	//e pronto, já terminei
	this->isInitialized = false;
}

bool Window::SetWindowAlpha(const unsigned char &valorAlpha) const
{
	//so faço se estiver em janela
	if (this->isFullscreen)
		return false;

	//mudo o estilo da janela (precisa de ter WS_EX_LAYERED)
	SetWindowLong(this->hWnd, GWL_EXSTYLE, GetWindowLong(this->hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);

	//agora basta mudar o alpha da janela
	return (SetLayeredWindowAttributes(this->hWnd, RGB(0,0,0), (valorAlpha < 10) ? 10 : valorAlpha, LWA_ALPHA) == TRUE);
}

bool Window::SendMessageClose() const
{
	//basta mandar esta mensagem
	return (SendNotifyMessage(this->hWnd, WM_CLOSE, 0, 0) == TRUE);
}

bool Window::SetFocus() const
{
	//basta chamar esta função
	return (::SetFocus(this->hWnd) != nullptr);
}

void Window::PeekMessageDispatch(bool translateMessage) const
{
	MSG msg;

	//mando verificar se existem mensagens disponíveis
	if (PeekMessage(&msg, this->hWnd, 0, 0, PM_REMOVE) == TRUE)
	{
		//chegando aqui tenho uma mensagem válida, por isso traduzo-a (se for necessário) e mando-a
		if (translateMessage)
			TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

void Window::MsgBoxInfo(const HorseRadish::String &msg)
{
	wchar_t msgConverted[512];

	//converto a mensagem e mostro-a
	msg.Convert(HorseRadish::String::Windows, msgConverted, sizeof(msgConverted));
	MessageBox(nullptr, msgConverted, L"Info", MB_OK | MB_ICONINFORMATION);
}

void Window::MsgBoxInfo(const char * const msg)
{
	HorseRadish::String msgConvertida;

	//converto a mensagem e pronto
	msgConvertida.Set(HorseRadish::String::UTF8, msg);
	Window::MsgBoxInfo(msgConvertida);
}

void Window::MsgBoxAviso(const HorseRadish::String &msg)
{
	wchar_t msgConverted[512];

	//converto a mensagem e mostro-a
	msg.Convert(HorseRadish::String::Windows, msgConverted, sizeof(msgConverted));
	MessageBox(nullptr, msgConverted, L"Warning", MB_OK | MB_ICONWARNING);
}

void Window::MsgBoxAviso(const char * const msg)
{
	HorseRadish::String msgConvertida;

	//converto a mensagem e pronto
	msgConvertida.Set(HorseRadish::String::UTF8, msg);
	Window::MsgBoxAviso(msgConvertida);
}

void Window::MsgBoxErro(const HorseRadish::String &msg)
{
	wchar_t msgConverted[512];

	//converto a mensagem e mostro-a
	msg.Convert(HorseRadish::String::Windows, msgConverted, sizeof(msgConverted));
	MessageBox(nullptr, msgConverted, L"Error", MB_OK | MB_ICONERROR);
}

void Window::MsgBoxErro(const char * const msg)
{
	HorseRadish::String msgConvertida;

	//converto a mensagem e pronto
	msgConvertida.Set(HorseRadish::String::UTF8, msg);
	Window::MsgBoxErro(msgConvertida);
}

bool Window::CommandLineHasParam(PWSTR cmdLine, const HorseRadish::hChar * const parameterName)
{
	HorseRadish::String commandLine, curToken;

	//verifico parametros
	if ((cmdLine == nullptr) || (parameterName == nullptr))
		return false;

	//transformo para string
	commandLine.Set(HorseRadish::String::Windows, cmdLine);

	//passo por todos os tokens que tenho
	for(HorseRadish::String::Tokenizer tok(commandLine, ' '); tok.IsLast() == false; )
	{
		//tiro este token
		tok.Read(curToken);
		if (curToken.GetSizeBytes() == 0)
			break;

		//limpo o token
		curToken.Trim();

		//se for este o parâmetro, já está
		if (curToken == parameterName)
			return true;
	}

	//chegando aqui não achei nada
	return false;
}

bool Window::CommandLineGetParam(PWSTR cmdLine, const HorseRadish::hChar * const parameterName, HorseRadish::String &parameterValue)
{
	HorseRadish::String commandLine, curToken;

	//por omissão
	parameterValue.SetEmpty();

	//verifico parametros
	if ((cmdLine == nullptr) || (parameterName == nullptr))
		return false;

	//transformo para string
	commandLine.Set(HorseRadish::String::Windows, cmdLine);

	//passo por todos os tokens que tenho
	for(HorseRadish::String::Tokenizer tok(commandLine, ' '); tok.IsLast() == false; )
	{
		//tiro este token
		tok.Read(curToken);
		if (curToken.GetSizeBytes() == 0)
			break;

		//limpo o token
		curToken.Trim();

		//se não for este o parâmetro
		if (curToken != parameterName)
			continue;

		//se estou no fim, correu mal
		if (tok.IsLast() == true)
			return false;

		//leio o próximo token e pronto
		tok.Read(parameterValue);
		return true;
	}

	//chegando aqui não achei nada
	return false;
}

bool Window::CommandLineGetParam(PWSTR cmdLine, const HorseRadish::hChar * const parameterName, int &parameterValue)
{
	HorseRadish::String paramValueString;

	//peço o parâmetro
	if (Window::CommandLineGetParam(cmdLine, parameterName, paramValueString) == false)
		return false;

	//converto para inteiro e já tá
	parameterValue = paramValueString.ToInt();
	return true;
}

//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§   Classe OpenglContext	§§§§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§

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

	//tento obter este ponteiro
	ptrWGlGetProcAddress = (PROC (APIENTRY *)(LPCSTR lpcstr))GetProcAddress(openglModule, "wglGetProcAddress");
	if (ptrWGlGetProcAddress == nullptr)
		return;

	//tiro todos os ponteiros necessários
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
	
	//tenho de registar uma classe qualquer
	memset(&winClassAux, 0, sizeof(WNDCLASS));
	winClassAux.hInstance = hInstance;
	winClassAux.lpszClassName = L"gl aux window";
	winClassAux.lpfnWndProc = auxWindowWGLExtProc;
	if (RegisterClass(&winClassAux) == 0)
		return false;

	//crio a janela
	hWndAux = CreateWindow(L"gl aux window", L"gl aux window", WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, 0, 0, 8, 8, HWND_DESKTOP, nullptr, hInstance, nullptr);
	if (hWndAux == nullptr)
	{
		UnregisterClass(L"gl aux window", hInstance);
		return false;
	}

	//preparo o pixelFormat
	memset(&pfFormatD, 0, sizeof(PIXELFORMATDESCRIPTOR));
	pfFormatD.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfFormatD.nVersion = 1;
	pfFormatD.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfFormatD.iPixelType = PFD_TYPE_RGBA;
	pfFormatD.cColorBits = 32;
	pfFormatD.cDepthBits = 24;
	pfFormatD.iLayerType = PFD_MAIN_PLANE;
	
	//trato das tretas do DC, pixelFormat, RC
	hDCAux = GetDC(hWndAux);
	SetPixelFormat(hDCAux, ChoosePixelFormat(hDCAux, &pfFormatD), &pfFormatD);
	hRCAux = this->wglCreateContext(hDCAux);
	this->wglMakeCurrent(hDCAux, hRCAux);

	//carrego as extensões do WGL
	this->loadWGLFunctions(openglModule);

	//limpo tudo e já tá
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

	//guardo isto
	this->window = window;

	//verificar parametros
	if ( (this->window == nullptr) || (this->window->hWnd == nullptr) || (openGLModuleName == nullptr) || (*openGLModuleName == '\0') || (contextMajorVersion < 3) || (contextMinorVersion < 0))
	{
		Window::MsgBoxErro("Incorrect data! Cannot create window.\nApplication cannot proceed.");
		return;
	}

	//tenho de converter o nome do modulo de OpenGL para WideChar
	HorseRadish::UTF::ConvertUTF8To(openGLModuleName, HorseRadish::UTF::Windows, openGLModuleNameWChar, sizeof(openGLModuleNameWChar));

	//isto dá jeito
	openglModule = GetModuleHandle(openGLModuleNameWChar);
	if (openglModule == nullptr)
	{
		Window::MsgBoxErro("Incorrect OpenGL module name.\nApplication cannot proceed.");
		return;
	}

	//tenho de buscar os ponteiros para isto
	this->wglCreateContext = (HGLRC (APIENTRY *)(HDC hdc))GetProcAddress(openglModule, "wglCreateContext");
	this->wglDeleteContext = (BOOL (APIENTRY *)(HGLRC hglrc))GetProcAddress(openglModule, "wglDeleteContext");
	this->wglMakeCurrent = (BOOL (APIENTRY *)(HDC hdc, HGLRC hglrc))GetProcAddress(openglModule, "wglMakeCurrent");
	this->wglSwapBuffers = (BOOL (APIENTRY *)(HDC hdc))GetProcAddress(openglModule, "wglSwapBuffers");
	if ((this->wglCreateContext == nullptr) || (this->wglDeleteContext == nullptr) || (this->wglMakeCurrent == nullptr) || (this->wglSwapBuffers == nullptr))
		return;

	//a primeira coisa é que tenho de ler as extensões WGL que tenho (e pra isso preciso de uma janela temporária)
	auxWindowWGLExt(this->window->hInstance, openglModule);

	//não continuo se não tiver isto
	if ((this->wglChoosePixelFormatARB == nullptr) || (this->wglGetExtensionsStringARB == nullptr) || (this->wglCreateContextAttribsARB == nullptr))
		return;

	//obtenho o device context
	this->hDC = GetDC(this->window->hWnd);
	if (this->hDC == nullptr)
	{
		Window::MsgBoxErro("Unable to device context!\nApplication cannot proceed.");
		return;
	}

	//preciso de ler a string das extensões e verificar se existe a que procuro
	wglExt = this->wglGetExtensionsStringARB(this->hDC);
	if ((wglExt == nullptr) || (strstr(wglExt, "WGL_ARB_create_context_profile") == nullptr))
	{
		Window::MsgBoxErro("Extensions WGL_ARB_create_context_profile not supported!\nApplication cannot proceed.");
		return;
	}

	//procuro o melhor PFD
	{
		int pixelFormat;
		UINT numFormats;

		//crio os atributos necessários
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

		//mando escolher um pixelFormat adequado
		this->usedPFD = -1;
		if (this->wglChoosePixelFormatARB(this->hDC, iAttributes, fAttributes, 1, &pixelFormat, &numFormats) == TRUE)
			this->usedPFD = pixelFormat;
	}

	//carrego o pixel format
	if ((this->usedPFD == 0) || (SetPixelFormat(this->hDC,this->usedPFD,nullptr) == false))
	{
		Window::MsgBoxErro("Cannot find a useful pixel format!\nApplication cannot proceed.");
		return;
	}

	//crios os atributos pretendidos para o rendering context
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

	//crio o rendering context
	this->hRC = this->wglCreateContextAttribsARB(this->hDC, 0, contextAttrib);
	if (this->hRC == nullptr)
	{
		Window::MsgBoxErro("Unable to create a rendering context!\nApplication cannot proceed.");
		return;
	}

	//activo o rendering context
	if (this->wglMakeCurrent(this->hDC, this->hRC) == FALSE)
	{
		Window::MsgBoxErro("Unable to activate a rendering context!\nApplication cannot proceed.");
		return;
	}

	//acabei de criar o contexto
	this->contextCreated = true;

	//carrego as extensões do WGL
	this->loadWGLFunctions(openglModule);
}

OpenglContext::~OpenglContext()
{
	//se tiver um rendering context
	if (this->hRC != nullptr)
	{
		//deixa de ser o actual
		if (this->wglMakeCurrent(this->hDC, nullptr) == FALSE)
			Window::MsgBoxErro("Unable to release rendering context.");

		//apago-o
		if (this->wglDeleteContext(this->hRC) == FALSE)
			Window::MsgBoxErro("Unable to delete rendering context.");

		//já não tenho nada
		this->hRC = nullptr;
	}

	//se tiver um device context
	if (this->hDC != nullptr)
	{
		//liberto-o
		if (ReleaseDC(this->window->hWnd, this->hDC) == 0)
			Window::MsgBoxErro("Unable to release device context.");

		//já não tenho nada
		this->hDC = nullptr;
	}
}

bool OpenglContext::TakeScreenshot(HorseRadish::Streams::FileStream &fileStream) const
{
	HorseRadish::Imaging::Image *imgWrite;
	unsigned char *pixels;

	//buffer temporário para a imagem
	pixels = new unsigned char[this->window->winWidth * this->window->winHeight * 3];

	//leio os pixeis
	HorseRadish::OpenGL::glReadPixels(0, 0, this->window->winWidth, this->window->winHeight, GL_BGR, GL_UNSIGNED_BYTE, pixels);
	
	//crio uma nova imagem
	imgWrite = new HorseRadish::Imaging::Image(this->window->winWidth, this->window->winHeight, HorseRadish::Imaging::Image::UByte, HorseRadish::Imaging::Image::BGR, pixels, true);

	//mando escrever a imagem
	HorseRadish::Imaging::Factory::SaveBMP(&HorseRadish::Streams::StreamWriter(&fileStream), imgWrite);

	//apago a imagem (apaga automaticamente os pixeis)
	delete imgWrite;

	//e correu tudo bem
	return true;
}

void OpenglContext::SetSwapInterval(const unsigned int &interval) const
{
	//basta verificar se tenho a função e chamo-a
	if (this->wglSwapIntervalEXT != nullptr)
		this->wglSwapIntervalEXT(interval);
}

bool OpenglContext::SwapBuffers() const
{
	//basta verificar se tenho a função e chamo-a
	return ((this->wglSwapBuffers != nullptr) && (this->wglSwapBuffers(this->hDC) != FALSE));
}

//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§   Classe RawInput	§§§§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
RawInput::RawInput(const Window * const window)
{
	RAWINPUTDEVICE rawInputDevice[1];

	//lipo isto
	memset(ratoSnapshot, 0, sizeof(ratoSnapshot));
	memset(ratoPosAccum, 0, sizeof(ratoPosAccum));

	//preciso de espaço as teclas
	numMaxTeclas = 256;
	teclasSnapshot = new bool[numMaxTeclas];
	teclasTempoReal = new bool[numMaxTeclas];

	//limpo o estado das teclas
	memset(teclasSnapshot, 0, sizeof(bool) * numMaxTeclas);
	memset(teclasTempoReal, 0, sizeof(bool) * numMaxTeclas);

	//preciso disto
	InitializeCriticalSection(&criticalSection);

	//só tenho de registar o rato
    rawInputDevice[0].usUsagePage = 0x01;
    rawInputDevice[0].usUsage = 0x02;	//o rato
    rawInputDevice[0].dwFlags = 0;
	rawInputDevice[0].hwndTarget = window->hWnd;
    RegisterRawInputDevices(rawInputDevice, 1, sizeof(rawInputDevice[0]));
}

//destrutor
RawInput::~RawInput()
{
	//já não preciso disto
	DeleteCriticalSection(&criticalSection);

	//limpo as coisas do rato
	memset(ratoSnapshot, 0, sizeof(ratoSnapshot));
	memset(ratoPosAccum, 0, sizeof(ratoPosAccum));

	//limpo as coisas do teclado
	delete[] teclasSnapshot;
	delete[] teclasTempoReal;
	teclasSnapshot = nullptr;
	teclasTempoReal = nullptr;
}

void RawInput::ProcessRawInput(const RAWINPUT * const rawInputData)
{
	//se não tenho nada ou não pertençe ao rato
	if ((rawInputData == nullptr) || (rawInputData->header.dwType != RIM_TYPEMOUSE))
		return;

	//preciso de acesso exclusivo ao dados do rato
	EnterCriticalSection(&criticalSection);

	//se for o rato
	if (rawInputData->header.dwType == RIM_TYPEMOUSE) 
	{
		ratoPosAccum[0] += rawInputData->data.mouse.lLastX;
		ratoPosAccum[1] += rawInputData->data.mouse.lLastY;
	}

	//já não preciso deste acesso
	LeaveCriticalSection(&criticalSection);
}

void RawInput::ProcessKey(bool keyDown, WPARAM wParam, LPARAM lParam)
{
	int virtualKeyCode;

	//tiro o código e se não estiver no intervalo correcto, saio
	virtualKeyCode = wParam;
	if (virtualKeyCode >= numMaxTeclas)
		return;

	//preciso de acesso exclusivo ao dados do rato
	EnterCriticalSection(&criticalSection);

	//basta gravar isto
	teclasTempoReal[virtualKeyCode] = keyDown;

	//já não preciso deste acesso
	LeaveCriticalSection(&criticalSection);
}

void RawInput::Snapshot()
{
	//preciso de acesso exclusivo ao dados do rato e teclado
	EnterCriticalSection(&criticalSection);

	//para o caso do rato, basta copiar os valores e reiniciar o acumulado
	ratoSnapshot[0] = ratoPosAccum[0];
	ratoSnapshot[1] = ratoPosAccum[1];
	ratoSnapshot[2] = ratoPosAccum[2];
	ratoPosAccum[0] = ratoPosAccum[1] = ratoPosAccum[2] = 0.0f;

	//copio as teclas que tenho (em tempo real) para o snapshot
	memcpy(teclasSnapshot, teclasTempoReal, sizeof(bool) * numMaxTeclas);

	//já não preciso deste acesso
	LeaveCriticalSection(&criticalSection);
}

//devolve TRUE se a tecla VCODE estiver presionada
bool RawInput::KStatus(const int &vcode) const	
{
	if (vcode >= numMaxTeclas)
		return false;
	return teclasSnapshot[vcode];
}

//devolve a posição relativa do rato no eixo X
float RawInput::MStatusPosX() const
{
	return ratoSnapshot[0];
}

//devolve a posição relativa do rato no eixo Y
float RawInput::MStatusPosY() const
{
	return ratoSnapshot[1];
}

//devolve a posição relativa do rato no eixo Z (scroll wheel)
float RawInput::MStatusPosZ() const
{
	return ratoSnapshot[2];
}