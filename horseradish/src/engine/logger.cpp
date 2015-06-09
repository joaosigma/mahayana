#include "logger.hpp"

#include "common\Stream.hpp"

#include <ctime>

namespace HorseRadish
{
	namespace Engine
	{
		const int Logger::DefaultCapacityKB = 1024;

		bool Logger::checkEntryData(const char * const entryData, bool &hasFormattedText, unsigned int &dataSize)
		{
			dataSize = 0;
			hasFormattedText = false;

			bool insideSection = false;
			const char *walker = entryData;
			for (; *walker != '\0'; walker++)
			{
				if (insideSection && ((walker[0] == '$') || (walker[0] == '{')))
					return false; //invalid chars inside section

				if (insideSection && (walker[0] == '}'))
				{
					insideSection = false;
					continue;
				}

				if ((walker[0] == '$') && (walker[1] == '{'))
				{
					if ((walker != entryData) && (walker[-1] == '$'))
						continue;

					walker++;
					insideSection = true;
					hasFormattedText |= true;
				}
			}

			if (insideSection)
				return false;

			dataSize = (walker - entryData) + 1; //empty strings are logged
			return true;
		}

		void Logger::writeToFile()
		{
			if (mWalkerFlush == mWalkerWrite)
				return;

			HorseRadish::Streams::StreamWriter streamWriter(*mOutFileStream);

			while (mWalkerFlush != mWalkerWrite)
			{
				switch (static_cast<Logger::ModuleType>(mWalkerFlush->moduleType))
				{
				case Logger::ModuleType::SysRuntime:
					streamWriter.WriteString("sysRuntime\t", false);
					break;
				case Logger::ModuleType::FileSystem:
					streamWriter.WriteString("fileSystem\t", false);
					break;
				case Logger::ModuleType::Graphics:
					streamWriter.WriteString("graphics\t", false);
					break;
				case Logger::ModuleType::Audio:
					streamWriter.WriteString("audio\t", false);
					break;
				case Logger::ModuleType::Network:
					streamWriter.WriteString("network\t", false);
					break;
				case Logger::ModuleType::PlayRuntime:
					streamWriter.WriteString("playRuntime\t", false);
					break;
				case Logger::ModuleType::Misc:
				default:
					streamWriter.WriteString("misc\t", false);
					break;
				}

				switch (static_cast<Logger::EntryType>(mWalkerFlush->entryType))
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
					streamWriter.WriteString("????\t{", false);
					break;
				}

				{
					char bufferTmp[256];

					std::time_t tmT = std::chrono::system_clock::to_time_t(mWalkerFlush->timestamp);
					std::tm* tmUTC = std::gmtime(&tmT);

					strftime(bufferTmp, sizeof(bufferTmp), "%Y-%m-%d %H:%M:%S", tmUTC);

					streamWriter.Write(bufferTmp, strlen(bufferTmp) - 1);
					streamWriter.WriteString("}\t", false);
				}

				if (!mWalkerFlush->hasFormattedText)
				{
					streamWriter.Write(reinterpret_cast<unsigned char*>(mWalkerFlush)+sizeof(EntryHeader), mWalkerFlush->entryTotalSize - sizeof(EntryHeader) - 1);
				}
				else
				{
					auto walker = reinterpret_cast<const char*>(mWalkerFlush)+sizeof(EntryHeader);
					while ((walker[0] == '$') && (walker[1] == '{'))
					{
						for (; (*walker != '\0') && (*walker != '}'); walker++);
						if (*walker == '\0')
							break;
						walker++;
					}

					auto walkerNext = walker;
					for (; *walkerNext != '\0'; walkerNext++)
					{
						if ((walkerNext[0] == '$') && (walkerNext[1] == '{') && (walker[-1] != '$'))
						{
							if ((walkerNext - walker) > 0)
								streamWriter.Write(walker, walkerNext - walker);

							for (; (*walkerNext != '\0') && (*walkerNext != '}'); walkerNext++);
							if (*walkerNext == '}')
								walkerNext++;

							walker = walkerNext;
							if (*walkerNext == '\0')
								break;
						}
					}

					if ((walkerNext - walker) > 0)
						streamWriter.Write(walker, walkerNext - walker);
				}

				streamWriter.Write(HorseRadish::Platform::NewLine, HorseRadish::Platform::NewLineSize);

				if (mWalkerFlush->proxEntry < mWalkerFlush)
					this->mWalkerFlushBehind = true;
				mWalkerFlush = mWalkerFlush->proxEntry;
			}
		}

