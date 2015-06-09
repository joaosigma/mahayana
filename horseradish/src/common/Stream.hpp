#pragma once

#include "String.hpp"
#include "Types.hpp"
#include "Math.hpp"
#include "Path.hpp"

#include "Platform.hpp"

#include <memory>
#include <windows.h>

namespace HorseRadish
{
	namespace Streams
	{
		class MemoryStream;
		class FileStream;

		class Stream
		{
		public:
			enum SeekOrigin { Begin, Current, End };

			Stream() { }
			virtual ~Stream() = 0;

			virtual void Close() = 0;
			virtual void Flush() = 0;

			virtual bool CanRead() const = 0;
			virtual bool CanRead(int numBytes) const = 0;
			virtual bool CanWrite() const = 0;
			virtual bool CanWrite(int numBytes) const = 0;
			virtual int GetLength() const = 0;
			virtual int GetPosition() const = 0;

			virtual int Read(void * const outBuffer, int numBytes) = 0;
			virtual int ReadLine(void * const outBuffer, const int bufferSize) = 0;
			virtual int ReadUntil(void * const outBuffer, const int bufferSize, const char goal) = 0;
			virtual int Write(const void * const inBuffer, int numBytes) = 0;
			virtual int Seek(const int offset, const SeekOrigin seekOrigin) = 0;

			virtual std::unique_ptr<MemoryStream> readEntireContent() const = 0;
		};

		class MemoryStream : public Stream
		{
		public:
			enum class ManagementType { None, ManagedStatic, ManagedGrow };

		private:
			const void *data;
			const hUInt8 *dataBegin, *dataEnd, *dataWalker;
			int dataSize;
			bool closed, canWrite;
			ManagementType managementType;

		public:
			MemoryStream(MemoryStream&& stream);
			MemoryStream(const void * const bufferData, int bufferSize, bool canWrite, const ManagementType &managementType);
			~MemoryStream();

			MemoryStream(const MemoryStream&) = delete;
			const MemoryStream& operator=(const MemoryStream&) = delete;

			void Close();
			void Flush();

			bool CanRead() const;
			bool CanRead(int numBytes) const;
			bool CanWrite() const;
			bool CanWrite(int numBytes) const;
			int GetLength() const;
			int GetPosition() const;

			int Read(void * const outBuffer, int numBytes);
			int ReadLine(void * const outBuffer, const int bufferSize);
			int ReadUntil(void * const outBuffer, const int bufferSize, const char goal);
			int Write(const void * const inBuffer, int numBytes);
			int Seek(const int offset, const SeekOrigin seekOrigin);

			std::unique_ptr<MemoryStream> readEntireContent() const;

			const hUInt8* getData() const;
			std::string toStr() const;
		};

		class FileStream : public Stream
		{
			HANDLE fileHandle;
			bool toRead, toWrite, closed;

			bool openFile(const HorseRadish::hChar * const filePath, bool toRead, bool toWrite);

		public:
			FileStream(FileStream&& stream);
			FileStream(const HorseRadish::hChar * const filePath, bool toRead, bool toWrite);
			~FileStream();

			FileStream(const FileStream&) = delete;
			const FileStream& operator=(const FileStream&) = delete;

			void Close();
			void Flush();

			bool CanRead() const;
			bool CanRead(int numBytes) const;
			bool CanWrite() const;
			bool CanWrite(int numBytes) const;
			int GetLength() const;
			int GetPosition() const;

			bool IsValid() const;

			int Read(void * const outBuffer, int numBytes);
			int ReadLine(void * const outBuffer, const int bufferSize);
			int ReadUntil(void * const outBuffer, const int bufferSize, const char goal);
			int Write(const void * const inBuffer, int numBytes);
			int Seek(const int offset, const SeekOrigin seekOrigin);

			std::unique_ptr<MemoryStream> readEntireContent() const;

			static std::unique_ptr<MemoryStream> ReadEntireFile(const HorseRadish::hChar * const filePath);
			static HorseRadish::String ReadEntireFileAsString(const HorseRadish::hChar * const filePath);
			static bool StreamDump(Stream* stream, const HorseRadish::hChar * const filePath);
		};

		class StreamReader
		{
			Stream &stream;
		public:

