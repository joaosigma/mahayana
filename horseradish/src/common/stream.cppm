export module core:stream;

import std;

namespace hr::streams
{
    export
    class MemoryStream;
    export
    class MemoryViewStream;
    export
    class FileStream;

    /// <summary>
    /// Interface for all known streams.
    /// </summary>
    export class Stream
    {
    public:
        enum class SeekOrigin
        {
            Begin,
            Current,
            End
        };

        Stream() = default;
        virtual ~Stream() noexcept = default;

        virtual void close() = 0;
        virtual void flush() = 0;

        [[nodiscard]] virtual bool canRead() const = 0;
        [[nodiscard]] virtual bool canRead(size_t numBytes) const = 0;
        [[nodiscard]] virtual bool canWrite() const = 0;
        [[nodiscard]] virtual bool canWrite(size_t numBytes) const = 0;
        [[nodiscard]] virtual size_t length() const = 0;
        [[nodiscard]] virtual size_t position() const = 0;

        [[nodiscard]] virtual size_t read(std::span<std::byte> dst) = 0;
        [[nodiscard]] virtual size_t write(std::span<const std::byte> src) = 0;
        [[nodiscard]] virtual bool seek(SeekOrigin seekOrigin, int offset) = 0;

        [[nodiscard]] virtual bool cloneAllContent(const std::function<std::span<std::byte>(size_t)>& targetBuffer) const = 0;
    };

    /// <summary>
    /// Should work similar to /dev/null.
    /// </summary>
    export class NullStream final: public Stream
    {
    public:
        void close() override
        {}
        void flush() override
        {}

        bool canRead() const override
        {
            return true;
        }
        bool canRead(size_t) const override
        {
            return true;
        }
        bool canWrite() const override
        {
            return true;
        }
        bool canWrite(size_t) const override
        {
            return true;
        }
        size_t length() const override
        {
            return 0;
        }
        size_t position() const override
        {
            return 0;
        }

        size_t read(std::span<std::byte> dst) override
        {
            std::ranges::fill(dst, std::byte{10});
            return dst.size();
        }
        size_t write(std::span<const std::byte> src) override
        {
            return src.size();
        }
        bool seek(SeekOrigin, int) override
        {
            return true;
        }

        bool cloneAllContent(const std::function<std::span<std::byte>(size_t)>&) const override
        {
            return true;
        }
    };

    /// <summary>
    /// A streamed backed by an internal buffer that grows as necessary.
    /// </summary>
    export class MemoryStream final: public Stream
    {
        std::vector<std::byte> mData;
        size_t mPos = 0;
        bool mIsClosed = false;

    public:
        explicit MemoryStream(size_t reserveSize = 1024)
        {
            mData.reserve((reserveSize < 1024) ? 1024 : reserveSize);
        }

        ~MemoryStream() noexcept
        {
            close();
        }

        MemoryStream(const MemoryStream&) = delete;
        const MemoryStream& operator=(const MemoryStream&) = delete;

        MemoryStream(MemoryStream&& stream) noexcept
        {
            *this = std::move(stream);
        }

        MemoryStream& operator=(MemoryStream&& stream) noexcept
        {
            if (this != &stream)
            {
                std::swap(mData, stream.mData);
                std::swap(mPos, stream.mPos);
            }

            return *this;
        }

        void close() override;
        void flush() override
        {}

        [[nodiscard]] bool canRead() const override;
        [[nodiscard]] bool canRead(size_t numBytes) const override;
        [[nodiscard]] bool canWrite() const override;
        [[nodiscard]] bool canWrite(size_t numBytes) const override;
        [[nodiscard]] size_t length() const override;
        [[nodiscard]] size_t position() const override;

        [[nodiscard]] size_t read(std::span<std::byte> dst) override;
        [[nodiscard]] size_t write(std::span<const std::byte> src) override;
        [[nodiscard]] bool seek(SeekOrigin seekOrigin, int offset) override;

        [[nodiscard]] bool cloneAllContent(const std::function<std::span<std::byte>(size_t)>& targetBuffer) const override;

        void reset() noexcept;
        void truncate(size_t newSize);

        template<typename Self>
        [[nodiscard]] auto data(this Self&& self) noexcept
        {
            return std::span{self.mData.data(), self.length()};
        }
    };

    /// <summary>
    /// A stream backed by a read-only, non-ownable external buffer.
    /// </summary>
    export class MemoryViewStream final: public Stream
    {
        std::span<std::byte> mData;
        size_t mPos = 0;
        bool mIsClosed = false;

