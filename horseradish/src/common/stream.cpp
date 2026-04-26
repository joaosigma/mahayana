#include "stream.hpp"

#include "scopedAction.hpp"

#include <algorithm>
#include <cassert>
#include <optional>

#include <windows.h>

namespace hr::streams
{
    namespace
    {
        std::optional<size_t> calculateSeek(size_t currentPos, size_t size, Stream::SeekOrigin seekOrigin, int offset)
        {
            int newPos;
            switch (seekOrigin)
            {
                case Stream::SeekOrigin::Begin:
                    newPos = offset;
                    break;
                case Stream::SeekOrigin::End:
                    newPos = static_cast<int>(size) + offset;
                    break;
                case Stream::SeekOrigin::Current:
                    newPos = static_cast<int>(currentPos) + offset;
                default:
                    return {};
            };

            newPos = std::clamp(newPos, 0, static_cast<int>(size));
            return static_cast<size_t>(newPos);
        }
    }

    void MemoryStream::close()
    {
        mPos = 0;
        mData.clear();
        mData.shrink_to_fit();
    }

    bool MemoryStream::canRead() const
    {
        return !mIsClosed && (mPos < mData.size());
    }

    bool MemoryStream::canRead(size_t numBytes) const
    {
        return canRead() && (numBytes <= (mData.size() - mPos));
    }

    bool MemoryStream::canWrite() const
    {
        return !mIsClosed;
    }

    bool MemoryStream::canWrite(size_t) const
    {
        return canWrite();
    }

    size_t MemoryStream::length() const
    {
        return mData.size();
    }

    size_t MemoryStream::position() const
    {
        return mPos;
    }

    size_t MemoryStream::read(std::span<std::byte> dst)
    {
        if (!canRead())
            return 0;

        auto numBytes = std::min(dst.size(), mData.size() - mPos);
        if (numBytes <= 0) [[unlikely]]
            return 0;

        std::ranges::copy_n(mData.data() + mPos, numBytes, dst.data());
        mPos += numBytes;

        return numBytes;
    }

    size_t MemoryStream::write(std::span<const std::byte> src)
    {
        if (src.empty() || !canWrite(src.size()))
            return 0;

        if (src.size() > (mData.size() - mPos))
        {
            mData.resize(mData.size() + (src.size() - (mData.size() - mPos)));
        }

        std::ranges::copy_n(src.data(), src.size(), mData.data() + mPos);
        mPos += src.size();

        return src.size();
    }

    bool MemoryStream::seek(SeekOrigin seekOrigin, int offset)
    {
        if (mIsClosed)
            return false;

        auto res = calculateSeek(mPos, mData.size(), seekOrigin, offset);
        if (!res)
            return false;

        mPos = *res;
        return true;
    }

    bool MemoryStream::cloneAllContent(const std::function<std::span<std::byte>(size_t)>& targetBuffer) const
    {
        auto buffer = targetBuffer(mData.size());
        if (buffer.empty())
            return false;

        std::ranges::copy_n(mData.data(), std::min(buffer.size(), mData.size()), buffer.data());
        return true;
    }

    void MemoryStream::reset() noexcept
    {
        if (mIsClosed)
            return;

        mPos = 0;
        mData.clear();
        mData.shrink_to_fit();
    }

    void MemoryStream::truncate(size_t newSize)
    {
        if (mIsClosed)
            return;

        mData.resize(newSize);
        mData.shrink_to_fit();
        mPos = std::clamp(mPos, 0uz, newSize);
    }

    void MemoryViewStream::close()
    {
        mIsClosed = true;
    }

    bool MemoryViewStream::canRead() const
    {
        return !mIsClosed;
    }

    bool MemoryViewStream::canRead(size_t numBytes) const
    {
        return canRead() && (numBytes <= (mData.size() - mPos));
    }

    bool MemoryViewStream::canWrite() const
    {
        return false;
    }

    bool MemoryViewStream::canWrite(size_t) const
    {
        return canWrite();
    }

    size_t MemoryViewStream::length() const
    {
        return mData.size();
    }

    size_t MemoryViewStream::position() const
    {
        return mPos;
    }

    size_t MemoryViewStream::read(std::span<std::byte> dst)
    {
        if (!canRead())
            return 0;

        auto numBytes = std::min(dst.size(), mData.size() - mPos);
        if (numBytes <= 0) [[unlikely]]
            return 0;

        std::ranges::copy_n(mData.data() + mPos, numBytes, dst.data());
        mPos += numBytes;

        return numBytes;
    }