		void Logger::threadFlushFunc()
		{
			while (true)
			{
				if (mThreadFlushExit)
					break;

				std::unique_lock<std::mutex> lock(mSyncLock);

				auto waitReson = mThreadFlushCondition.wait_for(lock, std::chrono::milliseconds(1500));
				if (mThreadFlushExit)
					break;

				if (waitReson == std::cv_status::timeout)
					this->writeToFile();
			}
		}

		bool Logger::addEntry(const EntryType &entryType, const ModuleType moduleType, const char * const entryData)
		{
			bool hasFormattedText;
			unsigned int entryDataSize;
			if (!Logger::checkEntryData(entryData, hasFormattedText, entryDataSize))
				return false;			

			int entryTotalSize = sizeof(EntryHeader) + entryDataSize;
			int entryTotalSizeNext = entryTotalSize + sizeof(EntryHeader); //the prevEntry of the next entry is always written, so we need 2 headers

			if ((reinterpret_cast<unsigned char*>(mWalkerWrite)+entryTotalSizeNext) >= reinterpret_cast<unsigned char*>(mDataEnd))
			{
				if ((mOutFileStream != nullptr) && (mWalkerFlush == mDataMain))
				{
					writeToFile();
					if (mWalkerFlush == mDataMain)
						return false;
				}

				mWalkerWrite = reinterpret_cast<EntryHeader*>(mDataMain);
				mWalkerFlushBehind = false;
			}

			if (!mWalkerFlushBehind && (mOutFileStream != nullptr) && ((reinterpret_cast<unsigned char*>(mWalkerWrite)+entryTotalSizeNext) >= reinterpret_cast<unsigned char*>(mWalkerFlush)))
			{
				writeToFile();
				if ((reinterpret_cast<unsigned char*>(mWalkerWrite)+entryTotalSizeNext) >= reinterpret_cast<unsigned char*>(mWalkerFlush))
					return false;
			}

			mWalkerWrite->hasFormattedText = hasFormattedText;
			mWalkerWrite->entryTotalSize = entryTotalSize;
			mWalkerWrite->entryType = entryType;
			mWalkerWrite->moduleType = moduleType;
			mWalkerWrite->timestamp = std::chrono::system_clock::now();

			memcpy(reinterpret_cast<unsigned char*>(mWalkerWrite)+sizeof(EntryHeader), entryData, entryDataSize);

			auto curEntry = mWalkerWrite;
			mWalkerWrite = reinterpret_cast<EntryHeader*>(reinterpret_cast<unsigned char*>(mWalkerWrite)+entryTotalSize);
			assert(mWalkerWrite < mDataEnd);

			mWalkerWrite->entryTotalSize = 0;
			mWalkerWrite->proxEntry = nullptr;
			mWalkerWrite->prevEntry = curEntry;
			curEntry->proxEntry = mWalkerWrite;

			mTotalEntries++;

			return true;
		}

		Logger::Logger(unsigned int logCapacityKB)
			: mTotalEntries(0), mDataMain(nullptr), mDataEnd(nullptr)
			, mThreadFlush(nullptr), mThreadFlushExit(false), mWalkerFlushBehind(true), mWalkerWrite(nullptr), mWalkerFlush(nullptr), mOutFileStream(nullptr)
		{
			if (logCapacityKB <= 0)
				return;

			int allocSize = ((logCapacityKB <= 0) ? Logger::DefaultCapacityKB : logCapacityKB) * 1024;

			mDataMain = malloc(allocSize);
			if (mDataMain == nullptr)
				return;

			this->mDataEnd = reinterpret_cast<unsigned char*>(mDataMain) + allocSize;
			mWalkerWrite = reinterpret_cast<EntryHeader*>(mDataMain);

			mWalkerWrite->entryTotalSize = 0;
			mWalkerWrite->hasFormattedText = false;
			mWalkerWrite->prevEntry = mWalkerWrite->proxEntry = nullptr;
		}

