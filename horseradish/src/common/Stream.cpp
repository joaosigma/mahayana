#include "Stream.hpp"

#include "ScopedAction.hpp"
#include "stringUtils.hpp"

namespace HorseRadish
{
	namespace Streams
	{
		Stream::~Stream()
		{ }

		MemoryStream::MemoryStream()
			: data(nullptr), dataBegin(nullptr), dataEnd(nullptr), dataWalker(nullptr), dataSize(0), closed(false), canWrite(false), managementType(ManagementType::None)
		{ }

		MemoryStream::MemoryStream(int initialSize, bool canWrite)
			: dataSize(initialSize)
			, canWrite(canWrite)
			, managementType(ManagementType::ManagedStatic)
		{
			this->data = malloc(this->dataSize);

			this->closed = false;

			this->dataBegin = reinterpret_cast<const hUInt8*>(this->data);
			this->dataEnd = this->dataBegin + this->dataSize;
			this->dataWalker = this->dataBegin;
		}

		MemoryStream::MemoryStream(const void * const bufferData, int bufferSize, bool canWrite, const MemoryStream::ManagementType &managementType)
			: data(bufferData)
			, dataSize(bufferSize)
			, canWrite(canWrite)
			, managementType(managementType)
		{
			if ((this->data == nullptr) || (this->dataSize < 0))
				this->dataSize = 0;

			this->closed = false;

			this->dataBegin = reinterpret_cast<const hUInt8*>(this->data);
			this->dataEnd = this->dataBegin + this->dataSize;
			this->dataWalker = this->dataBegin;
		}

		MemoryStream::~MemoryStream()
		{
			if (this->closed == false)
				this->Close();
		}

		MemoryStream::MemoryStream(MemoryStream&& stream)
		{
			this->data = stream.data;
			this->dataBegin = stream.dataBegin;
			this->dataEnd = stream.dataEnd;
			this->dataWalker = stream.dataWalker;
			this->dataSize = stream.dataSize;
			this->closed = stream.closed;
			this->canWrite = stream.canWrite;
			this->managementType = stream.managementType;

			stream.dataSize = 0;
			stream.closed = true;
			stream.canWrite = false;
			stream.managementType = ManagementType::None;
			stream.data = stream.dataBegin = stream.dataEnd = stream.dataWalker = nullptr;
		}

		void MemoryStream::Close()
		{
			this->closed = true;

			if ((this->managementType != ManagementType::None) && (this->data != nullptr))
				free((void*)this->data);

			this->data = this->dataBegin = this->dataEnd = this->dataWalker = nullptr;
			this->dataSize = 0;
			this->canWrite = false;
		}

		void MemoryStream::Flush()
		{ }

		bool MemoryStream::CanRead() const
		{
			if (closed == true)
				return false;

			return (this->dataWalker < this->dataEnd);
		}

		bool MemoryStream::CanRead(unsigned int numBytes) const
		{
			if (this->CanRead() == false)
				return false;

			if (numBytes < 0)
				return false;

			return (numBytes <= (this->dataEnd - this->dataWalker));
		}

		bool MemoryStream::CanWrite() const
		{
			return this->canWrite;
		}

		bool MemoryStream::CanWrite(unsigned int numBytes) const
		{
			return this->canWrite;
		}

		int MemoryStream::GetLength() const
		{
			return dataSize;
		}

		int MemoryStream::GetPosition() const
		{
			return (this->dataWalker - this->dataBegin);
		}

		int MemoryStream::Read(void * const outBuffer, int numBytes)
		{
			if (numBytes > (this->dataEnd - this->dataWalker))
				numBytes = (this->dataEnd - this->dataWalker);

			if (numBytes <= 0)
				return 0;

			memcpy(outBuffer, this->dataWalker, numBytes);
			this->dataWalker += numBytes;
			return numBytes;
		}

