#pragma once

#include "Types.hpp"
#include "Math.hpp"
#include "Path.hpp"

#include "Platform.hpp"

#include <memory>
#include <type_traits>

#include <windows.h>

namespace HorseRadish
{
	namespace Streams
	{
		class MemoryStream;
		class MemoryViewStream;
		class FileStream;

		class Stream
		{
		public:
			enum class SeekOrigin { Begin, Current, End };

			Stream()
			{ }
			virtual ~Stream()
			{ }

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
			 
			virtual std::unique_ptr<MemoryViewStream> readEntireContent() const = 0;
		};

		class MemoryStream final : public Stream
		{
			void* mData = nullptr;
			unsigned char *mDataBegin = nullptr, *mDataEnd = nullptr, *mDataWalker = nullptr;
			size_t mDataSize = 0;
			bool mIsClosed = false;

		public:
			MemoryStream(size_t reserveSize = 1024)
			{
				mDataSize = (reserveSize < 1024) ? 1024 : reserveSize;
				mData = realloc(mData, mDataSize);

				mDataBegin = mDataWalker = reinterpret_cast<unsigned char*>(mData);
				mDataEnd = mDataBegin + mDataSize;
			}

			~MemoryStream()
			{
				close();
			}

			MemoryStream(const MemoryStream&) = delete;
			const MemoryStream& operator=(const MemoryStream&) = delete;

			MemoryStream(MemoryStream&& stream)
			{
				*this = std::move(stream);
			}

			MemoryStream& operator=(MemoryStream&& stream)
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

			std::unique_ptr<MemoryViewStream> readEntireContent() const override;

			const void* getData() const;
			std::string toStr() const;
		};

		class MemoryViewStream final : public Stream
		{
			const void* mData = nullptr;
			const unsigned char *mDataBegin = nullptr, *mDataEnd = nullptr, *mDataWalker = nullptr;
			bool mIsClosed = false;
			std::shared_ptr<unsigned char> mDataShared;

		public:
			MemoryViewStream()
			{ }

			MemoryViewStream(std::shared_ptr<unsigned char> data, size_t dataSize)
				: MemoryViewStream(data, 0, dataSize)
			{ }

			MemoryViewStream(std::shared_ptr<unsigned char> data, size_t dataOffset, size_t dataSize)
			{
				mDataShared = data;
				mData = data.get();
				mDataBegin = reinterpret_cast<const unsigned char*>(mData) + dataOffset;
				mDataEnd = mDataBegin + dataSize;
			}

			MemoryViewStream(const MemoryViewStream&) = delete;
			const MemoryViewStream& operator=(const MemoryViewStream&) = delete;

			MemoryViewStream(MemoryViewStream&& stream)
			{
				*this = std::move(stream);
			}

			MemoryViewStream& operator=(MemoryViewStream&& stream)
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

			std::unique_ptr<MemoryViewStream> readEntireContent() const override;

			const void* getData() const;
			std::string toStr() const;
		};

		class FileStream final : public Stream
		{
			HANDLE mFileHandle = nullptr;
			bool mCanRead = false, mCanWrite = false, mClosed = false;

			bool openFile(const std::string& filePath, bool toRead, bool toWrite);

		public:
			static std::unique_ptr<MemoryViewStream> readEntireFile(const std::string& filePath);
			static std::string readEntireFileAsString(const std::string& filePath);
			static bool streamDump(Stream& stream, const std::string& filePath);

		public:
			FileStream(const std::string& filePath, bool toRead, bool toWrite)
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

			FileStream(FileStream&& stream)
			{
				*this = std::move(stream);
			}

			FileStream& operator=(FileStream&& stream)
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

			std::unique_ptr<MemoryViewStream> readEntireContent() const override;
		};

		class StreamReader
		{
			Stream &mStream;
		public:

			StreamReader(Stream &stream)
				: mStream(stream)
			{ }

			const Stream& stream() const
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

			size_t read(void* const buffer, const size_t numBytes)
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

			bool skip(const size_t offset)
			{
				return mStream.seek(Stream::SeekOrigin::Current, offset);
			}

			bool seek(Stream::SeekOrigin seekOrigin, const int offset)
			{
				return mStream.seek(seekOrigin, offset);
			}
		};

		class StreamWriter
		{
			Stream &mStream;
		public:

			StreamWriter(Stream &stream)
				: mStream(stream)
			{ }

			const Stream& stream() const
			{
				return mStream;
			}

			template<typename T>
			bool write(const T& value)
			{
				if (!mStream.canWrite())
					return false;

				return (mStream.write(&value, sizeof(T)) == sizeof(T));
			}

			size_t write(const void* const buffer, const size_t numBytes)
			{
				if (!mStream.canWrite())
					return 0;

				return mStream.write(buffer, numBytes);
			}

			size_t writeString(const char* const str, bool includeTerminator = false)
			{
				if (!mStream.canWrite() || (str == nullptr))
					return 0;

				auto strLen = strlen(str);
				if (includeTerminator)
					strLen++;

				return mStream.write(str, strLen);
			}

			size_t position() const
			{
				return mStream.position();
			}

			bool skip(const size_t offset)
			{
				return mStream.seek(Stream::SeekOrigin::Current, offset);
			}

			bool seek(Stream::SeekOrigin seekOrigin, const int offset)
			{
				return mStream.seek(seekOrigin, offset);
			}
		};

		class TextWriter
		{
			Stream &mStream;

		public:

			TextWriter(Stream &stream)
				: mStream(stream)
			{ }

			const Stream& stream() const
			{
				return mStream;
			}

			bool write(const char* const string, bool writeLine = false)
			{
				if ((string == nullptr) || (string[0] == '\0'))
					return 0;

				return write(string, strlen(string), writeLine);
			}

			bool write(const char* const string, size_t bytesToWrite, bool writeLine = false)
			{
				if (!mStream.canWrite() || (string == nullptr) || (bytesToWrite <= 0))
					return 0;

				auto success = (mStream.write(string, bytesToWrite) == bytesToWrite);

				if (success && writeLine)
					success &= (mStream.write(HorseRadish::Platform::NewLine, HorseRadish::Platform::NewLineSize) == HorseRadish::Platform::NewLineSize);

				return success;
			}

			void writeLine()
			{
				mStream.write(HorseRadish::Platform::NewLine, HorseRadish::Platform::NewLineSize);
			}
		};

	} //Streams
} //HorseRadish