		Logger::Logger(unsigned int logCapacityKB, const HorseRadish::IO::Path &filePath, const bool threadedFlush)
			: Logger(logCapacityKB)
		{
			if (logCapacityKB <= 0)
				return;

			mOutFileStream = new HorseRadish::Streams::FileStream(filePath, false, true);
			mWalkerFlush = mWalkerWrite;

			if (threadedFlush)
				mThreadFlush = new std::thread(&Logger::threadFlushFunc, this);
		}

		Logger::~Logger()
		{
			if (mThreadFlush)
			{
				mThreadFlushExit = true;
				mThreadFlushCondition.notify_one();
				mThreadFlush->join();

				delete mThreadFlush;
				mThreadFlush = nullptr;
			}

			if (mOutFileStream != nullptr)
			{
				this->writeToFile();
				delete mOutFileStream;
			}
			mOutFileStream = nullptr;

			if (mDataMain != nullptr)
				free(mDataMain);

			mDataMain = mDataEnd = nullptr;
			mWalkerWrite = mWalkerFlush = nullptr;
		}

		void Logger::AddLog(const EntryType entryType, const ModuleType moduleType, const char * const entryData)
		{
			if ((mDataMain == nullptr) || (entryData == nullptr) || (entryData[0] == '\0'))
				return;

			std::lock_guard<std::mutex> lock(mSyncLock);

			this->addEntry(entryType, moduleType, entryData);
		}

		void Logger::AddLog(const EntryType entryType, const ModuleType moduleType, const std::string &entryData)
		{
			this->AddLog(entryType, moduleType, entryData.c_str());
		}

		void Logger::AddInfo(const ModuleType moduleType, const char * const entryData)
		{
			if ((mDataMain == nullptr) || (entryData == nullptr) || (entryData[0] == '\0'))
				return;

			std::lock_guard<std::mutex> lock(mSyncLock);

			this->addEntry(EntryType::Info, moduleType, entryData);
		}

		void Logger::AddInfo(const ModuleType moduleType, const std::string &entryData)
		{
			this->AddInfo(moduleType, entryData.c_str());
		}

		void Logger::AddWarning(const ModuleType moduleType, const char * const entryData)
		{
			if ((mDataMain == nullptr) || (entryData == nullptr) || (entryData[0] == '\0'))
				return;

			std::lock_guard<std::mutex> lock(mSyncLock);

			this->addEntry(EntryType::Warning, moduleType, entryData);
		}

		void Logger::AddWarning(const ModuleType moduleType, const std::string &entryData)
		{
			this->AddWarning(moduleType, entryData.c_str());
		}

		void Logger::AddError(const ModuleType moduleType, const char * const entryData)
		{
			if ((mDataMain == nullptr) || (entryData == nullptr) || (entryData[0] == '\0'))
				return;

			std::lock_guard<std::mutex> lock(mSyncLock);

			this->addEntry(EntryType::Error, moduleType, entryData);
		}

		void Logger::AddError(const ModuleType moduleType, const std::string &entryData)
		{
			this->AddError(moduleType, entryData.c_str());
		}

		void Logger::IterateLast(std::function<bool(const EntryType, const ModuleType, const bool, const char * const)> logEntryCb, unsigned int offset)
		{
			if ((mDataMain == nullptr) || !logEntryCb)
				return;

			std::lock_guard<std::mutex> lock(mSyncLock);

			EntryHeader *walkerBegin = mWalkerWrite;
			EntryHeader *walker = mWalkerWrite->prevEntry;

			for (; walker != nullptr; walker = walker->prevEntry)
			{
				if ((walker > walkerBegin) && (walker->prevEntry >= (walkerBegin + 1)))
					break;

				if (offset > 0)
				{
					offset--;
					continue;
				}

				if (!logEntryCb(walker->entryType, walker->moduleType, walker->hasFormattedText, reinterpret_cast<const char*>(walker)+sizeof(EntryHeader)))
					break;
			}
		}

		unsigned int Logger::GetTotalEntries() const
		{
			return mTotalEntries;
		}

		void Logger::FlushToFile()
		{
			if ((mDataMain == nullptr) || (mOutFileStream == nullptr))
				return;

			std::lock_guard<std::mutex> lock(mSyncLock);

			this->writeToFile();
		}

	} //Engine
} //HorseRadish