		int MemoryStream::ReadLine(void * const outBuffer, const int bufferSize)
		{
			if ((outBuffer == nullptr) || (bufferSize <= 0))
				return 0;

			if (this->CanRead() == false)
				return 0;

			auto outBufferWalker = reinterpret_cast<char*>(outBuffer);

			auto actualRead = 0;
			do{
				if (*this->dataWalker == '\n')
				{
					this->dataWalker++;
					return actualRead;
				}

				if (*this->dataWalker == '\r')
				{
					this->dataWalker++;

					if (this->dataWalker < this->dataEnd)
					{
						if (*this->dataWalker == '\n')
							this->dataWalker++;
					}

					return actualRead;
				}

				*outBufferWalker = *this->dataWalker;

				actualRead++;
				this->dataWalker++;

				if (actualRead >= bufferSize)
					return -actualRead;

				outBufferWalker++;
			} while (this->dataWalker < this->dataEnd);

			return -actualRead;
		}

		int MemoryStream::ReadUntil(void * const outBuffer, const int bufferSize, const char goal)
		{
			if ((outBuffer == nullptr) || (bufferSize <= 0))
				return 0;

			if (this->CanRead() == false)
				return 0;

			auto outBufferWalker = reinterpret_cast<char*>(outBuffer);

			auto actualRead = 0;
			do{
				*outBufferWalker = *this->dataWalker;

				actualRead++;
				this->dataWalker++;

				if (*outBufferWalker == goal)
					return actualRead;

				if (actualRead >= bufferSize)
					return -actualRead;

				outBufferWalker++;
			} while (this->dataWalker < this->dataEnd);

			return -actualRead;
		}

		int MemoryStream::Write(const void * const inBuffer, int numBytes)
		{
			if (this->canWrite == false)
				return 0;

			memcpy((void*)this->dataWalker, inBuffer, numBytes);
			this->dataWalker += numBytes;
			return numBytes;
		}

		int MemoryStream::Seek(const int offset, const SeekOrigin seekOrigin)
		{
			if (seekOrigin == Stream::Begin)
				this->dataWalker = this->dataBegin + offset;
			else if (seekOrigin == Stream::End)
				this->dataWalker = this->dataEnd + offset;
			else if (seekOrigin == Stream::Current)
				this->dataWalker += offset;

			if (this->dataWalker < this->dataBegin)
				this->dataWalker = this->dataBegin;
			if (this->dataWalker > this->dataEnd)
				this->dataWalker = this->dataEnd;

			return this->GetPosition();
		}

		std::unique_ptr<MemoryStream> MemoryStream::readEntireContent() const
		{
			return std::unique_ptr<MemoryStream>(new MemoryStream(this->data, this->dataSize, false, MemoryStream::ManagementType::None));
		}

		const hUInt8* MemoryStream::getData() const
		{
			return dataBegin;
		}

		std::string MemoryStream::toStr() const
		{
			std::string finalStr;
			finalStr.resize(this->dataSize + 1);

			memcpy(&finalStr[0], this->data, this->dataSize);
			reinterpret_cast<char*>(&finalStr[0])[this->dataSize] = '\0';

			return finalStr;
		}

