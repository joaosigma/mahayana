#include "Logger.hpp"

#include "Stream.hpp"
#include "Hashing.hpp"

#include <stdlib.h>

namespace HorseRadish
{
	namespace Logging
	{
		const int Logger::DefaultCapacityKB = 1024;

		const Logger::EntryHeader* Logger::getNextEntryHeader(const Logger::EntryHeader *entryHeader)
		{
			if (entryHeader == nullptr)
				return nullptr;

			return ((const Logger::EntryHeader*)(reinterpret_cast<const char *>(entryHeader)+entryHeader->entrySize));
		}

		const Logger::EntryHeader* Logger::getPreviousEntryHeader(const Logger::EntryHeader *entryHeader)
		{
			if (entryHeader == nullptr)
				return nullptr;

			auto entryFooter = reinterpret_cast<const Logger::EntryFooter*>(reinterpret_cast<const char *>(entryHeader)-sizeof(Logger::EntryFooter));

			return ((const Logger::EntryHeader*)(((const char *)entryHeader) - entryFooter->entrySize));
		}

		const void* Logger::getEntryMetada(const EntryHeader *entryHeader)
		{
			if (entryHeader == nullptr)
				return nullptr;

			return (reinterpret_cast<const char *>(entryHeader)+sizeof(Logger::EntryHeader));
		}

		const void* Logger::getEntryData(const EntryHeader *entryHeader)
		{
			if (entryHeader == nullptr)
				return nullptr;

			return (reinterpret_cast<const char *>(entryHeader)+sizeof(Logger::EntryHeader) + entryHeader->metadataSize);
		}

		int Logger::getEntryMetadataSize(const EntryHeader *entryHeader)
		{
			if (entryHeader == nullptr)
				return 0;

			return (entryHeader->metadataSize);
		}

		int Logger::getEntryDataSize(const EntryHeader *entryHeader)
		{
			if (entryHeader == nullptr)
				return 0;

			return (entryHeader->entrySize - sizeof(Logger::EntryHeader) - sizeof(Logger::EntryFooter) - entryHeader->metadataSize);
		}

		int Logger::getUsedSpace() const
		{
			return (((const char*)this->dataNext) - ((const char*)this->dataMain));
		}

		int Logger::getFreeSpace() const
		{
			return (((const char*)this->dataEnd) - ((const char*)this->dataNext));
		}

		void Logger::writeToFile(bool resetData)
		{
			if ((this->dataMain >= this->dataNext) || (this->filePath.IsEmpty() == true))
			{
				if (resetData == true)
				{
					this->numberCurrentEntries = 0;
					this->dataNext = this->dataMain;
				}

				return;
			}

	{
		HorseRadish::Streams::FileStream fileStream(this->filePath, false, true);

		{
			const Logger::EntryHeader *curEntry;
			char auxBuffer[256];

			HorseRadish::Streams::StreamWriter streamWriter(fileStream);

			curEntry = (const Logger::EntryHeader*)this->dataMain;

			for (; curEntry < this->dataNext; curEntry = ((const Logger::EntryHeader*)(((const char *)curEntry) + curEntry->entrySize)))
			{
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

				ctime_s(auxBuffer, sizeof(auxBuffer), &curEntry->entryTimestamp);
				streamWriter.Write(auxBuffer, strlen(auxBuffer) - 1);
				streamWriter.WriteString("}\t", false);

				streamWriter.Write(Logger::getEntryData(curEntry), Logger::getEntryDataSize(curEntry) - 1);

				streamWriter.Write(HorseRadish::Platform::NewLine, HorseRadish::Platform::NewLineSize);
			}
		}
	}

			if (resetData == true)
			{
				this->numberCurrentEntries = 0;
				this->dataNext = this->dataMain;
			}

			appendToFile = true;
		}

