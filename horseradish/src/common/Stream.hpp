#pragma once
#ifndef __HSTREAM__
#define __HSTREAM__

#include "Platform.hpp"
#include "String.hpp"
#include "Types.hpp"
#include "Math.hpp"
#include "Path.hpp"

#include <windows.h>
#include <stdio.h>

namespace HorseRadish
{
namespace Streams
{

///§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§   -= Tipos de streams =-   §§§§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
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
	virtual const void* ReadContent(int &contentSize, bool &contentCopied) const = 0;
	virtual int Write(const void * const inBuffer, int numBytes) = 0;
	virtual int Seek(const int offset, const SeekOrigin seekOrigin) = 0;
};

class MemoryStream : public Stream
{
public:
	enum ManagementType { None, ManagedStatic, ManagedGrow };

private:
	const void *data;
	const hUInt8 *dataBegin, *dataEnd, *dataWalker;
	int dataSize;
	bool closed, canWrite;
	ManagementType managementType;

public:
	MemoryStream(const void * const bufferData, int bufferSize, bool canWrite, const ManagementType &managementType);
	~MemoryStream();

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
	const void* ReadContent(int &contentSize, bool &contentCopied) const;
	int Write(const void * const inBuffer, int numBytes);
	int Seek(const int offset, const SeekOrigin seekOrigin);
};

class FileStream : public Stream
{
	HANDLE fileHandle;
	bool toRead, toWrite, closed;

	bool openFile(const HorseRadish::hChar * const filePath, bool toRead, bool toWrite);

public:
	FileStream(const HorseRadish::hChar * const filePath, bool toRead, bool toWrite);
	~FileStream();

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
	const void* ReadContent(int &contentSize, bool &contentCopied) const;
	int Write(const void * const inBuffer, int numBytes);
	int Seek(const int offset, const SeekOrigin seekOrigin);

	static void* ReadEntireFile(const HorseRadish::hChar * const filePath, int &fileSize);
	static HorseRadish::String ReadEntireFileAsString(const HorseRadish::hChar * const filePath);
	static bool StreamDump(Stream* stream, const HorseRadish::hChar * const filePath);
};

///§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§   -= Manipuladores de streams (escrever / ler) =-   §§§§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
class StreamReader
{
	Stream *stream;
public:
	
    StreamReader(Stream * const stream) { this->stream = stream; }
	~StreamReader() { }