		bool FileStream::openFile(const std::string& filePath, bool toRead, bool toWrite)
		{
			this->toRead = toRead;
			this->toWrite = toWrite;

			this->closed = false;
			this->fileHandle = nullptr;

			if ((toRead == false) && (toWrite == false))
				return false;

			auto filePathWChar = HorseRadish::StringUtils::conv2UTF16(filePath);

			if ((toRead == true) && (toWrite == true))
				this->fileHandle = CreateFile(filePathWChar.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
			else if (toRead == true)
				this->fileHandle = CreateFile(filePathWChar.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
			else if (toWrite == true)
				this->fileHandle = CreateFile(filePathWChar.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

			if (this->fileHandle == INVALID_HANDLE_VALUE)
			{
				this->fileHandle = nullptr;
				return false;
			}

			return true;
		}

		FileStream::FileStream(FileStream&& stream)
		{
			this->fileHandle = stream.fileHandle;
			this->toRead = stream.toRead;
			this->toWrite = stream.toWrite;
			this->closed = stream.closed;

			stream.fileHandle = nullptr;
			stream.toRead = stream.toWrite = false;
			stream.closed = true;
		}

		FileStream::FileStream(const std::string& filePath, bool toRead, bool toWrite)
		{
			this->toRead = this->toWrite = false;
			this->closed = false;
			this->fileHandle = nullptr;

			this->openFile(filePath, toRead, toWrite);
		}

		FileStream::~FileStream()
		{
			if (this->closed == false)
				this->Close();
		}

		void FileStream::Close()
		{
			if (this->fileHandle != nullptr)
				CloseHandle(this->fileHandle);

			this->closed = true;

			this->toRead = false;
			this->toWrite = false;
			this->fileHandle = nullptr;
		}

		void FileStream::Flush()
		{
			if (this->fileHandle != nullptr)
				FlushFileBuffers(this->fileHandle);
		}

		bool FileStream::CanRead() const
		{
			if (closed == true)
				return false;

			if (this->toRead == false)
				return false;

			return (SetFilePointer(this->fileHandle, 0, nullptr, FILE_CURRENT) < GetFileSize(this->fileHandle, nullptr));
		}

		bool FileStream::CanRead(unsigned int numBytes) const
		{
			if (closed == true)
				return false;

			if (this->toRead == false)
				return false;

			return ((GetFileSize(this->fileHandle, nullptr) - SetFilePointer(this->fileHandle, 0, nullptr, FILE_CURRENT)) >= numBytes);
		}

		bool FileStream::CanWrite() const
		{
			if (closed == true)
				return false;

			return (this->toWrite);
		}

		bool FileStream::CanWrite(unsigned int numBytes) const
		{
			return (this->CanWrite());
		}

		int FileStream::GetLength() const
		{
			if (this->fileHandle == nullptr)
				return 0;

			return GetFileSize(this->fileHandle, nullptr);
		}

		int FileStream::GetPosition() const
		{
			if (this->fileHandle == nullptr)
				return 0;

			return SetFilePointer(this->fileHandle, 0, nullptr, FILE_CURRENT);
		}

		bool FileStream::IsValid() const
		{
			return (this->fileHandle != nullptr);
		}

		int FileStream::Read(void * const outBuffer, int numBytes)
		{
			DWORD bytesRead;

			if ((outBuffer == nullptr) || (numBytes <= 0))
				return 0;

			if (this->fileHandle == nullptr)
				return -1;

			if (ReadFile(this->fileHandle, outBuffer, numBytes, &bytesRead, nullptr) == 0)
				return -1;

			return bytesRead;
		}

		int FileStream::ReadLine(void * const outBuffer, const int bufferSize)
		{
			DWORD bytesRead;
			char dataRead;

			if ((outBuffer == nullptr) || (bufferSize <= 0))
				return 0;

			if (this->CanRead() == false)
				return 0;

			auto outBufferWalker = reinterpret_cast<char*>(outBuffer);

			auto actualRead = 0;
			while (true)
			{
				if (ReadFile(this->fileHandle, &dataRead, 1, &bytesRead, nullptr) == 0)
					break;

				if (bytesRead != 1)
					break;

				if (dataRead == '\n')
					return actualRead;

				if (dataRead == '\r')
				{
					if (ReadFile(this->fileHandle, &dataRead, 1, &bytesRead, nullptr) == 0)
						break;

					if (bytesRead != 1)
						break;

					if (dataRead != '\n')
						SetFilePointer(this->fileHandle, -1, nullptr, FILE_CURRENT);

					return actualRead;
				}

				*outBufferWalker = dataRead;
				actualRead++;

				if (actualRead >= bufferSize)
					return -actualRead;

				outBufferWalker++;
			}

			return -actualRead;
		}

		int FileStream::ReadUntil(void * const outBuffer, const int bufferSize, const char goal)
		{
			DWORD bytesRead;

			if ((outBuffer == nullptr) || (bufferSize <= 0))
				return 0;

			if (this->CanRead() == false)
				return 0;

			auto outBufferWalker = reinterpret_cast<char*>(outBuffer);

			auto actualRead = 0;
			while (true)
			{
				if (ReadFile(this->fileHandle, outBufferWalker, 1, &bytesRead, nullptr) == 0)
					break;

				if (bytesRead != 1)
					break;

				actualRead++;

				if (*outBufferWalker == goal)
					return actualRead;

				if (actualRead >= bufferSize)
					return -actualRead;

				outBufferWalker++;
			}

			return -actualRead;
		}

		int FileStream::Write(const void * const inBuffer, int numBytes)
		{
			DWORD bytesWritten;

			if ((inBuffer == nullptr) || (numBytes <= 0))
				return 0;

			if (this->fileHandle == nullptr)
				return -1;

			if (WriteFile(this->fileHandle, inBuffer, numBytes, &bytesWritten, nullptr) == 0)
				return -1;

			return bytesWritten;
		}

		int FileStream::Seek(const int offset, const SeekOrigin seekOrigin)
		{
			if (this->fileHandle == nullptr)
				return -1;

			if (seekOrigin == Stream::Begin)
				return SetFilePointer(this->fileHandle, offset, nullptr, FILE_BEGIN);
			if (seekOrigin == Stream::End)
				return SetFilePointer(this->fileHandle, offset, nullptr, FILE_END);
			if (seekOrigin == Stream::Current)
				return SetFilePointer(this->fileHandle, offset, nullptr, FILE_CURRENT);

			return -1;
		}

		std::unique_ptr<MemoryStream> FileStream::readEntireContent() const
		{
			if (this->fileHandle == nullptr)
				return std::unique_ptr<MemoryStream>();

			auto outBufferSize = GetFileSize(this->fileHandle, nullptr);

			void* outBuffer = malloc(outBufferSize);
			if (outBuffer == nullptr)
				return std::unique_ptr<MemoryStream>();

			auto curPos = SetFilePointer(this->fileHandle, 0, nullptr, FILE_CURRENT);
			SetFilePointer(this->fileHandle, 0, nullptr, FILE_BEGIN);

			DWORD bytesRead;
			auto readSuccess = (ReadFile(this->fileHandle, outBuffer, outBufferSize, &bytesRead, nullptr) != 0);

			SetFilePointer(this->fileHandle, curPos, nullptr, FILE_BEGIN);

			if ((readSuccess == false) || (outBufferSize != bytesRead))
			{
				free(outBuffer);
				return std::unique_ptr<MemoryStream>();
			}

			return std::unique_ptr<MemoryStream>(new MemoryStream(outBuffer, outBufferSize, false, MemoryStream::ManagementType::ManagedStatic));
		}

		std::unique_ptr<MemoryStream> FileStream::ReadEntireFile(const std::string& filePath)
		{
			HANDLE fileHandle;
			DWORD bytesRead;

			if (filePath.empty())
				return std::unique_ptr<MemoryStream>();

			{
				auto filePathWChar = HorseRadish::StringUtils::conv2UTF16(filePath);

				fileHandle = CreateFile(filePathWChar.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
				if (fileHandle == INVALID_HANDLE_VALUE)
					return std::unique_ptr<MemoryStream>();
			}

			ScopedAction scopedAction([&]()
			{
				CloseHandle(fileHandle);
			});

			auto outBufferSize = GetFileSize(fileHandle, nullptr);
			auto outBuffer = malloc(outBufferSize);
			if (outBuffer == nullptr)
				return std::unique_ptr<MemoryStream>();

			if ((ReadFile(fileHandle, outBuffer, outBufferSize, &bytesRead, nullptr) == 0) || (outBufferSize != bytesRead))
			{
				free(outBuffer);
				return std::unique_ptr<MemoryStream>();
			}

			return std::unique_ptr<MemoryStream>(new MemoryStream(outBuffer, outBufferSize, false, MemoryStream::ManagementType::ManagedStatic));
		}

		std::string FileStream::ReadEntireFileAsString(const std::string& filePath)
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

		bool FileStream::StreamDump(Stream* stream, const std::string& filePath)
		{
			HANDLE fileHandle;
			DWORD bytesWritten;
			unsigned char auxBuffer[1024];

			if ((stream == nullptr) || (stream->CanRead() == false) || (filePath.empty()))
				return false;

			{
				auto filePathWChar = HorseRadish::StringUtils::conv2UTF16(filePath);

				fileHandle = CreateFile(filePathWChar.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
				if (fileHandle == nullptr)
					return false;
			}

			auto filePos = stream->GetPosition();
			stream->Seek(0, Stream::Begin);

			while (true)
			{
				auto bytesRead = stream->Read(auxBuffer, sizeof(auxBuffer));
				if (bytesRead <= 0)
					break;

				WriteFile(fileHandle, auxBuffer, bytesRead, &bytesWritten, nullptr);
				if (bytesWritten != bytesRead)
					break;

				if (bytesRead < sizeof(auxBuffer))
					break;
			}

			CloseHandle(fileHandle);

			stream->Seek(filePos, Stream::Begin);

			return true;
		}

	} //Streams
} //HorseRadish