#pragma once

#include "../platform/platform.hpp"
#include "math.hpp"
#include "types.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <filesystem>
#include <functional>
#include <memory>
#include <span>
#include <type_traits>

#include <windows.h>

namespace hr::streams
{
    class MemoryStream;
    class MemoryViewStream;
    class FileStream;

    class Stream
    {
    public:
        enum class SeekOrigin
        {
            Begin,
            Current,
            End
        };

        Stream() = default;
        virtual ~Stream() = default;

        virtual void close() = 0;
        virtual void flush() = 0;

        virtual bool canRead() const = 0;
        virtual bool canRead(size_t numBytes) const = 0;
        virtual bool canWrite() const = 0;
        virtual bool canWrite(size_t numBytes) const = 0;
        virtual size_t length() const = 0;
        virtual size_t position() const = 0;

        virtual size_t read(void* const outBuffer, size_t numBytes) = 0;
        virtual size_t write(const void* const inBuffer, size_t numBytes) = 0;
        virtual bool seek(SeekOrigin seekOrigin, int offset) = 0;

        virtual bool cloneAllContent(MemoryViewStream& memView) const = 0;
        virtual bool cloneAllContent(std::shared_ptr<unsigned char>& buffer,
                                     size_t& bufferSize,
                                     std::function<std::shared_ptr<unsigned char>(size_t)> allocatorFunc = nullptr) const = 0;
    };

    class NullStream final: public Stream
    {
    public:
        void close() override
        {}
        void flush() override
        {}

        bool canRead() const override
        {
            return false;
        }
        bool canRead(size_t) const override
        {
            return false;
        }
        bool canWrite() const override
        {
            return false;
        }
        bool canWrite(size_t) const override
        {
            return false;
        }
        size_t length() const override
        {
            return 0;
        }
        size_t position() const override
        {
            return 0;
        }

        size_t read(void* const, size_t) override
        {
            return 0;
        }
        size_t write(const void* const, size_t) override
        {
            return 0;
        }
        bool seek(SeekOrigin, int) override
        {
            return false;
        }

        bool cloneAllContent(MemoryViewStream&) const override
        {
            return false;
        }
        bool cloneAllContent(std::shared_ptr<unsigned char>&, size_t&, std::function<std::shared_ptr<unsigned char>(size_t)> = nullptr) const override
        {
            return false;
        }
    };

    class MemoryStream final: public Stream
    {
        void* mData = nullptr;
        unsigned char *mDataBegin = nullptr, *mDataEnd = nullptr, *mDataWalker = nullptr;
        size_t mDataSize = 0;
        bool mIsClosed = false;

    public:
        explicit MemoryStream(size_t reserveSize = 1024)
        {
            mDataSize = (reserveSize < 1024) ? 1024 : reserveSize;
            mData = std::realloc(mData, mDataSize);

            mDataBegin = mDataWalker = reinterpret_cast<unsigned char*>(mData);
            mDataEnd = mDataBegin + mDataSize;
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
                std::swap(mDataBegin, stream.mDataBegin);
                std::swap(mDataEnd, stream.mDataEnd);
                std::swap(mDataWalker, stream.mDataWalker);
                std::swap(mDataSize, stream.mDataSize);
                std::swap(mIsClosed, stream.mIsClosed);
            }

            return *this;
        }

        void close() override;
        void flush() override;

        bool canRead() const override;
        bool canRead(size_t numBytes) const override;
        bool canWrite() const override;
        bool canWrite(size_t numBytes) const override;
        size_t length() const override;
        size_t position() const override;

        size_t read(void* const outBuffer, size_t numBytes) override;
        size_t write(const void* const inBuffer, size_t numBytes) override;
        bool seek(SeekOrigin seekOrigin, int offset) override;

        bool cloneAllContent(MemoryViewStream& memView) const override;
        bool cloneAllContent(std::shared_ptr<unsigned char>& buffer,
                             size_t& bufferSize,
                             std::function<std::shared_ptr<unsigned char>(size_t)> allocatorFunc = nullptr) const override;