	bool ReadInt8(hInt8 &outBuffer)
	{
		//se não posso ler os bytes necessários
		if (stream->CanRead(sizeof(hInt8)) == false)
			return false;

		//basta mandar ler
		if (stream->Read(&outBuffer, sizeof(hInt8)) != sizeof(hInt8))
			return false;
		return true;
	}
	bool ReadInt16(hInt16 &outBuffer)
	{
		//se não posso ler os bytes necessários
		if (stream->CanRead(sizeof(hInt16)) == false)
			return false;

		//basta mandar ler
		if (stream->Read(&outBuffer, sizeof(hInt16)) != sizeof(hInt16))
			return false;
		return true;
	}
	bool ReadInt32(hInt32 &outBuffer)
	{
		//se não posso ler os bytes necessários
		if (stream->CanRead(sizeof(hInt32)) == false)
			return false;

		//basta mandar ler
		if (stream->Read(&outBuffer, sizeof(hInt32)) != sizeof(hInt32))
			return false;
		return true;
	}
	bool ReadInt64(hInt64 &outBuffer)
	{
		//se não posso ler os bytes necessários
		if (stream->CanRead(sizeof(hInt64)) == false)
			return false;

		//basta mandar ler
		if (stream->Read(&outBuffer, sizeof(hInt64)) != sizeof(hInt64))
			return false;
		return true;
	}
	bool ReadDouble(double &outBuffer)
	{
		//se não posso ler os bytes necessários
		if (stream->CanRead(sizeof(double)) == false)
			return false;

		//basta mandar ler
		if (stream->Read(&outBuffer, sizeof(double)) != sizeof(double))
			return false;
		return true;
	}
	int ReadDoubles(double * const outBuffer, int numDoubles)
	{
		int contagem;

		//se nem posso ler
		if (stream->CanRead() == false)
			return 0;

		//começo a preparar para ler os doubles
		contagem = 0;
		while((stream->CanRead(sizeof(double)) == true) && (contagem < numDoubles))
		{
			//tento ler e se não consegui, saio
			if (stream->Read(outBuffer + contagem, sizeof(double)) != sizeof(double))
				break;

			//próximo
			contagem++;
		}

		//li este número
		return contagem;
	}
	bool ReadFloat(float &outBuffer)
	{
		//se não posso ler os bytes necessários
		if (stream->CanRead(sizeof(float)) == false)
			return false;

		//basta mandar ler
		if (stream->Read(&outBuffer, sizeof(float)) != sizeof(float))
			return false;
		return true;
	}
	int ReadFloats(float * const outBuffer, int numFloats)
	{
		int contagem;

		//se nem posso ler
		if (stream->CanRead() == false)
			return 0;

		//começo a preparar para ler os floats
		contagem = 0;
		while((stream->CanRead(sizeof(float)) == true) && (contagem < numFloats))
		{
			//tento ler e se não consegui, saio
			if (stream->Read(outBuffer + contagem, sizeof(float)) != sizeof(float))
				break;

			//próximo
			contagem++;
		}

		//li este número
		return contagem;
	}
	int Read(void * const outBuffer, int numBytes)
	{
		//basta mandar ler
		return (stream->Read(outBuffer, numBytes));
	}
	int ReadLine(void * const outBuffer, const int bufferSize)
	{
		//basta mandar ler
		return (stream->ReadLine(outBuffer, bufferSize));
	}
	int ReadLineString(HorseRadish::String &targetString, const HorseRadish::String::Encoding inputEncoding)
	{
		char lineBuffer[1024 * 2];

		//por omissão
		targetString.SetEmpty();

		//leio o que tiver a ler
		auto bytesLidos = stream->ReadLine(lineBuffer, sizeof(lineBuffer));
		if ((bytesLidos < 0) || (bytesLidos >= sizeof(lineBuffer)))
			return -bytesLidos;

		//fecho a string e converto
		lineBuffer[bytesLidos] = '\0';
		targetString.Set(inputEncoding, lineBuffer);

		//posso sair
		return bytesLidos;
	}
	int ReadUntil(void * const outBuffer, const int bufferSize, const char goal)
	{
		//basta mandar ler
		return (stream->ReadUntil(outBuffer, bufferSize, goal));
	}
	const void* ReadContent(int &contentSize, bool &contentCopied) const
	{
		//basta mandar ler
		return (stream->ReadContent(contentSize, contentCopied));
	}
	int GetPosition() const
	{
		//basta mandar ler a posição
		return (stream->GetPosition());
	}
	bool CanRead() const
	{
		//basta devolver isto
		return (stream->CanRead());
	}
	int Seek(const int offset)
	{
		//basta mandar mover
		return (stream->Seek(offset, Stream::Current));
	}
	int Seek(const int offset, const Stream::SeekOrigin seekOrigin)
	{
		//basta mandar mover
		return (stream->Seek(offset, seekOrigin));
	}
};

class StreamWriter
{
    Stream *stream;
public:
	
    StreamWriter(Stream * const stream) { this->stream = stream; }
	~StreamWriter() { }

