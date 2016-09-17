#include "Stream.hpp"

#include "ScopedAction.hpp"
#include "stringUtils.hpp"

#include <memory>

namespace HorseRadish { namespace Streams {

void MemoryStream::close()
{
	if (mData)
		free(mData);

	mDataSize = 0;
	mIsClosed = true;
	mData = mDataBegin = mDataEnd = mDataWalker = nullptr;
}

void MemoryStream::flush()
{ }

bool MemoryStream::canRead() const
{
	if (mIsClosed)
		return false;

	return (mDataWalker < mDataEnd);
}

bool MemoryStream::canRead(size_t numBytes) const
{
	if (!canRead())
		return false;

	return (numBytes <= (mDataEnd - mDataWalker));
}

bool MemoryStream::canWrite() const
{
	return !mIsClosed;
}

bool MemoryStream::canWrite(size_t numBytes) const
{
	return canWrite();
}

size_t MemoryStream::length() const
{
	return mDataSize;
}

size_t MemoryStream::position() const
{
	return (mDataWalker - mDataBegin);
}

size_t MemoryStream::read(void* const outBuffer, size_t numBytes)
{
	if (!canRead())
		return 0;

	if (numBytes > (mDataEnd - mDataWalker))
		numBytes = (mDataEnd - mDataWalker);

	if (numBytes <= 0)
		return 0;

	memcpy(outBuffer, mDataWalker, numBytes);
	mDataWalker += numBytes;
	return numBytes;
}

size_t MemoryStream::write(const void* const inBuffer, size_t numBytes)
{
	if (!canWrite())
		return 0;

	if ((mDataEnd - mDataWalker) > numBytes)
	{
		if (mDataSize <= 1024)
			mDataSize = (mDataSize * 10) / 4;
		else if (mDataSize <= (100 * 1024))
			mDataSize = (mDataSize * 10) / 5;
		else if (mDataSize <= (1024 * 1024))
			mDataSize = (mDataSize * 10) / 8;
		else if (mDataSize <= (1024 * 1024))
			mDataSize = (mDataSize * 10) / 8;
		else if (mDataSize <= (10 * 1024 * 1024))
			mDataSize = mDataSize + (1024 * 1024);

		auto oldPtr = mData;
		mData = realloc(mData, mDataSize);
		if (mData != oldPtr)
		{
			mDataBegin = reinterpret_cast<unsigned char*>(mData);
			mDataWalker = mDataBegin + (mDataWalker - reinterpret_cast<unsigned char*>(oldPtr));
			mDataEnd = mDataBegin + mDataSize;
		}
	}

	memcpy(mDataWalker, inBuffer, numBytes);
	mDataWalker += numBytes;
	return numBytes;
}

bool MemoryStream::seek(SeekOrigin seekOrigin, int offset)
{
	switch (seekOrigin)
	{
	case SeekOrigin::Begin:
		mDataWalker = mDataBegin + offset;
	case SeekOrigin::End:
		mDataWalker = mDataEnd + offset;
	case SeekOrigin::Current:
		mDataWalker += offset;
	};
	
	if (mDataWalker < mDataBegin)
		mDataWalker = mDataBegin;
	if (mDataWalker > mDataEnd)
		mDataWalker = mDataEnd;

	return true;
}

std::unique_ptr<MemoryViewStream> MemoryStream::readEntireContent() const
{
	if (length() == 0)
		return std::unique_ptr<MemoryViewStream>(new MemoryViewStream());

	std::shared_ptr<unsigned char> buffer(new unsigned char[mDataSize], std::default_delete<unsigned char[]>());
	return std::unique_ptr<MemoryViewStream>(new MemoryViewStream(buffer, mDataSize));
}

const void* MemoryStream::getData() const
{
	return mData;
}

std::string MemoryStream::toStr() const
{
	std::string finalStr;
	finalStr.resize(length() + 1);

	memcpy(&finalStr[0], mData, length());
	reinterpret_cast<char*>(&finalStr[0])[length()] = '\0';

	return finalStr;
}

void MemoryViewStream::close()
{
	mIsClosed = true;
}

void MemoryViewStream::flush()
{ }

bool MemoryViewStream::canRead() const
{
	return !mIsClosed;
}

bool MemoryViewStream::canRead(size_t numBytes) const
{
	if (!canRead())
		return false;

	return (numBytes <= (mDataEnd - mDataWalker));
}

bool MemoryViewStream::canWrite() const
{
	return false;
}

bool MemoryViewStream::canWrite(size_t numBytes) const
{
	return canWrite();
}

size_t MemoryViewStream::length() const
{
	return (mDataEnd - mDataBegin);
}

size_t MemoryViewStream::position() const
{
	return (mDataWalker - mDataBegin);
}

size_t MemoryViewStream::read(void* const outBuffer, size_t numBytes)
{
	if (numBytes > (mDataEnd - mDataWalker))
		numBytes = (mDataEnd - mDataWalker);

	if (numBytes <= 0)
		return 0;

	memcpy(outBuffer, mDataWalker, numBytes);
	mDataWalker += numBytes;

	return numBytes;
}

size_t MemoryViewStream::write(const void* const inBuffer, size_t numBytes)
{
	return 0;
}

bool MemoryViewStream::seek(SeekOrigin seekOrigin, int offset)
{
	switch (seekOrigin)
	{
	case SeekOrigin::Begin:
		mDataWalker = mDataBegin + offset;
		break;
	case SeekOrigin::End:
		mDataWalker = mDataEnd + offset;
		break;
	default:
		mDataWalker += offset;
	}
	
	if (mDataWalker < mDataBegin)
		mDataWalker = mDataBegin;
	if (mDataWalker > mDataEnd)
		mDataWalker = mDataEnd;

	return true;
}

std::unique_ptr<MemoryViewStream> MemoryViewStream::readEntireContent() const
{
	return std::unique_ptr<MemoryViewStream>(new MemoryViewStream(mDataShared, length()));
}

const void* MemoryViewStream::getData() const
{
	return mData;
}

std::string MemoryViewStream::toStr() const
{
	std::string finalStr;
	finalStr.resize(length() + 1);

	memcpy(&finalStr[0], mData, length());
	reinterpret_cast<char*>(&finalStr[0])[length()] = '\0';

	return finalStr;
}

bool FileStream::openFile(const std::string& filePath, bool toRead, bool toWrite)
{
	mCanRead = toRead;
	mCanWrite = toWrite;

	mClosed = false;
	mFileHandle = nullptr;

	if ((toRead == false) && (toWrite == false))
		return false;

	auto filePathWChar = HorseRadish::StringUtils::conv2UTF16(filePath);

	if ((toRead == true) && (toWrite == true))
		mFileHandle = CreateFile(filePathWChar.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	else if (toRead == true)
		mFileHandle = CreateFile(filePathWChar.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	else if (toWrite == true)
		mFileHandle = CreateFile(filePathWChar.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

	if (mFileHandle == INVALID_HANDLE_VALUE)
	{
		mFileHandle = nullptr;
		return false;
	}

	return true;
}

std::unique_ptr<MemoryViewStream> FileStream::readEntireFile(const std::string& filePath)
{
	if (filePath.empty())
		return std::unique_ptr<MemoryViewStream>();

	HANDLE fileHandle;
	{
		auto filePathWChar = HorseRadish::StringUtils::conv2UTF16(filePath);

		fileHandle = CreateFile(filePathWChar.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (fileHandle == INVALID_HANDLE_VALUE)
			return std::unique_ptr<MemoryViewStream>();
	}

	ScopedAction scopedAction([&]()
	{
		CloseHandle(fileHandle);
	});

	auto outBufferSize = GetFileSize(fileHandle, nullptr);
	if (outBufferSize <= 0)
		return std::unique_ptr<MemoryViewStream>(new MemoryViewStream());

	std::shared_ptr<unsigned char> outBuffer(new unsigned char[outBufferSize], std::default_delete<unsigned char[]>());

	DWORD bytesRead;
	if ((ReadFile(fileHandle, outBuffer.get(), outBufferSize, &bytesRead, nullptr) == 0) || (outBufferSize != bytesRead))
		return std::unique_ptr<MemoryViewStream>();

	return std::unique_ptr<MemoryViewStream>(new MemoryViewStream(outBuffer, outBufferSize));
}

std::string FileStream::readEntireFileAsString(const std::string& filePath)
{
	HANDLE fileHandle;
	DWORD bytesRead;

	if (filePath.empty())
		return std::string();

	{
		auto filePathWChar = HorseRadish::StringUtils::conv2UTF16(filePath);

		fileHandle = CreateFile(filePathWChar.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (fileHandle == INVALID_HANDLE_VALUE)
			return std::string();
	}

	ScopedAction scopedAction([&]()
	{
		CloseHandle(fileHandle);
	});

	auto outBufferSize = GetFileSize(fileHandle, nullptr);

	std::unique_ptr<char[]> tmpBuffer(new char[outBufferSize]);

	if ((ReadFile(fileHandle, tmpBuffer.get(), outBufferSize, &bytesRead, nullptr) == 0) || (outBufferSize != bytesRead))
		return std::string();

	return std::string(tmpBuffer.get(), outBufferSize);
}

bool FileStream::streamDump(Stream& stream, const std::string& filePath)
{			
	if ((stream.canRead() == false) || (filePath.empty()))
		return false;

	HANDLE fileHandle;
	{
		auto filePathWChar = HorseRadish::StringUtils::conv2UTF16(filePath);

		fileHandle = CreateFile(filePathWChar.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (fileHandle == nullptr)
			return false;
	}

	auto filePos = stream.position();
	stream.seek(SeekOrigin::Begin, 0);

	unsigned char auxBuffer[1024];

	while (true)
	{
		auto bytesRead = stream.read(auxBuffer, sizeof(auxBuffer));
		if (bytesRead <= 0)
			break;

		DWORD bytesWritten;
		WriteFile(fileHandle, auxBuffer, bytesRead, &bytesWritten, nullptr);
		if (bytesWritten != bytesRead)
			break;

		if (bytesRead < sizeof(auxBuffer))
			break;
	}

	CloseHandle(fileHandle);

	stream.seek(SeekOrigin::Begin, filePos);

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
	if (mFileHandle)
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

bool FileStream::canWrite(size_t numBytes) const
{
	return canWrite();
}

size_t FileStream::length() const
{
	if (mFileHandle == nullptr)
		return 0;

	return GetFileSize(mFileHandle, nullptr);
}

size_t FileStream::position() const
{
	if (mFileHandle == nullptr)
		return 0;

	return SetFilePointer(mFileHandle, 0, nullptr, FILE_CURRENT);
}

bool FileStream::isValid() const
{
	return (mFileHandle != nullptr);
}

size_t FileStream::read(void* const outBuffer, size_t numBytes)
{
	if ((outBuffer == nullptr) || (numBytes <= 0))
		return 0;

	if (mFileHandle == nullptr)
		return 0;

	DWORD bytesRead;
	if (ReadFile(mFileHandle, outBuffer, numBytes, &bytesRead, nullptr) == 0)
		return 0;

	return bytesRead;
}

size_t FileStream::write(const void* const inBuffer, size_t numBytes)
{
	DWORD bytesWritten;

	if ((inBuffer == nullptr) || (numBytes <= 0))
		return 0;

	if (mFileHandle == nullptr)
		return -1;

	if (WriteFile(mFileHandle, inBuffer, numBytes, &bytesWritten, nullptr) == 0)
		return -1;

	return bytesWritten;
}

bool FileStream::seek(SeekOrigin seekOrigin, int offset)
{
	if (mFileHandle == nullptr)
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

std::unique_ptr<MemoryViewStream> FileStream::readEntireContent() const
{
	if (mFileHandle == nullptr)
		return std::unique_ptr<MemoryViewStream>();

	auto outBufferSize = GetFileSize(mFileHandle, nullptr);
	if (outBufferSize <= 0)
		return std::unique_ptr<MemoryViewStream>(new MemoryViewStream());

	std::shared_ptr<unsigned char> outBuffer(new unsigned char[outBufferSize], std::default_delete<unsigned char[]>());

	auto curPos = SetFilePointer(mFileHandle, 0, nullptr, FILE_CURRENT);
	SetFilePointer(mFileHandle, 0, nullptr, FILE_BEGIN);

	DWORD bytesRead;
	auto readSuccess = (ReadFile(mFileHandle, outBuffer.get(), outBufferSize, &bytesRead, nullptr) != 0);

	SetFilePointer(mFileHandle, curPos, nullptr, FILE_BEGIN);

	if ((readSuccess == false) || (outBufferSize != bytesRead))
		return std::unique_ptr<MemoryViewStream>();

	return std::unique_ptr<MemoryViewStream>(new MemoryViewStream(outBuffer, outBufferSize));
}

} }