        const void* data() const;
        std::string toStr() const;
    };

    class MemoryViewStream final: public Stream
    {
        const void* mData = nullptr;
        const unsigned char *mDataBegin = nullptr, *mDataEnd = nullptr, *mDataWalker = nullptr;
        bool mIsClosed = false;
        std::shared_ptr<unsigned char> mDataShared;

    public:
        MemoryViewStream() = default;

        MemoryViewStream(std::shared_ptr<unsigned char> data, size_t dataSize)
          : MemoryViewStream(std::move(data), 0, dataSize)
        {}

        MemoryViewStream(std::shared_ptr<unsigned char> data, size_t dataOffset, size_t dataSize)
        {
            mDataShared = std::move(data);
            mData = mDataShared.get();
            mDataBegin = mDataWalker = reinterpret_cast<const unsigned char*>(mData) + dataOffset;
            mDataEnd = mDataBegin + dataSize;
        }

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
                std::swap(mDataBegin, stream.mDataBegin);
                std::swap(mDataEnd, stream.mDataEnd);
                std::swap(mDataWalker, stream.mDataWalker);
                std::swap(mDataShared, stream.mDataShared);
                std::swap(mIsClosed, stream.mIsClosed);
            }

            return *this;
        }

        void close() override;
        void flush() override;

        bool canRead() const override;
        bool canRead(size_t numBytes) const override;
        bool canWrite() const override;
        bool canWrite(size_t numBytes) const override;
        size_t length() const override;
        size_t position() const override;

        size_t read(void* const outBuffer, size_t numBytes) override;
        size_t write(const void* const inBuffer, size_t numBytes) override;
        bool seek(SeekOrigin seekOrigin, int offset) override;

        bool cloneAllContent(MemoryViewStream& memView) const override;
        bool cloneAllContent(std::shared_ptr<unsigned char>& buffer,
                             size_t& bufferSize,
                             std::function<std::shared_ptr<unsigned char>(size_t)> allocatorFunc = nullptr) const override;

        const void* data() const noexcept;

        template<typename TData>
        const TData* dataAs() const noexcept
        {
            return reinterpret_cast<const TData*>(data());
        }

        std::string toStr() const;
    };

    class FileStream final: public Stream
    {
        HANDLE mFileHandle = nullptr;
        bool mCanRead = false, mCanWrite = false, mClosed = false;

        bool openFile(const std::filesystem::path& filePath, bool toRead, bool toWrite);

    public:
        static std::filesystem::path createTmpFile(std::string_view baseName, std::string_view extension);

        static std::unique_ptr<MemoryViewStream> readEntireFile(const std::filesystem::path& filePath);
        static std::string readEntireFileAsString(const std::filesystem::path& filePath);
        static bool streamDump(Stream& stream, const std::filesystem::path& filePath);

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

        bool canRead() const override;
        bool canRead(size_t numBytes) const override;
        bool canWrite() const override;
        bool canWrite(size_t numBytes) const override;
        size_t length() const override;
        size_t position() const override;

        bool isValid() const;

        size_t read(void* const outBuffer, size_t numBytes) override;
        size_t write(const void* const inBuffer, size_t numBytes) override;
        bool seek(SeekOrigin seekOrigin, int offset) override;

        bool cloneAllContent(MemoryViewStream& memView) const override;
        bool cloneAllContent(std::shared_ptr<unsigned char>& buffer,
                             size_t& bufferSize,
                             std::function<std::shared_ptr<unsigned char>(size_t)> allocatorFunc = nullptr) const override;
    };

    class StreamReader final
    {
        Stream& mStream;

    public:
        explicit StreamReader(Stream& stream) noexcept
          : mStream(stream)
        {}

        Stream& stream() noexcept
        {
            return mStream;
        }

        const Stream& stream() const noexcept
        {
            return mStream;
        }

        template<typename T>
        bool read(T& destination)
        {
            static_assert(std::is_integral<T>::value || std::is_floating_point<T>::value, "Data type must be integer or floating-point");

            if (!mStream.canRead(sizeof(T)))
                return false;

            if (mStream.read(&destination, sizeof(T)) != sizeof(T))
                return false;

            return true;
        }

        template<typename T, size_t N>
        bool read(std::span<T, N> data)
        {
            if (!mStream.canRead(data.size_bytes()))
                return false;

            return (mStream.read(data.data(), data.size_bytes()) == data.size_bytes());
        }

        size_t read(void* const buffer, size_t numBytes)
        {
            return mStream.read(buffer, numBytes);
        }

        size_t position() const
        {
            return mStream.position();
        }

        bool canRead() const
        {
            return mStream.canRead();
        }

        bool skip(size_t offset)
        {
            return mStream.seek(Stream::SeekOrigin::Current, static_cast<int>(offset));
        }

        bool seek(Stream::SeekOrigin seekOrigin, int offset)
        {
            return mStream.seek(seekOrigin, offset);
        }
    };

    class StreamWriter final
    {
        Stream& mStream;

    public:
        explicit StreamWriter(Stream& stream) noexcept
          : mStream(stream)
        {}

        Stream& stream() noexcept
        {
            return mStream;
        }

        const Stream& stream() const noexcept
        {
            return mStream;
        }

        template<typename T>
        bool write(T value, std::enable_if_t<std::is_arithmetic<T>::value || std::is_enum<T>::value, bool> = true)
        {
            if (!mStream.canWrite())
                return false;

            return (mStream.write(&value, sizeof(T)) == sizeof(T));
        }

        template<typename T, size_t N>
        bool write(std::span<const T, N> data)
        {
            if (!mStream.canWrite())
                return 0;

            return (mStream.write(data.data(), data.size_bytes()) == data.size_bytes());
        }

        size_t write(const void* const buffer, const size_t numBytes)
        {
            if (!mStream.canWrite())
                return 0;

            return mStream.write(buffer, numBytes);
        }

        size_t write(StreamReader& reader, const size_t numBytes)
        {
            std::array<std::byte, 1024> tmpBuffer;

            size_t bytesRemaining = numBytes;
            size_t bytesTotalWritten = 0;

            while (bytesRemaining > 0)
            {
                auto bytesRead = reader.read(tmpBuffer.data(), std::min(bytesRemaining, tmpBuffer.size()));
                if (bytesRead <= 0)
                    break;

                auto bytesWriten = write(tmpBuffer.data(), bytesRead);

                bytesRemaining -= bytesRead;
                bytesTotalWritten += bytesWriten;

                if (bytesWriten != bytesRead)
                    break;
            }

            return bytesTotalWritten;
        }

        size_t writeString(std::string_view str)
        {
            if (str.empty() || !mStream.canWrite())
                return 0;

            return mStream.write(str.data(), str.size());
        }

        size_t position() const
        {
            return mStream.position();
        }

        bool skip(size_t offset)
        {
            return mStream.seek(Stream::SeekOrigin::Current, static_cast<int>(offset));
        }

        bool seek(Stream::SeekOrigin seekOrigin, int offset)
        {
            return mStream.seek(seekOrigin, offset);
        }
    };

    class TextWriter final
    {
        Stream& mStream;

    public:
        explicit TextWriter(Stream& stream) noexcept
          : mStream(stream)
        {}

        Stream& stream()
        {
            return mStream;
        }

        const Stream& stream() const
        {
            return mStream;
        }

        bool write(const char* const string, bool writeLine = false)
        {
            if (!string || (string[0] == '\0'))
                return 0;

            return write(string, strlen(string), writeLine);
        }

        bool write(const char* const string, size_t bytesToWrite, bool writeLine = false)
        {
            if (!string || (bytesToWrite <= 0) || !mStream.canWrite())
                return 0;

            auto success = (mStream.write(string, bytesToWrite) == bytesToWrite);

            if (success && writeLine)
                success &= (mStream.write(hr::platform::Platform::NewLine, hr::platform::Platform::NewLineSize) == hr::platform::Platform::NewLineSize);

            return success;
        }

        void writeLine()
        {
            mStream.write(hr::platform::Platform::NewLine, hr::platform::Platform::NewLineSize);
        }
    };
}
