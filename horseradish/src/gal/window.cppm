export module gal:window;

import std;

import core;
import :objects;
import :openglctx;

namespace hr::renderPlatform
{
    class WindowImpl;
    class OpenglContextImpl;

    export class Window
    {
        friend class OpenglContext;

    public:
        enum class WindowStyle
        {
            StyleFullscreen,
            StyleFullscreenWindow,
            StyleWindow
        };
        enum class VirtualKeys
        {
            Invalid,
            MouseLButton,
            MouseRButton,
            MouseMButton,
            Tab,
            Backspace,
            Return,
            Shift,
            Control,
            Escape,
            Space,
            PageUp,
            PageDown,
            End,
            Home,
            Left,
            Right,
            Up,
            Down,
            Delete,
            F1,
            F2,
            F3,
            F4,
            F5,
            F6,
            F7,
            F8,
            F9,
            F10,
            F11,
            F12

            // 0-9 correspond to ASCII '0'-'9' and A-Z correspond to ASCII 'A'-'Z'
        };

        class Message
        {
        public:
            enum class MessageType
            {
                Void,
                CharacterKey, //*params*: the Unicode of the caracter typed
                              //*flags*: short0 is the repeat count while short1 is the bitmask of MessageFlags values
                VirtualKey,   //*params*: the virtual key code
                              //*flags*: short0 is the repeat count while short1 is the bitmask of MessageFlags values
                MouseWheel,   //*params*: X coordinate in the low 16 bits and the Y coordinate in the high 16 bits (origin is upper-left)
                              //*flags*: short0 is signed-short representing the amount the distance the wheel rotates while short1 is the bitmask of MessageFlags values
                Resize        //*params*: X coordinate in the low 16 bits and the Y coordinate in the high 16 bits (origin is upper-left)
                              //*flags*: unused
            };
            enum class MessageFlags
            {
                ControlKey = (1 << 0),
                ShiftKey = (1 << 1)
            };

        private:
            MessageType mType{MessageType::Void};
            std::int32_t mParam{0};
            hr::types::hSplitUInt32 mFlags{0};

        public:
            Message() = default;

            Message(MessageType msgType) noexcept
              : mType{msgType}
            {}

            Message(MessageType msgType, int32_t msgParam) noexcept
              : mType{msgType}, mParam{msgParam}
            {}

            Message(MessageType msgType, int32_t msgParam, hr::types::hSplitUInt32 msgFlags) noexcept
              : mType{msgType}, mParam{msgParam}, mFlags{msgFlags}
            {}

            MessageType getType() const noexcept
            {
                return mType;
            }

            bool isType(MessageType msgType) const noexcept
            {
                return (this->mType == msgType);
            }

            int32_t getParam() const noexcept
            {
                return mParam;
            }

            hr::types::hSplitUInt32 getFlags() const noexcept
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
        Window(hr::engine::Logger& logger);
        ~Window() noexcept;

        std::string_view getErrorMsg() const;

        bool windowInit(std::string_view windowTitle, WindowStyle style, bool targetSecondaryDisplay, const size_t targetWidth, const size_t targeHeight);

        size_t getDisplayWidth() const;
        size_t getDisplayHeight() const;

        bool setWindowAlpha(const unsigned char& valorAlpha) const;
        bool sendMessageClose() const;
        bool setFocus() const;

        void rawInputSnapshot();
        bool rawInputGetKeyStatus(const unsigned int& vcode);
        bool rawInputGetKeyStatus(const Window::VirtualKeys& vcode);
        hr::Vector3f rawInputGetMouseStatus();

        int messageLoop(const std::function<void()>& closingCb);
        void processMessages(const std::function<void(const Message&)>& cb, const bool resetQueue);
    };

    export class OpenglContext: public hr::gl::objects::Context
    {
        bool mIsValid{false};
        std::unique_ptr<OpenglContextImpl> mImpl;

    public:
        OpenglContext(const Window& window, std::string_view openGLModuleName, int contextMajorVersion, int contextMinorVersion, bool contextDebug, bool contextForwardCompatible);
        ~OpenglContext() noexcept;

        bool isValid() const;
        std::string_view getErrorMsg() const;

        void setSwapInterval(size_t interval) const;
        bool swapBuffers(void) const;
    };
}