	bool WriteInt8(const hInt8 &inValue)
	{
		//se nem posso escrever
		if (stream->CanWrite() == false)
			return 0;

		//basta mandar escrever
		return (stream->Write(&inValue, sizeof(hInt8)) == sizeof(hInt8));
	}
	bool WriteInt16(const hInt16 &inValue)
	{
		//se nem posso escrever
		if (stream->CanWrite() == false)
			return 0;

		//basta mandar escrever
		return (stream->Write(&inValue, sizeof(hInt16)) == sizeof(hInt16));
	}
	bool WriteInt32(const hInt32 &inValue)
	{
		//se nem posso escrever
		if (stream->CanWrite() == false)
			return 0;

		//basta mandar escrever
		return (stream->Write(&inValue, sizeof(hInt32)) == sizeof(hInt32));
	}
	bool WriteInt64(const hInt64 &inValue)
	{
		//se nem posso escrever
		if (stream->CanWrite() == false)
			return 0;

		//basta mandar escrever
		return (stream->Write(&inValue, sizeof(hInt64)) == sizeof(hInt64));
	}
	bool WriteDouble(const double &inValue)
	{
		//se nem posso escrever
		if (stream->CanWrite() == false)
			return 0;

		//basta mandar escrever
		return (stream->Write(&inValue, sizeof(double)) == sizeof(double));
	}
	int WriteDoubles(const double * const inBuffer, int numDoubles)
	{
		//se nem posso escrever
		if (stream->CanWrite() == false)
			return 0;

		//verifico isto
		if (numDoubles <= 0)
			return 0;

		//para cada um que tenho de gravar
		for(int i=0; i<numDoubles; i++)
		{
			//escrevo este double e se falhar, posso já sair
			if (stream->Write(inBuffer+i, sizeof(double)) != sizeof(double))
				return i;
		}

		//chegando aqui gravei tudo
		return numDoubles;
	}
	bool WriteFloat(const float &inValue)
	{
		//se nem posso escrever
		if (stream->CanWrite() == false)
			return 0;

		//basta mandar escrever
		return (stream->Write(&inValue, sizeof(float)) == sizeof(float));
	}
	int WriteFloats(const float * const inBuffer, int numFloats)
	{
		//se nem posso escrever
		if (stream->CanWrite() == false)
			return 0;

		//verifico isto
		if (numFloats <= 0)
			return 0;

		//para cada um que tenho de gravar
		for(int i=0; i<numFloats; i++)
		{
			//escrevo este float e se falhar, posso já sair
			if (stream->Write(inBuffer+i, sizeof(float)) != sizeof(float))
				return i;
		}

		//chegando aqui gravei tudo
		return numFloats;
	}
	int Write(const void * const inBuffer, int numBytes)
	{
		//se nem posso escrever
		if (stream->CanWrite() == false)
			return 0;

		//basta mandar escrever
		return stream->Write(inBuffer, numBytes);
	}
	int WriteString(const char * const stringData, bool includeTerminator)
	{
		const hInt32 *stringIntWalker;
		const char *stringWalker;

		//se nem posso escrever
		if (stream->CanWrite() == false)
			return 0;

		//tento copiar a 4 caracteres de uma vez
		stringIntWalker = (const hInt32*)stringData;
		for(; Math::iHasZero(*stringIntWalker) == false; stringIntWalker++)
			stream->Write(stringIntWalker, sizeof(hInt32));

		//basta mandar escrever os restantes caracteres
		stringWalker = (const char *)stringIntWalker;
		for(; *stringWalker != '\0'; stringWalker++)
			stream->Write(stringWalker, 1);

		//se for para escrever o '\0'
		if (includeTerminator == true)
			stream->Write(stringWalker, 1);
	}
	int GetPosition() const
	{
		//basta mandar ler a posição
		return (stream->GetPosition());
	}
	int Seek(const int offset)
	{
		//basta mandar mover
		return (stream->Seek(offset, Stream::Current));
	}
	int Seek(const int offset, const Stream::SeekOrigin seekOrigin)
	{
		//basta mandar mover
		return (stream->Seek(offset, seekOrigin));
	}
};

class TextWriter
{
    Stream *stream;
	
public:
	
    TextWriter(Stream * const stream) { this->stream = stream; }
	~TextWriter() { }

	bool Write(const char * const string, bool writeLine = false)
	{
		//se não tenho nada de jeito
		if ((string == nullptr) || (string[0] == '\0'))
			return 0;

		//basta mandar escrever o número de caracteres da string
		return (this->Write(string, strlen(string), writeLine));
	}

	bool Write(const char * const string, int bytesToWrite, bool writeLine = false)
	{
		bool sucesso;

		//se nem posso escrever
		if (stream->CanWrite() == false)
			return 0;

		//se não tenho nada de jeito
		if ((string == nullptr) || (bytesToWrite <= 0))
			return 0;

		//basta mandar escrever
		sucesso = (stream->Write(string, bytesToWrite) == bytesToWrite);

		//se teve sucesso, escrevo ainda a linha
		if ((sucesso == true) && (writeLine == true))
			sucesso &= (stream->Write(HorseRadish::Platform::NewLine, HorseRadish::Platform::NewLineSize) == HorseRadish::Platform::NewLineSize);

		//devolvo se escrevi tudo como deve de ser
		return sucesso;
	}

	bool WriteFormat(const char * const format, ...)
	{
		va_list ap;
		char resultadoPrintf[512];

		//se não tenho nada de jeito
		if ((format == nullptr) || (format <= 0))
			return 0;

		//mando descodificar a mensagem
		va_start(ap, format);
			vsnprintf_s(resultadoPrintf, sizeof(resultadoPrintf), _TRUNCATE, format, ap);
		va_end(ap);

		//mando gravar a string
		return (this->Write(resultadoPrintf));
	}

	void WriteLine()
	{
		//basta mandar escrever a nova linha
		stream->Write(HorseRadish::Platform::NewLine, HorseRadish::Platform::NewLineSize);
	}
};

}//namespace Streams
}//namespace HorseRadish

#endif