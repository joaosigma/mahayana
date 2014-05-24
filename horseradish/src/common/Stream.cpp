#include "Stream.hpp"
#include "ScopedAction.hpp"
#include "UTF.hpp"

#include <malloc.h>
#include <memory.h>

namespace HorseRadish
{
namespace Streams
{

//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§	 Stream class	§§§§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§

//tenho de declarar um corpo ao destructor
Stream::~Stream() 
{}

//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§	 MemoryStream class	§§§§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
MemoryStream::MemoryStream(const void * const bufferData, int bufferSize, bool canWrite, const MemoryStream::ManagementType &managementType)
{
	//guardo simplesmente isto tudo
	this->data = bufferData;
	this->dataSize = bufferSize;
	this->canWrite = canWrite;
	this->managementType = managementType;

	//ajusto algumas coisas
	if ((this->data == nullptr) || (this->dataSize < 0))
		this->dataSize = 0;

	//não estou fechado
	this->closed = false;

	//ajusto este ponteiros
	this->dataBegin = reinterpret_cast<const hUInt8*>(this->data);
	this->dataEnd = this->dataBegin + this->dataSize;
	this->dataWalker = this->dataBegin;
}
MemoryStream::~MemoryStream()
{
	//se ainda não fechei, fecho
	if (this->closed == false)
		this->Close();
}

void MemoryStream::Close()
{
	//como é para fechar
	this->closed = true;

	//se for para gerir a memória
	if ((this->managementType != ManagementType::None) && (this->data != nullptr))
		free((void*)this->data);

	//limpo o resto
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
	//se já fechou
	if (closed == true)
		return false;

	//posso ler se ainda não cheguei ao fim
	return (this->dataWalker < this->dataEnd);
}
bool MemoryStream::CanRead(int numBytes) const
{
	//se nem sequer posso ler
	if (this->CanRead() == false)
		return false;

	//não dá jeito
	if (numBytes < 0)
		return false;

	//basta verificar quanto posso ler
	return (numBytes <= (this->dataEnd - this->dataWalker));
}
bool MemoryStream::CanWrite() const
{
	//nunca posso escrever
	return this->canWrite;
}
bool MemoryStream::CanWrite(int numBytes) const
{
	//nunca posso escrever
	return this->canWrite;
}
int MemoryStream::GetLength() const
{
	//basta devolver isto
	return dataSize;
}
int MemoryStream::GetPosition() const
{
	//basta fazer estas contas
	return (this->dataWalker - this->dataBegin);
}

int MemoryStream::Read(void * const outBuffer, int numBytes)
{
	//cuidado para não ler mais do que devia
	if (numBytes > (this->dataEnd - this->dataWalker))
		numBytes = (this->dataEnd - this->dataWalker);

	//se não for para ler nada
	if (numBytes <= 0)
		return 0;
	
	//copio os bytes, avanço o ponteiro e já está
	memcpy(outBuffer, this->dataWalker, numBytes);
	this->dataWalker += numBytes;
	return numBytes;
}

int MemoryStream::ReadLine(void * const outBuffer, const int bufferSize)
{
	int lerMesmo;
	char *outBufferWalker;

	//verificar isto
	if ((outBuffer == nullptr) || (bufferSize <= 0))
		return 0;

	//se nem sequer posso ler nada
	if (this->CanRead() == false)
		return 0;

	//vou precisar disto
	outBufferWalker = reinterpret_cast<char*>(outBuffer);
	
	//tenho de ter cuidado com o numero de bytes que vou ler para nao 
	//arrebentar o buffer do ficheiro
	lerMesmo = 0;
	do{
		//se estou numa nova linha
		if (*this->dataWalker == '\n')
		{
			//ignoro a própria linha e posso sair
			this->dataWalker++;
			return lerMesmo;
		}

		//se estou numa nova linha
		if (*this->dataWalker == '\r')
		{
			//ignoro a própria linha
			this->dataWalker++;

			//se ainda posso ler
			if (this->dataWalker < this->dataEnd)
			{
				//se ainda estou numa linha (o caso \r\n)
				if (*this->dataWalker == '\n')
					this->dataWalker++;
			}

			//posso sair
			return lerMesmo;
		}

		//leio este
		*outBufferWalker = *this->dataWalker;

		//avanço para a frente e tranco a string
		lerMesmo++;
		this->dataWalker++;

		//se cheguei ao fim do buffer, tenho de indicar que não tenho mais espaço
		if (lerMesmo >= bufferSize)
			return -lerMesmo;

		//para onde vou escrever o próximo
		outBufferWalker++;
	}while(this->dataWalker < this->dataEnd);

	//devolvo somente os que li
	//mas chegando aqui foi por ter chegado ao fim do ficheiro, logo devolvo negativo
	return -lerMesmo;
}

int MemoryStream::ReadUntil(void * const outBuffer, const int bufferSize, const char goal)
{
	int lerMesmo;
	char *outBufferWalker;

	//verificar isto
	if ((outBuffer == nullptr) || (bufferSize <= 0))
		return 0;

	//se nem sequer posso ler nada
	if (this->CanRead() == false)
		return 0;

	//vou precisar disto
	outBufferWalker = reinterpret_cast<char*>(outBuffer);
	
	//tenho de ter cuidado com o numero de bytes que vou ler para nao 
	//arrebentar o buffer do ficheiro
	lerMesmo = 0;
	do{
		//leio este
		*outBufferWalker = *this->dataWalker;

		//avanço para a frente e tranco a string
		lerMesmo++;
		this->dataWalker++;

		//se li o caracter final
		if (*outBufferWalker == goal)
			return lerMesmo;

		//se cheguei ao fim do buffer, tenho de indicar que não tenho mais espaço
		if (lerMesmo >= bufferSize)
			return -lerMesmo;

		//para onde vou escrever o próximo
		outBufferWalker++;
	}while(this->dataWalker < this->dataEnd);

	//devolvo somente os que li
	//mas chegando aqui foi por ter chegado ao fim do ficheiro, logo devolvo negativo
	return -lerMesmo;
}
const void* MemoryStream::ReadContent(int &contentSize, bool &contentCopied) const
{
	//gravo o tamanho
	contentSize = this->dataSize;

	//como não estou a fazer uma cópia
	contentCopied = false;

	//e posso devolver o buffer
	return this->data;
}
int MemoryStream::Write(const void * const inBuffer, int numBytes)
{
	//se não posso escrever
	if (this->canWrite == false)
		return 0;

	//gravo os bytes, avanço o ponteiro e já está
	memcpy((void*)this->dataWalker, inBuffer, numBytes);
	this->dataWalker += numBytes;
	return numBytes;
}
int MemoryStream::Seek(const int offset, const SeekOrigin seekOrigin)
{
	//efectuo sempre a operação, independente do offset e tipo
	if (seekOrigin == Stream::Begin)
		this->dataWalker = this->dataBegin + offset;
	else if (seekOrigin == Stream::End)
		this->dataWalker = this->dataEnd + offset;
	else if (seekOrigin == Stream::Current)
		this->dataWalker += offset;

	//certifico-me que não saio do buffer
	if (this->dataWalker < this->dataBegin)
		this->dataWalker = this->dataBegin;
	if (this->dataWalker > this->dataEnd)
		this->dataWalker = this->dataEnd;

	//devolvo a posição onde está
	return this->GetPosition();
}

//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§	 FileStream class	§§§§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§*/
bool FileStream::openFile(const HorseRadish::hChar * const filePath, bool toRead, bool toWrite)
{
	wchar_t filePathWChar[128];

	//guardo isto
	this->toRead = toRead;
	this->toWrite = toWrite;

	//assumo que não estou fechado nem tenho nehum ponteiro para qualquer ficheiro
	this->closed = false;
	this->fileHandle = nullptr;

	//se não é para fazer nada
	if ((toRead == false) && (toWrite == false))
		return false;

	//tenho de converter a string para WideChar
	HorseRadish::UTF::ConvertUTF8To(filePath, HorseRadish::UTF::Windows, filePathWChar, sizeof(filePathWChar));

	//tento abrir o ficheiro, conforme o tipo pedido
	if ((toRead == true) && (toWrite == true))
		this->fileHandle = CreateFile(filePathWChar, GENERIC_READ | GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	else if (toRead == true)
		this->fileHandle = CreateFile(filePathWChar, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	else if (toWrite == true)
		this->fileHandle = CreateFile(filePathWChar, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

	//se não tenho nada de jeito
	if (this->fileHandle == INVALID_HANDLE_VALUE)
	{
		this->fileHandle = nullptr;
		return false;
	}

	//tá tudo bem
	return true;
}

FileStream::FileStream(const HorseRadish::hChar * const filePath, bool toRead, bool toWrite)
{
	//por omissão
	this->toRead = this->toWrite = false;
	this->closed = false;
	this->fileHandle = nullptr;

	//abro o ficheiro
	this->openFile(filePath, toRead, toWrite);
}
FileStream::~FileStream()
{
	//se ainda não fechei, fecho
	if (this->closed == false)
		this->Close();
}

void FileStream::Close()
{
	//se tiver alguma coisa, fecho-a
	if (this->fileHandle != nullptr)
		CloseHandle(this->fileHandle);

	//como é para fechar
	this->closed = true;

	//limpo o resto
	this->toRead = false;
	this->toWrite = false;
	this->fileHandle = nullptr;
}
void FileStream::Flush()
{
	//se tiver alguma coisa, posso fazer flush dele
	if (this->fileHandle != nullptr)
		FlushFileBuffers(this->fileHandle);
}

bool FileStream::CanRead() const
{
	//se já fechou
	if (closed == true)
		return false;

	//se o ficheiro não foi aberto para leitura
	if (this->toRead == false)
		return false;

	//posso ler se ainda não cheguei ao fim do ficheiro
	return (SetFilePointer(this->fileHandle, 0, nullptr, FILE_CURRENT) < GetFileSize(this->fileHandle, nullptr));
}
bool FileStream::CanRead(int numBytes) const
{
	//se já fechou
	if (closed == true)
		return false;

	//se o ficheiro não foi aberto para leitura
	if (this->toRead == false)
		return false;

	//posso ler se ainda tenho mais bytes dos que os indicados
	return ((GetFileSize(this->fileHandle, nullptr) - SetFilePointer(this->fileHandle, 0, nullptr, FILE_CURRENT)) >= numBytes );
}
bool FileStream::CanWrite() const
{
	//se já fechou
	if (closed == true)
		return false;

	//posso escrever se o ficheiro foi aberto para escrita
	return (this->toWrite);
}
bool FileStream::CanWrite(int numBytes) const
{
	//basta devolver se posso ler
	return (this->CanWrite());
}
int FileStream::GetLength() const
{
	//se não tenho ficheiro
	if (this->fileHandle == nullptr)
		return 0;

	//basta devolver o tamanho dele
	return GetFileSize(this->fileHandle, nullptr);
}
int FileStream::GetPosition() const
{
	//se não tenho ficheiro
	if (this->fileHandle == nullptr)
		return 0;

	//basta devolver o tamanho dele
	return SetFilePointer(this->fileHandle, 0, nullptr, FILE_CURRENT);
}

bool FileStream::IsValid() const
{
	//basta o file handle ser diferente de null
	return (this->fileHandle != nullptr);
}

int FileStream::Read(void * const outBuffer, int numBytes)
{
	DWORD bytesRead;

	//verificar isto
	if ((outBuffer == nullptr) || (numBytes <= 0))
		return 0;

	//se não tenho ficheiro
	if (this->fileHandle == nullptr)
		return -1;

	//posso mandar ler
	if (ReadFile(this->fileHandle, outBuffer, numBytes, &bytesRead, nullptr) == 0)
		return -1;

	//li estes bytes
	return bytesRead;
}

int FileStream::ReadLine(void * const outBuffer, const int bufferSize)
{
	int lerMesmo;
	DWORD bytesRead;
	char dataRead, *outBufferWalker;

	//verificar isto
	if ((outBuffer == nullptr) || (bufferSize <= 0))
		return 0;

	//se nem sequer posso ler nada
	if (this->CanRead() == false)
		return 0;

	//vou precisar disto
	outBufferWalker = reinterpret_cast<char*>(outBuffer);
	
	//tenho de ter cuidado com o numero de bytes que vou ler para nao 
	//arrebentar o buffer do ficheiro
	lerMesmo = 0;
	while(true)
	{
		//leio um caracter
		if (ReadFile(this->fileHandle, &dataRead, 1, &bytesRead, nullptr) == 0)
			break;

		//tiver de ler um byte
		if (bytesRead != 1)
			break;

		//se estou numa nova linha, posso já sair
		if (dataRead == '\n')
			return lerMesmo;

		//se estou numa nova linha
		if (dataRead == '\r')
		{
			//volto a ler um caracter
			if (ReadFile(this->fileHandle, &dataRead, 1, &bytesRead, nullptr) == 0)
				break;

			//tiver de ler um byte
			if (bytesRead != 1)
				break;

			//se não for um '\n', tenho de voltar a trás (não era o caso \r\n)
			if (dataRead != '\n')
				SetFilePointer(this->fileHandle, -1, nullptr, FILE_CURRENT);

			//posso sair
			return lerMesmo;
		}

		//guardo o caracter lido
		*outBufferWalker = dataRead;
		lerMesmo++;

		//se cheguei ao fim do buffer, tenho de indicar que não tenho mais espaço
		if (lerMesmo >= bufferSize)
			return -lerMesmo;

		//para onde vou escrever o próximo
		outBufferWalker++;
	}

	//devolvo somente os que li
	//mas chegando aqui foi por ter chegado ao fim do ficheiro, logo devolvo negativo
	return -lerMesmo;
}

int FileStream::ReadUntil(void * const outBuffer, const int bufferSize, const char goal)
{
	int lerMesmo;
	DWORD bytesRead;
	char *outBufferWalker;

	//verificar isto
	if ((outBuffer == nullptr) || (bufferSize <= 0))
		return 0;

	//se nem sequer posso ler nada
	if (this->CanRead() == false)
		return 0;

	//vou precisar disto
	outBufferWalker = reinterpret_cast<char*>(outBuffer);
	
	//tenho de ter cuidado com o numero de bytes que vou ler para nao 
	//arrebentar o buffer do ficheiro
	lerMesmo = 0;
	while(true)
	{
		//leio um caracter
		if (ReadFile(this->fileHandle, outBufferWalker, 1, &bytesRead, nullptr) == 0)
			break;

		//tiver de ler um byte
		if (bytesRead != 1)
			break;

		//li mais um caracter
		lerMesmo++;

		//se li o caracter final
		if (*outBufferWalker == goal)
			return lerMesmo;

		//se cheguei ao fim do buffer, tenho de indicar que não tenho mais espaço
		if (lerMesmo >= bufferSize)
			return -lerMesmo;

		//para onde vou escrever o próximo
		outBufferWalker++;
	}

	//devolvo somente os que li
	//mas chegando aqui foi por ter chegado ao fim do ficheiro, logo devolvo negativo
	return -lerMesmo;
}

const void* FileStream::ReadContent(int &contentSize, bool &contentCopied) const
{
	int curPos, outBufferSize;
	DWORD bytesRead;
	bool readSuccess;
	void *outBuffer;

	//por omissão
	contentSize = 0;

	//sendo um ficheiro vou fazer sempre uma cópia
	contentCopied = true;

	//se não tenho ficheiro
	if (this->fileHandle == nullptr)
		return nullptr;

	//leio o tamanho do ficheiro e tento alocar espaço para o ler todo
	outBufferSize = GetFileSize(this->fileHandle, nullptr);
	outBuffer = malloc(outBufferSize);
	if (outBuffer == nullptr)
		return nullptr;

	//obtenho a posição actual do ficheiro e coloco-o no inicio
	curPos = SetFilePointer(this->fileHandle, 0, nullptr, FILE_CURRENT);
	SetFilePointer(this->fileHandle, 0, nullptr, FILE_BEGIN);

	//leio tudo
	readSuccess = (ReadFile(this->fileHandle, outBuffer, outBufferSize, &bytesRead, nullptr) != 0);

	//volto a colocar o ficheiro na posição correcta
	SetFilePointer(this->fileHandle, curPos, nullptr, FILE_BEGIN);

	//se falhou a ler ou não li tudo, dá erro
	if ((readSuccess == false) || (outBufferSize != bytesRead))
	{
		free(outBuffer);
		return nullptr;
	}

	//chegando aqui correu tudo bem
	contentSize = outBufferSize;
	return outBuffer;
}

int FileStream::Write(const void * const inBuffer, int numBytes)
{
	DWORD bytesWritten;

	//verificar isto
	if ((inBuffer == nullptr) || (numBytes <= 0))
		return 0;

	//se não tenho ficheiro
	if (this->fileHandle == nullptr)
		return -1;

	//posso mandar escrever
	if (WriteFile(this->fileHandle, inBuffer, numBytes, &bytesWritten, nullptr) == 0)
		return -1;

	//escrevi estes bytes
	return bytesWritten;
}

int FileStream::Seek(const int offset, const SeekOrigin seekOrigin)
{
	//se não tenho ficheiro
	if (this->fileHandle == nullptr)
		return -1;

	//conforme a operação a fazer
	if (seekOrigin == Stream::Begin)
		return SetFilePointer(this->fileHandle, offset, nullptr, FILE_BEGIN);
	if (seekOrigin == Stream::End)
		return SetFilePointer(this->fileHandle, offset, nullptr, FILE_END);
	if (seekOrigin == Stream::Current)
		return SetFilePointer(this->fileHandle, offset, nullptr, FILE_CURRENT);

	//chegando aqui alguma coisa está mal
	return -1;
}

void* FileStream::ReadEntireFile(const HorseRadish::hChar * const filePath, int &fileSize)
{
	HANDLE fileHandle;
	DWORD bytesRead;
	wchar_t filePathWChar[128];

	//por omissão
	fileSize = 0;

	//verifico isto
	if (filePath == nullptr)
		return nullptr;

	//tenho de converter a string para WideChar
	HorseRadish::UTF::ConvertUTF8To(filePath, HorseRadish::UTF::Windows, filePathWChar, sizeof(filePathWChar));

	//tento abrir o ficheiro
	fileHandle = CreateFile(filePathWChar, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (fileHandle == INVALID_HANDLE_VALUE)
		return nullptr;

	//para me certificar que tenho sempre de "fechar" o ficheiro
	ScopedAction scopedAction([&]()
	{
		CloseHandle(fileHandle);
	});

	//leio o tamanho do ficheiro e tento alocar espaço para o ler todo
	auto outBufferSize = GetFileSize(fileHandle, nullptr);
	auto outBuffer = malloc(outBufferSize);
	if (outBuffer == nullptr)
		return nullptr;

	//leio tudo
	if ((ReadFile(fileHandle, outBuffer, outBufferSize, &bytesRead, nullptr) == 0) || (outBufferSize !=bytesRead))
	{
		free(outBuffer);
		return nullptr;
	}

	//chegando aqui correu tudo bem
	fileSize = outBufferSize;
	return outBuffer;
}

HorseRadish::String FileStream::ReadEntireFileAsString(const HorseRadish::hChar * const filePath)
{
	HANDLE fileHandle;
	DWORD bytesRead;
	wchar_t filePathWChar[128];

	//verifico isto
	if (filePath == nullptr)
		return HorseRadish::String();

	//tenho de converter a string para WideChar
	HorseRadish::UTF::ConvertUTF8To(filePath, HorseRadish::UTF::Windows, filePathWChar, sizeof(filePathWChar));

	//tento abrir o ficheiro
	fileHandle = CreateFile(filePathWChar, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (fileHandle == INVALID_HANDLE_VALUE)
		return HorseRadish::String();

	//para me certificar que tenho sempre de "fechar" o ficheiro
	ScopedAction scopedAction([&]()
	{
		CloseHandle(fileHandle);
	});

	//leio o tamanho do ficheiro e tento alocar espaço para o ler todo
	auto outBufferSize = GetFileSize(fileHandle, nullptr);
	auto finalString = HorseRadish::String();

	finalString.Capacity(outBufferSize + 1);

	//leio tudo
	if ((ReadFile(fileHandle, (void*)finalString.GetData(), outBufferSize, &bytesRead, nullptr) == 0) || (outBufferSize != bytesRead))
		return HorseRadish::String();

	//chegando aqui correu tudo bem
	finalString.CloseAt(outBufferSize);
	return finalString;
}

bool FileStream::StreamDump(Stream* stream, const HorseRadish::hChar * const filePath)
{
	int filePos, bytesRead;
	DWORD bytesWritten;
	unsigned char auxBuffer[1024];
	HANDLE fileHandle;
	wchar_t filePathWChar[256];

	//verificar parametros
	if ((stream == nullptr) || (stream->CanRead() == false) || (filePath == nullptr))
		return false;

	//tenho de converter a string para WideChar
	HorseRadish::UTF::ConvertUTF8To(filePath, HorseRadish::UTF::Windows, filePathWChar, sizeof(filePathWChar));

	//tenho de abrir o ficheiro
	fileHandle = CreateFile(filePathWChar, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (fileHandle == nullptr)
		return false;

	//guardo a posição actual e movo o stream para o início
	filePos = stream->GetPosition();
	stream->Seek(0, Stream::Begin);

	//enquanto tenho coisas a ler
	while (true)
	{
		//leio alguns bytes do stream
		bytesRead = stream->Read(auxBuffer, sizeof(auxBuffer));
		if (bytesRead <= 0)
			break;

		//escrevo esses bytes
		WriteFile(fileHandle, auxBuffer, bytesRead, &bytesWritten, nullptr);
		if (bytesWritten != bytesRead)
			break;

		//se já li bytes a menos, já li tudo
		if (bytesRead < sizeof(auxBuffer))
			break;
	}

	//posso fechar o ficheiro
	CloseHandle(fileHandle);

	//reponho a posição anterior do stream
	stream->Seek(filePos, Stream::Begin);

	//chegando aqui correu tudo bem
	return true;
}

}//namespace Streams
}//namespace HorseRadish