    size_t MemoryViewStream::write(std::span<const std::byte>)
    {
        return 0;
    }

    bool MemoryViewStream::seek(SeekOrigin seekOrigin, int offset)
    {
        if (mIsClosed)
            return false;

        auto res = calculateSeek(mPos, mData.size(), seekOrigin, offset);
        if (!res)
            return false;

        mPos = *res;
        return true;
    }

    bool MemoryViewStream::cloneAllContent(const std::function<std::span<std::byte>(size_t)>& targetBuffer) const
    {
        auto buffer = targetBuffer(mData.size());
        if (buffer.empty())
            return false;

        std::ranges::copy_n(mData.data() + mPos, std::min(buffer.size(), mData.size()), buffer.data());
        return true;
    }

    bool FileStream::openFile(const std::filesystem::path& filePath, bool toRead, bool toWrite)
    {
        mCanRead = toRead;
        mCanWrite = toWrite;

        mClosed = false;
        mFileHandle = nullptr;

        if (!toRead && !toWrite)
            return false;

        if (toRead && toWrite)
            mFileHandle = CreateFile(filePath.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        else if (toRead)
            mFileHandle = CreateFile(filePath.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        else if (toWrite)
            mFileHandle = CreateFile(filePath.c_str(), GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

        if (mFileHandle == INVALID_HANDLE_VALUE)
        {
            mFileHandle = nullptr;
            return false;
        }

        return true;
    }

    std::filesystem::path FileStream::createTmpFile(std::string_view baseName, std::string_view extension)
    {
        auto path = std::filesystem::temp_directory_path() / std::format("{}{}", baseName, extension);
        if (!std::filesystem::exists(path))
            return path;

        for (size_t curIndex = 0; true; ++curIndex)
        {
            path = std::filesystem::temp_directory_path() / std::format("{}_{}{}", baseName, curIndex, extension);
            if (std::filesystem::exists(path))
                continue;

            return path;
        }
    }

    std::optional<MemoryStream> FileStream::readEntireFile(const std::filesystem::path& filePath)
    {
        if (filePath.empty())
            return {};

        auto fileHandle = CreateFile(filePath.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (fileHandle == INVALID_HANDLE_VALUE)
            return {};

        ScopedAction scopedAction([&]() { CloseHandle(fileHandle); });

        auto outBufferSize = GetFileSize(fileHandle, nullptr);
        if (outBufferSize <= 0)
            return MemoryStream();

        MemoryStream memStream;
        memStream.truncate(outBufferSize);
        assert(memStream.data().size() == outBufferSize);

        DWORD bytesRead;
        if ((ReadFile(fileHandle, memStream.data().data(), outBufferSize, &bytesRead, nullptr) == 0) || (outBufferSize != bytesRead))
            return {};

        return memStream;
    }

    std::optional<std::string> FileStream::readEntireFileAsString(const std::filesystem::path& filePath)
    {
        if (filePath.empty())
            return {};

        auto fileHandle = CreateFile(filePath.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (fileHandle == INVALID_HANDLE_VALUE)
            return {};

        ScopedAction scopedAction([&]() { CloseHandle(fileHandle); });

        auto outBufferSize = GetFileSize(fileHandle, nullptr);
        if (outBufferSize <= 0)
            return std::string{};

        std::string str;
        str.resize_and_overwrite(static_cast<size_t>(outBufferSize),
                                 [fileHandle, outBufferSize](char* buffer, size_t bufferSize)
                                 {
                                     auto targetSize = std::min(static_cast<DWORD>(bufferSize), outBufferSize);

                                     DWORD bytesRead;
                                     if ((ReadFile(fileHandle, buffer, targetSize, &bytesRead, nullptr) == 0) || (bytesRead != targetSize))
                                         return 0uz;

                                     return static_cast<size_t>(bytesRead);
                                 });

        if (str.empty())
            return {};

        return str;
    }

    bool FileStream::streamFullDump(Stream& stream, const std::filesystem::path& filePath)
    {
        if (!stream.canRead() || (filePath.empty()))
            return false;

        auto fileHandle = CreateFile(filePath.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (!fileHandle)
            return false;

        ScopedAction _([&]() { CloseHandle(fileHandle); });

        auto filePos = stream.position();
        stream.seek(SeekOrigin::Begin, 0);

        ScopedAction __([&]() { stream.seek(SeekOrigin::Begin, filePos); });

        std::array<std::byte, 1024> auxBuffer;
        while (true)
        {
            auto bytesRead = stream.read(auxBuffer);
            if (bytesRead <= 0)
                break;

            DWORD bytesWritten;
            WriteFile(fileHandle, auxBuffer.data(), bytesRead, &bytesWritten, nullptr);
            if (bytesWritten != bytesRead)
                return false;

            if (bytesRead < auxBuffer.size())
                break; // done
        }

        return true;
    }

    void FileStream::close()
    {
        if (mFileHandle)
            CloseHandle(mFileHandle);

        mClosed = true;
        mCanRead = false;
        mCanWrite = false;
        mFileHandle = nullptr;
    }

    void FileStream::flush()
    {
        if (!mClosed && mFileHandle)
            FlushFileBuffers(mFileHandle);
    }

    bool FileStream::canRead() const
    {
        if (mClosed || !mCanRead)
            return false;

        return (SetFilePointer(mFileHandle, 0, nullptr, FILE_CURRENT) < GetFileSize(mFileHandle, nullptr));
    }

    bool FileStream::canRead(size_t numBytes) const
    {
        if (mClosed || !mCanRead)
            return false;

        return ((GetFileSize(mFileHandle, nullptr) - SetFilePointer(mFileHandle, 0, nullptr, FILE_CURRENT)) >= numBytes);
    }

    bool FileStream::canWrite() const
    {
        return (mClosed ? false : mCanWrite);
    }

    bool FileStream::canWrite(size_t) const
    {
        return canWrite();
    }

    size_t FileStream::length() const
    {
        if (!mFileHandle)
            return 0;

        return GetFileSize(mFileHandle, nullptr);
    }

    size_t FileStream::position() const
    {
        if (!mFileHandle)
            return 0;

        return static_cast<size_t>(SetFilePointer(mFileHandle, 0, nullptr, FILE_CURRENT));
    }

    bool FileStream::isValid() const
    {
        return (mFileHandle != nullptr);
    }

    size_t FileStream::read(std::span<std::byte> dst)
    {
        if (dst.size() <= 0)
            return 0;

        if (!mFileHandle)
            return 0;

        DWORD bytesRead;
        if (ReadFile(mFileHandle, dst.data(), dst.size(), &bytesRead, nullptr) == 0)
            return 0;

        return static_cast<size_t>(bytesRead);
    }

    size_t FileStream::write(std::span<const std::byte> src)
    {
        if (src.empty() || !mFileHandle)
            return 0;

        DWORD bytesWritten;
        if (WriteFile(mFileHandle, src.data(), src.size(), &bytesWritten, nullptr) == 0)
            return 0;

        return static_cast<size_t>(bytesWritten);
    }

    bool FileStream::seek(SeekOrigin seekOrigin, int offset)
    {
        if (!mFileHandle)
            return false;

        switch (seekOrigin)
        {
            case SeekOrigin::Begin:
                return (SetFilePointer(mFileHandle, offset, nullptr, FILE_BEGIN) != INVALID_SET_FILE_POINTER);
            case SeekOrigin::End:
                return (SetFilePointer(mFileHandle, offset, nullptr, FILE_END) != INVALID_SET_FILE_POINTER);
            case SeekOrigin::Current:
                return (SetFilePointer(mFileHandle, offset, nullptr, FILE_CURRENT) != INVALID_SET_FILE_POINTER);
            default:
                break;
        }

        return false;
    }

    bool FileStream::cloneAllContent(const std::function<std::span<std::byte>(size_t)>& targetBuffer) const
    {
        if (!mFileHandle)
            return false;

        auto outBufferSize = GetFileSize(mFileHandle, nullptr);
        if (outBufferSize <= 0)
            return true;

        auto outBuffer = targetBuffer(outBufferSize);
        if (outBuffer.empty())
            return false;

        auto curPos = SetFilePointer(mFileHandle, 0, nullptr, FILE_CURRENT);
        SetFilePointer(mFileHandle, 0, nullptr, FILE_BEGIN);

        ScopedAction _([&]() { SetFilePointer(mFileHandle, curPos, nullptr, FILE_BEGIN); });

        outBufferSize = std::min(static_cast<DWORD>(outBuffer.size()), outBufferSize);

        DWORD bytesRead;
        return (ReadFile(mFileHandle, outBuffer.data(), outBufferSize, &bytesRead, nullptr) != 0) && (outBufferSize == bytesRead);
    }
}