			StreamReader(Stream &stream)
				: stream(stream)
			{ }

			const Stream& getStream() const
			{
				return stream;
			}

			bool ReadInt8(hInt8 &outBuffer)
			{
				if (stream.CanRead(sizeof(hInt8)) == false)
					return false;

				if (stream.Read(&outBuffer, sizeof(hInt8)) != sizeof(hInt8))
					return false;

				return true;
			}

			bool ReadInt16(hInt16 &outBuffer)
			{
				if (stream.CanRead(sizeof(hInt16)) == false)
					return false;

				if (stream.Read(&outBuffer, sizeof(hInt16)) != sizeof(hInt16))
					return false;

				return true;
			}

			bool ReadInt32(hInt32 &outBuffer)
			{
				if (stream.CanRead(sizeof(hInt32)) == false)
					return false;

				if (stream.Read(&outBuffer, sizeof(hInt32)) != sizeof(hInt32))
					return false;

				return true;
			}

			bool ReadInt64(hInt64 &outBuffer)
			{
				if (stream.CanRead(sizeof(hInt64)) == false)
					return false;

				if (stream.Read(&outBuffer, sizeof(hInt64)) != sizeof(hInt64))
					return false;

				return true;
			}

			bool ReadDouble(double &outBuffer)
			{
				if (stream.CanRead(sizeof(double)) == false)
					return false;

				if (stream.Read(&outBuffer, sizeof(double)) != sizeof(double))
					return false;

				return true;
			}

			int ReadDoubles(double * const outBuffer, int numDoubles)
			{
				if (stream.CanRead() == false)
					return 0;

				auto doublesRead = 0;
				while ((stream.CanRead(sizeof(double)) == true) && (doublesRead < numDoubles))
				{
					if (stream.Read(outBuffer + doublesRead, sizeof(double)) != sizeof(double))
						break;

					doublesRead++;
				}

				return doublesRead;
			}

			bool ReadFloat(float &outBuffer)
			{
				if (stream.CanRead(sizeof(float)) == false)
					return false;

				if (stream.Read(&outBuffer, sizeof(float)) != sizeof(float))
					return false;

				return true;
			}

			int ReadFloats(float * const outBuffer, int numFloats)
			{
				if (stream.CanRead() == false)
					return 0;

				auto floatsRead = 0;
				while ((stream.CanRead(sizeof(float)) == true) && (floatsRead < numFloats))
				{
					if (stream.Read(outBuffer + floatsRead, sizeof(float)) != sizeof(float))
						break;

					floatsRead++;
				}

				return floatsRead;
			}

			int Read(void * const outBuffer, int numBytes)
			{
				return (stream.Read(outBuffer, numBytes));
			}

			int ReadLine(void * const outBuffer, const int bufferSize)
			{
				return (stream.ReadLine(outBuffer, bufferSize));
			}

			int ReadLineString(HorseRadish::String &targetString, const HorseRadish::String::Encoding inputEncoding)
			{
				char lineBuffer[1024 * 2];

				targetString.SetEmpty();

				auto bytesLidos = stream.ReadLine(lineBuffer, sizeof(lineBuffer));
				if ((bytesLidos < 0) || (bytesLidos >= sizeof(lineBuffer)))
					return -bytesLidos;

				lineBuffer[bytesLidos] = '\0';
				targetString.Set(inputEncoding, lineBuffer);

				return bytesLidos;
			}

			int ReadUntil(void * const outBuffer, const int bufferSize, const char goal)
			{
				return (stream.ReadUntil(outBuffer, bufferSize, goal));
			}

			int GetPosition() const
			{
				return (stream.GetPosition());
			}

			bool CanRead() const
			{
				return (stream.CanRead());
			}

			int Seek(const int offset)
			{
				return (stream.Seek(offset, Stream::Current));
			}

			int Seek(const int offset, const Stream::SeekOrigin seekOrigin)
			{
				return (stream.Seek(offset, seekOrigin));
			}
		};

		class StreamWriter
		{
			Stream &stream;
		public:

			StreamWriter(Stream &stream)
				: stream(stream)
			{ }

			const Stream& getStream() const
			{
				return stream;
			}