    public:
        MemoryViewStream() = default;

        MemoryViewStream(std::span<std::byte> data)
          : mData{data}
        {}

        MemoryViewStream(const MemoryViewStream&) = delete;
        const MemoryViewStream& operator=(const MemoryViewStream&) = delete;

        MemoryViewStream(MemoryViewStream&& stream) noexcept
        {
            *this = std::move(stream);
        }

        MemoryViewStream& operator=(MemoryViewStream&& stream) noexcept
        {
            if (this != &stream)
            {
                std::swap(mData, stream.mData);
                std::swap(mPos, stream.mPos);
                std::swap(mIsClosed, stream.mIsClosed);
            }

            return *this;
        }

        void close() override;
        void flush() override
        {}

        [[nodiscard]] bool canRead() const override;
        [[nodiscard]] bool canRead(size_t numBytes) const override;
        [[nodiscard]] bool canWrite() const override;
        [[nodiscard]] bool canWrite(size_t numBytes) const override;
        [[nodiscard]] size_t length() const override;
        [[nodiscard]] size_t position() const override;

        [[nodiscard]] size_t read(std::span<std::byte> dst) override;
        [[nodiscard]] size_t write(std::span<const std::byte> src) override;
        [[nodiscard]] bool seek(SeekOrigin seekOrigin, int offset) override;

        [[nodiscard]] bool cloneAllContent(const std::function<std::span<std::byte>(size_t)>& targetBuffer) const override;

        template<typename Self>
        [[nodiscard]] auto data(this Self&& self) noexcept
        {
            return self.mData;
        }
    };

    /// <summary>
    /// A regular file stream using blocking system calls.
    /// </summary>
    export class FileStream final: public Stream
    {
        void* mFileHandle = nullptr;
        bool mCanRead = false, mCanWrite = false, mClosed = false;

        bool openFile(const std::filesystem::path& filePath, bool toRead, bool toWrite);

    public:
        [[nodiscard]] static std::filesystem::path createTmpFile(std::string_view baseName, std::string_view extension);

        [[nodiscard]] static std::optional<MemoryStream> readEntireFile(const std::filesystem::path& filePath);
        [[nodiscard]] static std::optional<std::string> readEntireFileAsString(const std::filesystem::path& filePath);
        [[nodiscard]] static bool streamFullDump(Stream& stream, const std::filesystem::path& filePath);

    public:
        FileStream() = default;

        FileStream(const std::filesystem::path& filePath, bool toRead, bool toWrite)
        {
            openFile(filePath, toRead, toWrite);
        }

        ~FileStream()
        {
            if (!mClosed)
                close();
        }

        FileStream(const FileStream&) = delete;
        const FileStream& operator=(const FileStream&) = delete;

        FileStream(FileStream&& stream) noexcept
        {
            *this = std::move(stream);
        }

        FileStream& operator=(FileStream&& stream) noexcept
        {
            if (this != &stream)
            {
                std::swap(mFileHandle, stream.mFileHandle);
                std::swap(mCanRead, stream.mCanRead);
                std::swap(mCanWrite, stream.mCanWrite);
                std::swap(mClosed, stream.mClosed);
            }

            return *this;
        }

        void close() override;
        void flush() override;

        [[nodiscard]] bool canRead() const override;
        [[nodiscard]] bool canRead(size_t numBytes) const override;
        [[nodiscard]] bool canWrite() const override;
        [[nodiscard]] bool canWrite(size_t numBytes) const override;
        [[nodiscard]] size_t length() const override;
        [[nodiscard]] size_t position() const override;

        [[nodiscard]] bool isValid() const;

        [[nodiscard]] size_t read(std::span<std::byte> dst) override;
        [[nodiscard]] size_t write(std::span<const std::byte> src) override;
        [[nodiscard]] bool seek(SeekOrigin seekOrigin, int offset) override;

        [[nodiscard]] bool cloneAllContent(const std::function<std::span<std::byte>(size_t)>& targetBuffer) const override;
    };

    export class StreamReader final
    {
        Stream& mStream;

    public:
        explicit StreamReader(Stream& stream) noexcept
          : mStream(stream)
        {}

        template<typename Self>
        auto& stream(this Self&& self) noexcept
        {
            return self.mStream;
        }

        template<typename T, size_t N>
        [[nodiscard]] bool read(std::span<T, N> dst)
        {
            if (!mStream.canRead(dst.size_bytes()))
                return false;

            auto bytes = std::as_writable_bytes(dst);
            return (mStream.read(bytes) == bytes.size_bytes());
        }

