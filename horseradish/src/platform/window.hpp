#pragma once

#include "../engine/logger.hpp"
#include "../common/vector.hpp"
#include "../common/opengl/objects.hpp"

namespace hr::platform
{
	class WindowImpl;
	class OpenglContextImpl;

	class Window
	{
		friend class OpenglContext;

	public:
		enum class WindowStyle { StyleFullscreen, StyleFullscreenWindow, StyleWindow };
		enum class VirtualKeys {
			Invalid, MouseLButton, MouseRButton, MouseMButton, Tab, Backspace, Return, Shift, Control, Escape, Space, PageUp, PageDown, End, Home, Left, Right, Up, Down, Delete,
			F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12

			//0-9 correspond to ASCII '0'-'9' and A-Z correspond to ASCII 'A'-'Z'
		}; 

		class Message
		{
		public:
			enum class MessageType
			{ 
				Void,
				CharacterKey, //*params*: the Unicode of the caracter typed
							  //*flags*: short0 is the repeat count while short1 is the bitmask of MessageFlags values
				VirtualKey, //*params*: the virtual key code
							//*flags*: short0 is the repeat count while short1 is the bitmask of MessageFlags values
				MouseWheel, //*params*: X coordinate in the low 16 bits and the Y coordinate in the high 16 bits (origin is upper-left)
							//*flags*: short0 is signed-short representing the amount the distance the wheel rotates while short1 is the bitmask of MessageFlags values
				Resize //*params*: X coordinate in the low 16 bits and the Y coordinate in the high 16 bits (origin is upper-left)
					   //*flags*: unused
			};
			enum class MessageFlags { ControlKey = (1 << 0), ShiftKey = (1 << 1) };

		private:
			MessageType mType{ MessageType::Void };
			hr::hInt32 mParam{ 0 };
			hr::hSplitUInt32 mFlags{ 0 };

		public:
			Message() = default;

			Message(MessageType msgType)
				: mType{ msgType }
			{ }

			Message(MessageType msgType, hr::hInt32 msgParam)
				: mType{ msgType }, mParam{ msgParam }
			{ }

			Message(MessageType msgType, hr::hInt32 msgParam, hr::hSplitUInt32 msgFlags)
				: mType{ msgType }, mParam{ msgParam }, mFlags{ msgFlags }
			{ }
			
			MessageType getType() const
			{
				return mType;
			}

			bool isType(MessageType msgType) const
			{
				return (this->mType == msgType);
			}

			hr::hInt32 getParam() const
			{
				return mParam;
			}

			hr::hSplitUInt32 getFlags() const
			{
				return mFlags;
			}
		};

	private:
		std::unique_ptr<WindowImpl> mImpl;

	public:
		static void MsgBoxInfo(std::string_view msg);
		static void MsgBoxWarn(std::string_view msg);
		static void MsgBoxError(std::string_view msg);

	public:
		Window(hr::engine::Logger &logger);
		~Window();

		std::string getErrorMsg() const;

		bool windowInit(std::string_view windowTitle, WindowStyle style, bool targetSecondaryDisplay, const size_t targetWidth, const size_t targeHeight);

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
		void processMessages(const std::function<void(const Message&)>& cb, const bool resetQueue);
	};

	class OpenglContext
		: public hr::gl::objects::Context
	{
		bool mIsValid{ false };
		std::unique_ptr<OpenglContextImpl> mImpl;

	public:
		OpenglContext(const Window &window, std::string_view openGLModuleName, int contextMajorVersion, int contextMinorVersion, bool contextDebug, bool contextForwardCompatible);

		bool isValid() const;
		std::string getErrorMsg() const;

		void setSwapInterval(size_t interval) const;
		bool swapBuffers(void) const;
	};
}
