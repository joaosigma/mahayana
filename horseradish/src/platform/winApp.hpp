#pragma once

#include "common\Platform.hpp"
#include "common\String.hpp"
#include "common\opengl\objects.hpp"

#include "wglext.h"

#include <mutex>

#include <windows.h>

class Window;
class OpenglContext;
class RawInput;

class Window
{
	HWND hWnd;
	HINSTANCE hInstance;
	DEVMODE originalDeviceMode;
	wchar_t className[128];
	bool isFullscreen, isInitialized;
	unsigned int winWidth, winHeight;

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
	void PeekMessageAndDispatch() const;

	static void MsgBoxInfo(const HorseRadish::String &msg);
	static void MsgBoxInfo(const char * const msg);
	static void MsgBoxWarn(const HorseRadish::String &msg);
	static void MsgBoxWarn(const char * const msg);
	static void MsgBoxError(const HorseRadish::String &msg);
	static void MsgBoxError(const char * const msg);

	static bool CommandLineHasParam(PWSTR cmdLine, const HorseRadish::hChar * const parameterName);
	static bool CommandLineGetParam(PWSTR cmdLine, const HorseRadish::hChar * const parameterName, HorseRadish::String &parameterValue);
	static bool CommandLineGetParam(PWSTR cmdLine, const HorseRadish::hChar * const parameterName, int &parameterValue);
};

class OpenglContext : public HorseRadish::OpenGL::Objects::Context
{
	HDC hDC;
	HGLRC hRC;
	const Window *window;
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
	OpenglContext(const Window * const window, const HorseRadish::hChar *openGLModuleName, int contextMajorVersion, int contextMinorVersion, bool contextDebug, bool contextForwardCompatible);
	~OpenglContext();

	void SetSwapInterval(const unsigned int &interval) const;
	bool SwapBuffers(void) const;
};

class RawInput
{
	int numMaxKeyStrokes;
	std::mutex mutex;
	float mouseSnapshot[3], mousePosAccum[3];
	bool *keysSnapshot, *keysRealtime;

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