		bool Logger::addEntry(const EntryType &entryType, const char * const entryData, const void * const metadata, int metadataSize)
		{
			int entryDataSize, entryTotalSize;
			EntryHeader newEntryHeader;
			EntryFooter newEntryFooter;

			if ((this->dataNext == nullptr) || (entryData == nullptr) || (entryData[0] == '\0'))
				return false;

			entryDataSize = strlen(entryData) + 1;
			if (entryDataSize <= 0)
				return false;

			if ((metadataSize < 0) || (metadata == nullptr))
				metadataSize = 0;

			entryTotalSize = sizeof(EntryHeader) + metadataSize + entryDataSize + sizeof(EntryFooter);

			if (this->getFreeSpace() < entryTotalSize)
				this->writeToFile(true);

			if (this->getFreeSpace() < entryTotalSize)
				return false;

			time(&newEntryHeader.entryTimestamp);

			if (this->dataNext <= this->dataMain)
				newEntryHeader.prevEntrySize = 0;
			else
				newEntryHeader.prevEntrySize = Logger::getPreviousEntryHeader((const EntryHeader*)this->dataNext)->entrySize;

			newEntryHeader.entrySize = entryTotalSize;
			newEntryHeader.metadataSize = metadataSize;
			newEntryHeader.entryType = entryType;
			newEntryHeader.entryCRC8 = 0;
			newEntryHeader.entryCRC8 = HorseRadish::Hashing::CalculateCRC8(&newEntryHeader, sizeof(newEntryHeader));

			newEntryFooter.entrySize = newEntryHeader.entrySize;
			newEntryFooter.entryCRC8 = newEntryHeader.entryCRC8;

			memcpy(this->dataNext, &newEntryHeader, sizeof(EntryHeader));
			if (metadataSize > 0)
				memcpy(((char *)this->dataNext) + sizeof(EntryHeader), metadata, metadataSize);
			memcpy(((char *)this->dataNext) + sizeof(EntryHeader) + metadataSize, entryData, entryDataSize);
			memcpy(((char *)this->dataNext) + sizeof(EntryHeader) + metadataSize + entryDataSize, &newEntryFooter, sizeof(EntryFooter));

			this->dataNext = ((char *)this->dataNext) + newEntryHeader.entrySize;

			this->numberTotalEntries++;
			this->numberCurrentEntries++;

			return true;
		}

		Logger::Logger(int logCapacityKB)
			: dataMain(nullptr), dataEnd(nullptr), dataNext(nullptr), appendToFile(false), numberTotalEntries(0), numberCurrentEntries(0)
		{
			this->filePath.Clear();

			int allocSize = ((logCapacityKB <= 0) ? Logger::DefaultCapacityKB : logCapacityKB) * 1024;

			this->dataMain = malloc(allocSize);
			if (this->dataMain == nullptr)
				return;

			this->dataNext = this->dataMain;
			this->dataEnd = ((char * const)this->dataMain) + allocSize;
		}

		Logger::Logger(int logCapacityKB, const HorseRadish::IO::Path &filePath, bool appendToFile)
			: dataMain(nullptr), dataEnd(nullptr), dataNext(nullptr), appendToFile(appendToFile), numberTotalEntries(0), numberCurrentEntries(0)
		{
			this->filePath.Set(filePath);

			int allocSize = ((logCapacityKB <= 0) ? Logger::DefaultCapacityKB : logCapacityKB) * 1024;

			this->dataMain = malloc(allocSize);
			if (this->dataMain == nullptr)
				return;

			this->dataNext = this->dataMain;
			this->dataEnd = ((char * const)this->dataMain) + allocSize;
		}

		Logger::~Logger()
		{
			this->writeToFile(true);

			if (this->dataMain != nullptr)
				free(this->dataMain);

			this->dataMain = this->dataEnd = this->dataNext = nullptr;
			this->numberTotalEntries = this->numberCurrentEntries = 0;
			this->filePath.Clear();
			this->appendToFile = false;
		}

		void Logger::Log(const EntryType &entryType, const char * const entryData)
		{
			if ((this->dataMain == nullptr) || (entryData == nullptr))
				return;

			{
				std::lock_guard<std::mutex> lock(this->syncLock);

				this->addEntry(entryType, entryData, nullptr, 0);
			}
		}

		void Logger::Log(const EntryType &entryType, const void * const metadata, const int metadataSize, const char * const entryData)
		{
			if ((this->dataMain == nullptr) || (entryData == nullptr))
				return;

			{
				std::lock_guard<std::mutex> lock(this->syncLock);

				this->addEntry(entryType, entryData, metadata, metadataSize);
			}
		}

		void Logger::Log(const char * const entryData)
		{
			if ((this->dataMain == nullptr) || (entryData == nullptr))
				return;

			{
				std::lock_guard<std::mutex> lock(this->syncLock);

				this->addEntry(EntryType::Normal, entryData, nullptr, 0);
			}
		}

		void Logger::Log(const void * const metadata, const int metadataSize, const char * const entryData)
		{
			if ((this->dataMain == nullptr) || (entryData == nullptr))
				return;

			{
				std::lock_guard<std::mutex> lock(this->syncLock);

				this->addEntry(EntryType::Normal, entryData, metadata, metadataSize);
			}
		}

		void Logger::LogInfo(const char * const entryData)
		{
			if ((this->dataMain == nullptr) || (entryData == nullptr))
				return;

			{
				std::lock_guard<std::mutex> lock(this->syncLock);

				this->addEntry(EntryType::Info, entryData, nullptr, 0);
			}
		}

		void Logger::LogInfo(const void * const metadata, const int metadataSize, const char * const entryData)
		{
			if ((this->dataMain == nullptr) || (entryData == nullptr))
				return;

			{
				std::lock_guard<std::mutex> lock(this->syncLock);

				this->addEntry(EntryType::Info, entryData, metadata, metadataSize);
			}
		}

