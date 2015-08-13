#pragma once

#include "..\engine\logger.hpp"
#include "common\Vector.hpp"
#include "common\opengl\objects.hpp"

class WindowImpl;
class OpenglContextImpl;

class Window
{
	friend class OpenglContext;

public:
	enum class VirtualKeys {
		Invalid, MouseLButton, MouseRButton, MouseMButton, Tab, Backspace, Return, Shift, Control, Escape, Space, PageUp, PageDown, End, Home, Left, Right, Up, Down, Delete,
		F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12 }; //0-9 correspond to ASCII '0'-'9' and A-Z correspond to ASCII 'A'-'Z'

	class Message {

	public:
		enum class MessageType
		{ 
			Void,
			CharacterKey, //*params*: the Unicode of the caracter typed
						  //*flags*: short0 is the repeat count while short1 is the bitmask of MessageFlags values
			VirtualKey, //*params*: the virtual key code
					    //*flags*: short0 is the repeat count while short1 is the bitmask of MessageFlags values
			MouseWheel //*params*: X coordinate in the low 16 bits and the Y coordinate in the high 16 bits (origin is upper-left)
					   //*flags*: short0 is signed-short representing the amount the distance the wheel rotates while short1 is the bitmask of MessageFlags values
		};
		enum class MessageFlags { ControlKey = (1 << 0), ShiftKey = (1 << 1) };

	private:
		MessageType mType;
		HorseRadish::hInt32 mParam;
		HorseRadish::hSplitUInt32 mFlags;

	public:
		Message(MessageType msgType, HorseRadish::hInt32 msgParam, HorseRadish::hSplitUInt32 msgFlags)
			: mType(msgType), mParam(msgParam), mFlags(msgFlags)
		{ }

		Message()
			: Message(MessageType::Void, 0, 0)
		{ }

		Message(MessageType msgType)
			: Message(msgType, 0, 0)
		{ }

		Message(MessageType msgType, HorseRadish::hInt32 msgParam)
			: mType(msgType), mFlags(0), mParam(msgParam)
		{ }

		MessageType getType() const
		{
			return mType;
		}

		bool isType(MessageType msgType) const
		{
			return (this->mType == msgType);
		}

		HorseRadish::hInt32 getParam() const
		{
			return mParam;
		}

		HorseRadish::hSplitUInt32 getFlags() const
		{
			return mFlags;
		}
	};

private:
	std::unique_ptr<WindowImpl> mImpl;

public:
	Window(HorseRadish::Engine::Logger &logger);
	~Window();

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
	void ProcessMessages(std::function<void(const Message&)> cb, const bool resetQueue);

	static void MsgBoxInfo(const std::string& msg);
	static void MsgBoxInfo(const char * const msg);
	static void MsgBoxWarn(const std::string& msg);
	static void MsgBoxWarn(const char * const msg);
	static void MsgBoxError(const std::string& msg);
	static void MsgBoxError(const char * const msg);
};

class OpenglContext : public HorseRadish::OpenGL::Objects::Context
{
	bool mIsValid;
	std::unique_ptr<OpenglContextImpl> mImpl;

public:
	OpenglContext(const Window &window, const std::string& openGLModuleName, int contextMajorVersion, int contextMinorVersion, bool contextDebug, bool contextForwardCompatible);

	bool isValid() const;
	std::string getErrorMsg() const;

	void setSwapInterval(const unsigned int &interval) const;
	bool swapBuffers(void) const;
};
