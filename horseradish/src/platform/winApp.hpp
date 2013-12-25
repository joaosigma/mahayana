#pragma once
#ifndef __WINSYS__
#define __WINSYS__

#include "common\Platform.hpp"
#include "common\String.hpp"
#include "common\FileSystem.hpp"
#include "common\opengl\objects.hpp"

#include <windows.h>

//declarações
class Window;
class OpenglContext;
class RawInput;

//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§   Classe Window	§§§§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
class Window
{
	HWND hWnd;
	HINSTANCE hInstance;
	DEVMODE originalDeviceMode;
	wchar_t className[128];
	bool isFullscreen, isInitialized;
	unsigned int winWidth, winHeight;

	//sou amigo deste pessoal
	friend class OpenglContext;
	friend class RawInput;
	
public:
	Window(HINSTANCE hInst);
	~Window();

	bool WindowInit(WNDPROC ProcFunc, const HorseRadish::hChar *windowTitle, const unsigned int winWidth, const unsigned int winHeight, const bool winFullscreen);
	bool WindowEditorInit(WNDPROC ProcFunc, const HorseRadish::hChar *windowTitle, const unsigned int winWidth, const unsigned int winHeight, const HWND handleWindowParent);
	void WindowKill(void);

	bool SetWindowAlpha(const unsigned char &valorAlpha) const;
	bool SendMessageClose() const;
	bool SetFocus() const;
	void PeekMessageDispatch(bool translateMessage = true) const;

	static void MsgBoxInfo(const HorseRadish::String &msg);
	static void MsgBoxInfo(const char * const msg);
	static void MsgBoxAviso(const HorseRadish::String &msg);
	static void MsgBoxAviso(const char * const msg);
	static void MsgBoxErro(const HorseRadish::String &msg);
	static void MsgBoxErro(const char * const msg);

	static bool CommandLineHasParam(PWSTR cmdLine, const HorseRadish::hChar * const parameterName);
	static bool CommandLineGetParam(PWSTR cmdLine, const HorseRadish::hChar * const parameterName, HorseRadish::String &parameterValue);
	static bool CommandLineGetParam(PWSTR cmdLine, const HorseRadish::hChar * const parameterName, int &parameterValue);
};

//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§   Classe OpenglContext	§§§§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
class OpenglContext : public HorseRadish::OpenGL::Objects::Context
{
	HDC hDC;
	HGLRC hRC;
	const Window *window;
	unsigned int usedPFD;

	//estas funções estão sempre presentes no módulo de OpenGL
	HGLRC (APIENTRY *wglCreateContext)			(HDC hdc);
	BOOL  (APIENTRY *wglMakeCurrent)			(HDC hdc, HGLRC hglrc);
	BOOL  (APIENTRY *wglDeleteContext)			(HGLRC hglrc);
	BOOL  (APIENTRY *wglSwapBuffers)			(HDC hdc);

	//estas são as extensões de WGL suportadas
	// WGL_ARB_create_context
	typedef HGLRC (APIENTRY *PFNWGLCREATECONTEXTATTRIBSARBPROC)(HDC hDC, HGLRC hShareContext, const int *attribList);
	PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB;
	// WGL_ARB_extensions_string
	typedef const char * (APIENTRYP PFNWGLGETEXTENSIONSSTRINGARBPROC) (HDC hdc);
	PFNWGLGETEXTENSIONSSTRINGARBPROC wglGetExtensionsStringARB;
	// WGL_ARB_pixel_format
	typedef BOOL (APIENTRYP PFNWGLGETPIXELFORMATATTRIBIVARBPROC)(HDC hdc, int iPixelFormat, int iLayerPlane, UINT nAttributes, const int *piAttributes, int *piValues);
	typedef BOOL (APIENTRYP PFNWGLGETPIXELFORMATATTRIBFVARBPROC)(HDC hdc, int iPixelFormat, int iLayerPlane, UINT nAttributes, const int *piAttributes, FLOAT *pfValues);
	typedef BOOL (APIENTRYP PFNWGLCHOOSEPIXELFORMATARBPROC)(HDC hdc, const int *piAttribIList, const FLOAT *pfAttribFList, UINT nMaxFormats, int *piFormats, UINT *nNumFormats);
	PFNWGLGETPIXELFORMATATTRIBIVARBPROC wglGetPixelFormatAttribivARB;
	PFNWGLGETPIXELFORMATATTRIBFVARBPROC wglGetPixelFormatAttribfvARB;
	PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB;
	// WGL_EXT_swap_control
	typedef BOOL (APIENTRYP PFNWGLSWAPINTERVALEXTPROC)(int interval);
	typedef int (APIENTRYP PFNWGLGETSWAPINTERVALEXTPROC)(void);
	PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT;
	PFNWGLGETSWAPINTERVALEXTPROC wglGetSwapIntervalEXT;
	
	void loadWGLFunctions(HMODULE openglModule);
	bool auxWindowWGLExt(HINSTANCE hInstance, HMODULE openglModule);
public:
	OpenglContext(const Window * const window, const HorseRadish::hChar *openGLModuleName, int contextMajorVersion, int contextMinorVersion, bool contextDebug, bool contextForwardCompatible);
	~OpenglContext();

	bool TakeScreenshot(HorseRadish::Streams::FileStream &fileStream) const;
	void SetSwapInterval(const unsigned int &interval) const;
	bool SwapBuffers(void) const;
};

//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§   Classe RawInput	§§§§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
class RawInput
{
	int numMaxTeclas;
	CRITICAL_SECTION criticalSection;
	float ratoSnapshot[3], ratoPosAccum[3];
	bool *teclasSnapshot, *teclasTempoReal;	

public:
	RawInput(const Window * const window);
	~RawInput();

	void ProcessRawInput(const RAWINPUT * const rawInputData);
	void ProcessKey(bool keyDown, WPARAM wParam, LPARAM lParam);
	void Snapshot();

	bool  KStatus(const int &vcode) const;
	float MStatusPosX() const;
	float MStatusPosY() const;
	float MStatusPosZ() const;
};

#endif