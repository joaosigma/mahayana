#include "Stream.hpp"

#include "ScopedAction.hpp"
#include "UTF.hpp"

#include <malloc.h>
#include <memory.h>

namespace HorseRadish
{
	namespace Streams
	{
		Stream::~Stream()
		{}

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

		void MemoryStream::Close()
		{
			this->closed = true;

			if ((this->managementType != ManagementType::None) && (this->data != nullptr))
				free((void*)this->data);

			this->data = nullptr;
			this->dataBegin = nullptr;
			this->dataEnd = nullptr;
			this->dataWalker = nullptr;
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

		bool MemoryStream::CanRead(int numBytes) const
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

		bool MemoryStream::CanWrite(int numBytes) const
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

		const void* MemoryStream::ReadContent(int &contentSize, bool &contentCopied) const
		{
			contentSize = this->dataSize;

			contentCopied = false;

			return this->data;
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

		bool FileStream::openFile(const HorseRadish::hChar * const filePath, bool toRead, bool toWrite)
		{
			this->toRead = toRead;
			this->toWrite = toWrite;

			this->closed = false;
			this->fileHandle = nullptr;

			if ((toRead == false) && (toWrite == false))
				return false;

			wchar_t filePathWChar[128];
			HorseRadish::UTF::ConvertUTF8To(filePath, HorseRadish::UTF::Encoding::Windows, filePathWChar, sizeof(filePathWChar));

			if ((toRead == true) && (toWrite == true))
				this->fileHandle = CreateFile(filePathWChar, GENERIC_READ | GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
			else if (toRead == true)
				this->fileHandle = CreateFile(filePathWChar, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
			else if (toWrite == true)
				this->fileHandle = CreateFile(filePathWChar, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

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

		FileStream::FileStream(const HorseRadish::hChar * const filePath, bool toRead, bool toWrite)
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

		bool FileStream::CanRead(int numBytes) const
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

		bool FileStream::CanWrite(int numBytes) const
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

		const void* FileStream::ReadContent(int &contentSize, bool &contentCopied) const
		{
			DWORD bytesRead;

			contentSize = 0;
			contentCopied = true;

			if (this->fileHandle == nullptr)
				return nullptr;

			auto outBufferSize = GetFileSize(this->fileHandle, nullptr);
			auto outBuffer = malloc(outBufferSize);
			if (outBuffer == nullptr)
				return nullptr;

			auto curPos = SetFilePointer(this->fileHandle, 0, nullptr, FILE_CURRENT);
			SetFilePointer(this->fileHandle, 0, nullptr, FILE_BEGIN);

			auto readSuccess = (ReadFile(this->fileHandle, outBuffer, outBufferSize, &bytesRead, nullptr) != 0);

			SetFilePointer(this->fileHandle, curPos, nullptr, FILE_BEGIN);

			if ((readSuccess == false) || (outBufferSize != bytesRead))
			{
				free(outBuffer);
				return nullptr;
			}

			contentSize = outBufferSize;
			return outBuffer;
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

		void* FileStream::ReadEntireFile(const HorseRadish::hChar * const filePath, int &fileSize)
		{
			HANDLE fileHandle;
			DWORD bytesRead;

			fileSize = 0;

			if (filePath == nullptr)
				return nullptr;

			{
				wchar_t filePathWChar[128];
				HorseRadish::UTF::ConvertUTF8To(filePath, HorseRadish::UTF::Encoding::Windows, filePathWChar, sizeof(filePathWChar));

				fileHandle = CreateFile(filePathWChar, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
				if (fileHandle == INVALID_HANDLE_VALUE)
					return nullptr;
			}

			ScopedAction scopedAction([&]()
			{
				CloseHandle(fileHandle);
			});

			auto outBufferSize = GetFileSize(fileHandle, nullptr);
			auto outBuffer = malloc(outBufferSize);
			if (outBuffer == nullptr)
				return nullptr;

			if ((ReadFile(fileHandle, outBuffer, outBufferSize, &bytesRead, nullptr) == 0) || (outBufferSize != bytesRead))
			{
				free(outBuffer);
				return nullptr;
			}

			fileSize = outBufferSize;
			return outBuffer;
		}

		HorseRadish::String FileStream::ReadEntireFileAsString(const HorseRadish::hChar * const filePath)
		{
			HANDLE fileHandle;
			DWORD bytesRead;

			if (filePath == nullptr)
				return HorseRadish::String();

			{
				wchar_t filePathWChar[128];
				HorseRadish::UTF::ConvertUTF8To(filePath, HorseRadish::UTF::Encoding::Windows, filePathWChar, sizeof(filePathWChar));

				fileHandle = CreateFile(filePathWChar, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
				if (fileHandle == INVALID_HANDLE_VALUE)
					return HorseRadish::String();
			}

			ScopedAction scopedAction([&]()
			{
				CloseHandle(fileHandle);
			});

			auto outBufferSize = GetFileSize(fileHandle, nullptr);
			auto finalString = HorseRadish::String();

			finalString.Capacity(outBufferSize + 1);

			if ((ReadFile(fileHandle, (void*)finalString.GetData(), outBufferSize, &bytesRead, nullptr) == 0) || (outBufferSize != bytesRead))
				return HorseRadish::String();

			finalString.CloseAt(outBufferSize);
			return finalString;
		}

		bool FileStream::StreamDump(Stream* stream, const HorseRadish::hChar * const filePath)
		{
			HANDLE fileHandle;
			DWORD bytesWritten;
			unsigned char auxBuffer[1024];

			if ((stream == nullptr) || (stream->CanRead() == false) || (filePath == nullptr))
				return false;

			{
				wchar_t filePathWChar[256];
				HorseRadish::UTF::ConvertUTF8To(filePath, HorseRadish::UTF::Encoding::Windows, filePathWChar, sizeof(filePathWChar));

				fileHandle = CreateFile(filePathWChar, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
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