		void Logger::LogWarning(const char * const entryData)
		{
			if ((this->dataMain == nullptr) || (entryData == nullptr))
				return;

			{
				std::lock_guard<std::mutex> lock(this->syncLock);

				this->addEntry(EntryType::Warning, entryData, nullptr, 0);
			}
		}

		void Logger::LogWarning(const void * const metadata, const int metadataSize, const char * const entryData)
		{
			if ((this->dataMain == nullptr) || (entryData == nullptr))
				return;

			{
				std::lock_guard<std::mutex> lock(this->syncLock);

				this->addEntry(EntryType::Warning, entryData, metadata, metadataSize);
			}
		}

		void Logger::LogError(const char * const entryData)
		{
			if ((this->dataMain == nullptr) || (entryData == nullptr))
				return;

			{
				std::lock_guard<std::mutex> lock(this->syncLock);

				this->addEntry(EntryType::Error, entryData, nullptr, 0);
			}
		}

		void Logger::LogError(const void * const metadata, const int metadataSize, const char * const entryData)
		{
			if ((this->dataMain == nullptr) || (entryData == nullptr))
				return;

			{
				std::lock_guard<std::mutex> lock(this->syncLock);

				this->addEntry(EntryType::Error, entryData, metadata, metadataSize);
			}
		}

		void Logger::Reset()
		{
			if (this->dataMain == nullptr)
				return;

			{
				std::lock_guard<std::mutex> lock(this->syncLock);

				this->dataNext = this->dataMain;
				this->numberCurrentEntries = 0;
			}
		}

		void Logger::WriteToFile(bool resetData)
		{
			if (this->dataMain == nullptr)
				return;

			{
				std::lock_guard<std::mutex> lock(this->syncLock);

				this->writeToFile(resetData);
			}
		}

		void Logger::Iterate(bool fromBottom, std::function<bool(const char * const data, const int dataSize)> funcCallback, int numberOffset) const
		{
			if ((this->dataMain == nullptr) || (funcCallback == nullptr))
				return;

			{
				std::lock_guard<std::mutex> lock(this->syncLock);

				if (this->dataNext <= this->dataMain)
					return;

				if (fromBottom == true)
				{
					auto entryWalker = reinterpret_cast<const EntryHeader*>(this->dataNext);
					entryWalker = Logger::getPreviousEntryHeader(entryWalker);

					for (; entryWalker >= this->dataMain; entryWalker = Logger::getPreviousEntryHeader(entryWalker))
					{
						if (numberOffset > 0)
						{
							numberOffset--;
							continue;
						}

						if (funcCallback((const char*)Logger::getEntryData(entryWalker), Logger::getEntryDataSize(entryWalker)) == false)
							break;
					}
				}
				else
				{
					auto entryWalker = reinterpret_cast<const EntryHeader*>(this->dataMain);

					for (; entryWalker < this->dataNext; entryWalker = Logger::getNextEntryHeader(entryWalker))
					{
						if (numberOffset > 0)
						{
							numberOffset--;
							continue;
						}

						if (funcCallback((const char*)Logger::getEntryData(entryWalker), Logger::getEntryDataSize(entryWalker)) == false)
							break;
					}
				}
			}
		}

		void Logger::Iterate(bool fromBottom, std::function<bool(const void * const metadata, const int metadataSize, const char * const data, const int dataSize)> funcCallback, int numberOffset) const
		{
			if ((this->dataMain == nullptr) || (funcCallback == nullptr))
				return;

			{
				std::lock_guard<std::mutex> lock(this->syncLock);

				if (this->dataNext <= this->dataMain)
					return;

				if (fromBottom == true)
				{
					auto entryWalker = reinterpret_cast<const EntryHeader*>(this->dataNext);
					entryWalker = Logger::getPreviousEntryHeader(entryWalker);

					for (; entryWalker >= this->dataMain; entryWalker = Logger::getPreviousEntryHeader(entryWalker))
					{
						if (numberOffset > 0)
						{
							numberOffset--;
							continue;
						}

						if (funcCallback(Logger::getEntryMetada(entryWalker), Logger::getEntryMetadataSize(entryWalker), (const char*)Logger::getEntryData(entryWalker), Logger::getEntryDataSize(entryWalker)) == false)
							break;
					}
				}
				else
				{
					auto entryWalker = reinterpret_cast<const EntryHeader*>(this->dataMain);

					for (; entryWalker < this->dataNext; entryWalker = Logger::getNextEntryHeader(entryWalker))
					{
						if (numberOffset > 0)
						{
							numberOffset--;
							continue;
						}

						if (funcCallback(Logger::getEntryMetada(entryWalker), Logger::getEntryMetadataSize(entryWalker), (const char*)Logger::getEntryData(entryWalker), Logger::getEntryDataSize(entryWalker)) == false)
							break;
					}
				}
			}
		}

	} //Logging
} //HorseRadish