			bool WriteInt8(const hInt8 &inValue)
			{
				if (stream.CanWrite() == false)
					return 0;

				return (stream.Write(&inValue, sizeof(hInt8)) == sizeof(hInt8));
			}

			bool WriteInt16(const hInt16 &inValue)
			{
				if (stream.CanWrite() == false)
					return 0;

				return (stream.Write(&inValue, sizeof(hInt16)) == sizeof(hInt16));
			}

			bool WriteInt32(const hInt32 &inValue)
			{
				if (stream.CanWrite() == false)
					return 0;

				return (stream.Write(&inValue, sizeof(hInt32)) == sizeof(hInt32));
			}

			bool WriteInt64(const hInt64 &inValue)
			{
				if (stream.CanWrite() == false)
					return 0;

				return (stream.Write(&inValue, sizeof(hInt64)) == sizeof(hInt64));
			}

			bool WriteDouble(const double &inValue)
			{
				if (stream.CanWrite() == false)
					return 0;

				return (stream.Write(&inValue, sizeof(double)) == sizeof(double));
			}

			int WriteDoubles(const double * const inBuffer, int numDoubles)
			{
				if (!stream.CanWrite() || (numDoubles <= 0))
					return 0;

				for (int i = 0; i < numDoubles; i++)
				{
					if (stream.Write(inBuffer + i, sizeof(double)) != sizeof(double))
						return i;
				}

				return numDoubles;
			}

			bool WriteFloat(const float &inValue)
			{
				if (stream.CanWrite() == false)
					return 0;

				return (stream.Write(&inValue, sizeof(float)) == sizeof(float));
			}

			int WriteFloats(const float * const inBuffer, int numFloats)
			{
				if (!stream.CanWrite() || (numFloats <= 0))
					return 0;

				for (int i = 0; i < numFloats; i++)
				{
					if (stream.Write(inBuffer + i, sizeof(float)) != sizeof(float))
						return i;
				}

				return numFloats;
			}

			int Write(const void * const inBuffer, int numBytes)
			{
				if (stream.CanWrite() == false)
					return 0;

				return stream.Write(inBuffer, numBytes);
			}

			int WriteString(const char * const stringData, bool includeTerminator)
			{
				if (stream.CanWrite() == false)
					return 0;

				auto bytesWritten = 0;

				auto stringIntWalker = reinterpret_cast<const hInt32*>(stringData);
				for (; Math::iHasZero(*stringIntWalker) == false; stringIntWalker++)
					bytesWritten += stream.Write(stringIntWalker, sizeof(hInt32));

				auto stringWalker = reinterpret_cast<const char *>(stringIntWalker);
				for (; *stringWalker != '\0'; stringWalker++)
					bytesWritten += stream.Write(stringWalker, 1);

				if (includeTerminator == true)
					bytesWritten += stream.Write(stringWalker, 1);

				return bytesWritten;
			}

			int GetPosition() const
			{
				return (stream.GetPosition());
			}

			int Seek(const int offset)
			{
				return (stream.Seek(offset, Stream::Current));
			}

			int Seek(const int offset, const Stream::SeekOrigin seekOrigin)
			{
				return (stream.Seek(offset, seekOrigin));
			}
		};

		class TextWriter
		{
			Stream &stream;

		public:

			TextWriter(Stream &stream)
				: stream(stream)
			{ }

			const Stream& getStream() const
			{
				return stream;
			}

			bool Write(const char * const string, bool writeLine = false)
			{
				if ((string == nullptr) || (string[0] == '\0'))
					return 0;

				return (this->Write(string, strlen(string), writeLine));
			}

			bool Write(const char * const string, int bytesToWrite, bool writeLine = false)
			{
				if (!stream.CanWrite() || (string == nullptr) || (bytesToWrite <= 0))
					return 0;

				auto sucesso = (stream.Write(string, bytesToWrite) == bytesToWrite);

				if (sucesso && writeLine)
					sucesso &= (stream.Write(HorseRadish::Platform::NewLine, HorseRadish::Platform::NewLineSize) == HorseRadish::Platform::NewLineSize);

				return sucesso;
			}

			void WriteLine()
			{
				stream.Write(HorseRadish::Platform::NewLine, HorseRadish::Platform::NewLineSize);
			}
		};

	} //Streams
} //HorseRadish
