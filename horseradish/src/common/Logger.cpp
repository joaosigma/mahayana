#include "Logger.hpp"
#include "Stream.hpp"
#include "Hashing.hpp"

#include <stdlib.h>

namespace HorseRadish
{

namespace Logging
{

const Logger::EntryHeader* Logger::getNextEntryHeader(const Logger::EntryHeader *entryHeader)
{
	const Logger::EntryFooter *entryFooter;

	//se não tenho nada
	if (entryHeader == nullptr)
		return nullptr;

	//já posso calcular o header correcto
	return ((const Logger::EntryHeader*)(((const char *)entryHeader) + entryHeader->entrySize));
}

const Logger::EntryHeader* Logger::getPreviousEntryHeader(const Logger::EntryHeader *entryHeader)
{
	const Logger::EntryFooter *entryFooter;

	//se não tenho nada
	if (entryHeader == nullptr)
		return nullptr;

	//preciso do footer
	entryFooter = (const Logger::EntryFooter*)(((const char *)entryHeader) - sizeof(Logger::EntryFooter));

	//já posso calcular o header correcto
	return ((const Logger::EntryHeader*)(((const char *)entryHeader) - entryFooter->entrySize));
}

const void* Logger::getEntryMetada(const EntryHeader *entryHeader)
{
	//se não tenho nada
	if (entryHeader == nullptr)
		return nullptr;

	//basta devolver isto
	return (((const char *)entryHeader) + sizeof(Logger::EntryHeader));
}

const void* Logger::getEntryData(const EntryHeader *entryHeader)
{
	//se não tenho nada
	if (entryHeader == nullptr)
		return nullptr;

	//basta devolver isto
	return (((const char *)entryHeader) + sizeof(Logger::EntryHeader) + entryHeader->metadataSize);
}

int Logger::getEntryMetadataSize(const EntryHeader *entryHeader)
{
	//se não tenho nada
	if (entryHeader == nullptr)
		return 0;

	//basta fazer estas contas
	return (entryHeader->metadataSize);
}

int Logger::getEntryDataSize(const EntryHeader *entryHeader)
{
	//se não tenho nada
	if (entryHeader == nullptr)
		return 0;

	//basta fazer estas contas
	return (entryHeader->entrySize - sizeof(Logger::EntryHeader) - sizeof(Logger::EntryFooter) - entryHeader->metadataSize);
}

int Logger::getUsedSpace() const
{
	//simples continha
	return ( ((const char*)this->dataNext) - ((const char*)this->dataMain));
}

int Logger::getFreeSpace() const
{
	//simples continha
	return ( ((const char*)this->dataEnd) - ((const char*)this->dataNext));
}

void Logger::writeToFile(bool resetData)
{
	//se não tenho nada para escrever
	if ((this->dataMain >= this->dataNext) || (this->filePath.IsEmpty() == true))
	{
		//se tenho de fazer reset aos dados
		if (resetData == true)
		{
			this->numberCurrentEntries = 0;
			this->dataNext = this->dataMain;
		}

		//posso já sair
		return;
	}

	{
		//para onde vou escrever
		HorseRadish::Streams::FileStream fileStream(this->filePath, false, true);
	
		{
			const Logger::EntryHeader *curEntry;
			char auxBuffer[256];

			//o stream writer
			HorseRadish::Streams::StreamWriter streamWriter(&fileStream);

			//o início
			curEntry = (const Logger::EntryHeader*)this->dataMain;

			//para todas as entradas que tenho
			for(; curEntry < this->dataNext; curEntry = ((const Logger::EntryHeader*)(((const char *)curEntry) + curEntry->entrySize)))
			{
				//conforme o tipo
				switch (curEntry->entryType)
				{
					case Logger::EntryType::Error:
						streamWriter.WriteString("error\t{", false);
						break;
					case Logger::EntryType::Info:
						streamWriter.WriteString("info\t{", false);
						break;
					case Logger::EntryType::Warning:
						streamWriter.WriteString("warning\t{", false);
						break;
					default:
						streamWriter.WriteString("normal\t{", false);
						break;
				}
				
				//converto o tempo para algo legível e escrevo (o último caracter é \n portanto omito-o)
				ctime_s(auxBuffer, sizeof(auxBuffer), &curEntry->entryTimestamp);
				streamWriter.Write(auxBuffer, strlen(auxBuffer) - 1);
				streamWriter.WriteString("}\t", false);

				//posso escrever o conteúdo da entrada
				streamWriter.WriteInt8('{');
				streamWriter.Write(Logger::getEntryData(curEntry), Logger::getEntryDataSize(curEntry) - 1);
				streamWriter.WriteInt8('}');

				//mudo de linha
				streamWriter.Write(HorseRadish::Platform::NewLine, HorseRadish::Platform::NewLineSize);
			}
		}
	}

	//se tenho de fazer reset aos dados
	if (resetData == true)
	{
		this->numberCurrentEntries = 0;
		this->dataNext = this->dataMain;
	}

	//como escrevi para o ficheiro, se for preciso escrever outra vez, tenho de fazer append
	appendToFile = true;
}

bool Logger::addEntry(const EntryType &entryType, const char * const entryData, const void * const metadata, int metadataSize)
{
	int entryDataSize, entryTotalSize;
	EntryHeader newEntryHeader;
	EntryFooter newEntryFooter;

	//verifica parametros
	if ((this->dataNext == nullptr) || (entryData == nullptr) || (entryData[0] == '\0'))
		return false;

	//preciso do tamanho da frase (mais um para o '\0')
	entryDataSize = strlen(entryData) + 1;
	if (entryDataSize <= 0)
		return false;

	//evitar problemas
	if ((metadataSize < 0) || (metadata == nullptr))
		metadataSize = 0;

	//posso já calcular o espaço total de que necessito
	entryTotalSize = sizeof(EntryHeader) + metadataSize + entryDataSize + sizeof(EntryFooter);

	//se não tenho espaço
	if (this->getFreeSpace() < entryTotalSize)
		this->writeToFile(true);

	//se ainda não tenho espaço
	if (this->getFreeSpace() < entryTotalSize)
		return false;

	//preciso de saber a data desta entrada
	time(&newEntryHeader.entryTimestamp);

	//preciso de saber qual o tamanho da entrada anterior
	if (this->dataNext <= this->dataMain)
		newEntryHeader.prevEntrySize = 0;
	else
		newEntryHeader.prevEntrySize = Logger::getPreviousEntryHeader((const EntryHeader*)this->dataNext)->entrySize;

	//preparo os restantes dados do cabeçalho
	newEntryHeader.entrySize = entryTotalSize;
	newEntryHeader.metadataSize = metadataSize;
	newEntryHeader.entryType = entryType;
	newEntryHeader.entryCRC8 = 0;
	newEntryHeader.entryCRC8 = HorseRadish::Hashing::CalculateCRC8(&newEntryHeader, sizeof(newEntryHeader));

	//preparo os dados do parte final
	newEntryFooter.entrySize = newEntryHeader.entrySize;
	newEntryFooter.entryCRC8 = newEntryHeader.entryCRC8;

	//basta gravar tudo
	memcpy(this->dataNext, &newEntryHeader, sizeof(EntryHeader));
	if (metadataSize > 0)
		memcpy(((char *)this->dataNext) + sizeof(EntryHeader), metadata, metadataSize);
	memcpy(((char *)this->dataNext) + sizeof(EntryHeader) + metadataSize, entryData, entryDataSize);
	memcpy(((char *)this->dataNext) + sizeof(EntryHeader) + metadataSize + entryDataSize, &newEntryFooter, sizeof(EntryFooter));

	//ajusto o ponteiro para a próxima escrita
	this->dataNext = ((char *)this->dataNext) + newEntryHeader.entrySize;

	//como meti mais uma entrada
	this->numberTotalEntries++;
	this->numberCurrentEntries++;

	//correu tudo bem
	return true;
}

Logger::Logger(int logCapacityKB)
{
	int allocSize;

	//limpo isto
	this->dataMain = this->dataEnd = this->dataNext = nullptr;
	this->numberTotalEntries = this->numberCurrentEntries = 0;
	this->filePath.Clear();
	this->appendToFile = false;

	//pra simplificar a vida
	allocSize = ((logCapacityKB <= 0) ? Logger::DefaultCapacityKB : logCapacityKB) * 1024;

	//crio o buffer com o tamanho indicado
	this->dataMain = malloc(allocSize);
	if (this->dataMain == nullptr)
		return;

	//ajusto os ponteiros
	this->dataNext = this->dataMain;
	this->dataEnd = ((char * const)this->dataMain) + allocSize;
}

Logger::Logger(int logCapacityKB, const HorseRadish::IO::Path &filePath, bool appendToFile)
{
	int allocSize;

	//limpo isto
	this->dataMain = this->dataEnd = this->dataNext = nullptr;
	this->numberTotalEntries = this->numberCurrentEntries = 0;
	
	//guardo isto
	this->filePath.Set(filePath);
	this->appendToFile = appendToFile;

	//pra simplificar a vida
	allocSize = ((logCapacityKB <= 0) ? Logger::DefaultCapacityKB : logCapacityKB) * 1024;

	//crio o buffer com o tamanho indicado
	this->dataMain = malloc(allocSize);
	if (this->dataMain == nullptr)
		return;

	//ajusto os ponteiros
	this->dataNext = this->dataMain;
	this->dataEnd = ((char * const)this->dataMain) + allocSize;
}

Logger::~Logger()
{
	//escrevo se for preciso
	this->writeToFile(true);

	//se tiver alguma a libertar
	if (this->dataMain != nullptr)
		free(this->dataMain);

	//limpo tudo
	this->dataMain = this->dataEnd = this->dataNext = nullptr;
	this->numberTotalEntries = this->numberCurrentEntries = 0;
	this->filePath.Clear();
	this->appendToFile = false;
}

void Logger::Log(const EntryType &entryType, const char * const entryData)
{
	//sem isto não vale a pena fazer nada
	if ((this->dataMain == nullptr) || (entryData == nullptr))
		return;

	{
		//para garantir acesso exclusivo
		HorseRadish::Threadding::LockAccess lock(this->syncLock);
		
		//basta chamar isto
		this->addEntry(entryType, entryData, nullptr, 0);
	}
}

void Logger::Log(const EntryType &entryType, const void * const metadata, const int metadataSize, const char * const entryData)
{
	//sem isto não vale a pena fazer nada
	if ((this->dataMain == nullptr) || (entryData == nullptr))
		return;

	{
		//para garantir acesso exclusivo
		HorseRadish::Threadding::LockAccess lock(this->syncLock);

		//basta chamar isto
		this->addEntry(entryType, entryData, metadata, metadataSize);
	}
}

void Logger::Log(const char * const entryData)
{
	//sem isto não vale a pena fazer nada
	if ((this->dataMain == nullptr) || (entryData == nullptr))
		return;

	{
		//para garantir acesso exclusivo
		HorseRadish::Threadding::LockAccess lock(this->syncLock);

		//basta chamar isto
		this->addEntry(EntryType::Normal, entryData, nullptr, 0);
	}
}

void Logger::Log(const void * const metadata, const int metadataSize, const char * const entryData)
{
	//sem isto não vale a pena fazer nada
	if ((this->dataMain == nullptr) || (entryData == nullptr))
		return;

	{
		//para garantir acesso exclusivo
		HorseRadish::Threadding::LockAccess lock(this->syncLock);

		//basta chamar isto
		this->addEntry(EntryType::Normal, entryData, metadata, metadataSize);
	}
}

void Logger::LogInfo(const char * const entryData)
{
	//sem isto não vale a pena fazer nada
	if ((this->dataMain == nullptr) || (entryData == nullptr))
		return;

	{
		//para garantir acesso exclusivo
		HorseRadish::Threadding::LockAccess lock(this->syncLock);

		//basta chamar isto
		this->addEntry(EntryType::Info, entryData, nullptr, 0);
	}
}

void Logger::LogInfo(const void * const metadata, const int metadataSize, const char * const entryData)
{
	//sem isto não vale a pena fazer nada
	if ((this->dataMain == nullptr) || (entryData == nullptr))
		return;

	{
		//para garantir acesso exclusivo
		HorseRadish::Threadding::LockAccess lock(this->syncLock);

		//basta chamar isto
		this->addEntry(EntryType::Info, entryData, metadata, metadataSize);
	}
}

void Logger::LogWarning(const char * const entryData)
{
	//sem isto não vale a pena fazer nada
	if ((this->dataMain == nullptr) || (entryData == nullptr))
		return;

	{
		//para garantir acesso exclusivo
		HorseRadish::Threadding::LockAccess lock(this->syncLock);

		//basta chamar isto
		this->addEntry(EntryType::Warning, entryData, nullptr, 0);
	}
}

void Logger::LogWarning(const void * const metadata, const int metadataSize, const char * const entryData)
{
	//sem isto não vale a pena fazer nada
	if ((this->dataMain == nullptr) || (entryData == nullptr))
		return;

	{
		//para garantir acesso exclusivo
		HorseRadish::Threadding::LockAccess lock(this->syncLock);

		//basta chamar isto
		this->addEntry(EntryType::Warning, entryData, metadata, metadataSize);
	}
}

void Logger::LogError(const char * const entryData)
{
	//sem isto não vale a pena fazer nada
	if ((this->dataMain == nullptr) || (entryData == nullptr))
		return;

	{
		//para garantir acesso exclusivo
		HorseRadish::Threadding::LockAccess lock(this->syncLock);

		//basta chamar isto
		this->addEntry(EntryType::Error, entryData, nullptr, 0);
	}
}

void Logger::LogError(const void * const metadata, const int metadataSize, const char * const entryData)
{
	//sem isto não vale a pena fazer nada
	if ((this->dataMain == nullptr) || (entryData == nullptr))
		return;

	{
		//para garantir acesso exclusivo
		HorseRadish::Threadding::LockAccess lock(this->syncLock);

		//basta chamar isto
		this->addEntry(EntryType::Error, entryData, metadata, metadataSize);
	}
}

void Logger::Reset()
{
	//sem isto não vale a pena fazer nada
	if (this->dataMain == nullptr)
		return;

	{
		//para garantir acesso exclusivo
		HorseRadish::Threadding::LockAccess lock(this->syncLock);

		//basta colocar a próxima escrita para o início (eliminando todos os restantes)
		this->dataNext = this->dataMain;

		//deixo de ter entradas actuais
		this->numberCurrentEntries = 0;
	}
}

void Logger::WriteToFile(bool resetData)
{
	//sem isto não vale a pena fazer nada
	if (this->dataMain == nullptr)
		return;

	{
		//para garantir acesso exclusivo
		HorseRadish::Threadding::LockAccess lock(this->syncLock);

		//basta chamar esta função
		this->writeToFile(resetData);
	}
}

void Logger::Iterate(bool fromBottom, std::function<bool (const char * const data, const int dataSize)> funcCallback, int numberOffset) const
{
	//sem isto não vale a pena fazer nada
	if ((this->dataMain == nullptr) || (funcCallback == nullptr))
		return;

	{
		//para garantir acesso exclusivo
		HorseRadish::Threadding::LockAccess lock(this->syncLock);

		//se não tenho nada
		if (this->dataNext <= this->dataMain)
			return;

		//se for de baixo para cima
		if (fromBottom == true)
		{
			const EntryHeader *entryWalker;

			//começo no fim
			entryWalker = (const EntryHeader*)this->dataNext;
			entryWalker = Logger::getPreviousEntryHeader(entryWalker);

			//até chegar ao inicio
			for(; entryWalker >= this->dataMain; entryWalker = Logger::getPreviousEntryHeader(entryWalker))
			{
				//se tenho de passar à frente de alguns
				if (numberOffset > 0)
				{
					numberOffset--;
					continue;
				}

				//chamo o callback e se pediram para sair
				if (funcCallback((const char*)Logger::getEntryData(entryWalker), Logger::getEntryDataSize(entryWalker)) == false)
					break;
			}
		}
		else
		{
			const EntryHeader *entryWalker;

			//começo no início
			entryWalker = (const EntryHeader*)this->dataMain;

			//até chegar ao fim
			for(; entryWalker < this->dataNext; entryWalker = Logger::getNextEntryHeader(entryWalker))
			{
				//se tenho de passar à frente de alguns
				if (numberOffset > 0)
				{
					numberOffset--;
					continue;
				}

				//chamo o callback e se pediram para sair
				if (funcCallback((const char*)Logger::getEntryData(entryWalker), Logger::getEntryDataSize(entryWalker)) == false)
					break;
			}
		}
	}
}

void Logger::Iterate(bool fromBottom, std::function<bool (const void * const metadata, const int metadataSize, const char * const data, const int dataSize)> funcCallback, int numberOffset) const
{
	//sem isto não vale a pena fazer nada
	if ((this->dataMain == nullptr) || (funcCallback == nullptr))
		return;

	{
		//para garantir acesso exclusivo
		HorseRadish::Threadding::LockAccess lock(this->syncLock);

		//se não tenho nada
		if (this->dataNext <= this->dataMain)
			return;

		//se for de baixo para cima
		if (fromBottom == true)
		{
			const EntryHeader *entryWalker;

			//começo no fim
			entryWalker = (const EntryHeader*)this->dataNext;
			entryWalker = Logger::getPreviousEntryHeader(entryWalker);

			//até chegar ao inicio
			for(; entryWalker >= this->dataMain; entryWalker = Logger::getPreviousEntryHeader(entryWalker))
			{
				//se tenho de passar à frente de alguns
				if (numberOffset > 0)
				{
					numberOffset--;
					continue;
				}

				//chamo o callback e se pediram para sair
				if (funcCallback(Logger::getEntryMetada(entryWalker), Logger::getEntryMetadataSize(entryWalker), (const char*)Logger::getEntryData(entryWalker), Logger::getEntryDataSize(entryWalker)) == false)
					break;
			}
		}
		else
		{
			const EntryHeader *entryWalker;

			//começo no início
			entryWalker = (const EntryHeader*)this->dataMain;

			//até chegar ao fim
			for(; entryWalker < this->dataNext; entryWalker = Logger::getNextEntryHeader(entryWalker))
			{
				//se tenho de passar à frente de alguns
				if (numberOffset > 0)
				{
					numberOffset--;
					continue;
				}

				//chamo o callback e se pediram para sair
				if (funcCallback(Logger::getEntryMetada(entryWalker), Logger::getEntryMetadataSize(entryWalker), (const char*)Logger::getEntryData(entryWalker), Logger::getEntryDataSize(entryWalker)) == false)
					break;
			}
		}
	}
}

//o tamanho por omissão para os logs (1 MB)
const int Logger::DefaultCapacityKB = 1024;

}//namespace Logging
}//namespace HorseRadish