        template<typename T>
        [[nodiscard]] bool read(std::span<T> dst)
        {
            if (!mStream.canRead(dst.size_bytes()))
                return false;

            auto bytes = std::as_writable_bytes(dst);
            return (mStream.read(bytes) == bytes.size_bytes());
        }

        template<typename T>
        [[nodiscard]] bool read(T& dst)
        {
            if (!mStream.canRead(sizeof(T)))
                return false;

            if (mStream.read({reinterpret_cast<std::byte*>(&dst), sizeof(T)}) != sizeof(T))
                return false;

            return true;
        }

        [[nodiscard]] size_t position() const
        {
            return mStream.position();
        }

        [[nodiscard]] bool canRead() const
        {
            return mStream.canRead();
        }

        [[nodiscard]] bool skip(size_t offset)
        {
            return mStream.seek(Stream::SeekOrigin::Current, static_cast<int>(offset));
        }

        [[nodiscard]] bool seek(Stream::SeekOrigin seekOrigin, int offset)
        {
            return mStream.seek(seekOrigin, offset);
        }
    };

    export class StreamWriter final
    {
        Stream& mStream;

    public:
        explicit StreamWriter(Stream& stream) noexcept
          : mStream(stream)
        {}

        template<typename Self>
        [[nodiscard]] auto& stream(this Self&& self) noexcept
        {
            return self.mStream;
        }

        template<typename T>
            requires std::is_arithmetic_v<T> || std::is_enum_v<T>
        [[nodiscard]] bool write(T value)
        {
            if (!mStream.canWrite(sizeof(T)))
                return false;

            return (mStream.write({reinterpret_cast<const std::byte*>(&value), sizeof(T)}) == sizeof(T));
        }

        [[nodiscard]] bool write(std::string_view str)
        {
            if (!mStream.canWrite(str.size()))
                return false;

            return (mStream.write({reinterpret_cast<const std::byte*>(str.data()), str.size()}) == str.size());
        }

        template<typename T, size_t N>
        [[nodiscard]] bool write(std::span<const T, N> src)
        {
            auto dataBytes = std::as_bytes(src);

            if (!mStream.canWrite(dataBytes.size()))
                return false;

            return (mStream.write(dataBytes) == dataBytes.size());
        }

        template<typename T>
        [[nodiscard]] bool write(std::span<const T> src)
        {
            auto dataBytes = std::as_bytes(src);

            if (!mStream.canWrite(dataBytes.size()))
                return false;

            return (mStream.write(dataBytes) == dataBytes.size());
        }

        template<typename T>
        [[nodiscard]] bool writeObject(const T& obj)
        {
            if (!mStream.canWrite(sizeof(T)))
                return false;

            return (mStream.write(std::span{reinterpret_cast<const std::byte*>(&obj), sizeof(T)}) == sizeof(T));
        }

        [[nodiscard]] size_t write(StreamReader& reader, size_t numBytes)
        {
            std::array<std::byte, 1024> tmpBuffer;

            size_t numBytesWritten = 0;
            while (numBytes > 0)
            {
                std::span<std::byte> dst{tmpBuffer.data(), std::min(numBytes, tmpBuffer.size())};

                if (!reader.read(dst))
                    break;

                if (!mStream.canWrite(dst.size()))
                    return numBytesWritten;

                auto bytesWriten = mStream.write(dst);

                numBytes -= dst.size();
                numBytesWritten += bytesWriten;

                if (bytesWriten != dst.size())
                    break;
            }

            return numBytesWritten;
        }

        [[nodiscard]] size_t position() const
        {
            return mStream.position();
        }

        [[nodiscard]] bool skip(size_t offset)
        {
            return mStream.seek(Stream::SeekOrigin::Current, static_cast<int>(offset));
        }

        [[nodiscard]] bool seek(Stream::SeekOrigin seekOrigin, int offset)
        {
            return mStream.seek(seekOrigin, offset);
        }
    };

    export class TextWriter final
    {
        Stream& mStream;

    public:
        explicit TextWriter(Stream& stream) noexcept
          : mStream(stream)
        {}

        template<typename Self>
        [[nodiscard]] auto& stream(this Self&& self) noexcept
        {
            return self.mStream;
        }

        [[nodiscard]] bool write(std::string_view str, bool writeLine = false)
        {
            auto success = str.empty() ? true : (mStream.write({reinterpret_cast<const std::byte*>(str.data()), str.size()}) == str.size());
            if (success && writeLine)
                success &= writeNewLine();

            return success;
        }

        [[nodiscard]] bool writeNewLine();
    };
}
