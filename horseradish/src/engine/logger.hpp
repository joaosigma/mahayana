#pragma once

#include "common\Types.hpp"
#include "common\Stream.hpp"

#include <mutex>
#include <chrono>
#include <thread>
#include <atomic>
#include <condition_variable>

namespace HorseRadish
{
	namespace Engine
	{
		class Logger{

			static const int DefaultCapacityKB;

		public:
			enum class EntryType { Info, Warning, Error };

			enum class ModuleType { SysRuntime, FileSystem, Graphics, Audio, Network, PlayRuntime, Misc };

			class Context
			{
			public:
				Context(Logger &logger, const Logger::ModuleType module)
					: mLogger(logger), mTargetModule(module)
				{ }

				Context(const Context&) = delete;
				Context& operator=(const Context&) = delete;

				void log(const EntryType entryType, const char * const entryData)
				{
					mLogger.AddLog(entryType, mTargetModule, entryData);
				}

				void info(const char * const entryData)
				{
					mLogger.AddInfo(mTargetModule, entryData);
				}

				void warn(const char * const entryData)
				{
					mLogger.AddWarning(mTargetModule, entryData);
				}

				void error(const char * const entryData)
				{
					mLogger.AddError(mTargetModule, entryData);
				}

				void log(const EntryType entryType, const std::string &entryData)
				{
					log(entryType, entryData.c_str());
				}

				void info(const std::string &entryData)
				{
					info(entryData.c_str());
				}

				void warn(const std::string &entryData)
				{
					warn(entryData.c_str());
				}

				void error(const std::string &entryData)
				{
					error(entryData.c_str());
				}

			private:
				Logger &mLogger;
				Logger::ModuleType mTargetModule;
			};

		private:
			struct EntryHeader{
				bool hasFormattedText;
				hUInt32 entryTotalSize;
				EntryHeader *proxEntry, *prevEntry;

				EntryType entryType;
				ModuleType moduleType;
				std::chrono::time_point<std::chrono::system_clock> timestamp;
			};

			static bool checkEntryData(const char * const entryData, bool &hasFormattedText, unsigned int &dataSize);

			mutable std::mutex mSyncLock;

			unsigned int mTotalEntries;
			void *mDataMain, *mDataEnd;
			bool mWalkerFlushBehind;
			std::thread *mThreadFlush;
			std::atomic<bool> mThreadFlushExit;
			std::condition_variable mThreadFlushCondition;
			EntryHeader *mWalkerWrite, *mWalkerFlush;
			HorseRadish::Streams::FileStream *mOutFileStream;

			void writeToFile();
			void threadFlushFunc();
			bool addEntry(const EntryType &entryType, const ModuleType moduleType, const char * const entryData);

		public:
			Logger(unsigned int logCapacityKB);
			Logger(unsigned int logCapacityKB, const HorseRadish::IO::Path &filePath, const bool threadedFlush);
			~Logger();

			Logger(const Logger&) = delete;
			Logger& operator=(const Logger&) = delete;

			void AddLog(const EntryType entryType, const ModuleType moduleType, const char * const entryData);
			void AddLog(const EntryType entryType, const ModuleType moduleType, const std::string &entryData);
			void AddInfo(const ModuleType moduleType, const char * const entryData);
			void AddInfo(const ModuleType moduleType, const std::string &entryData);
			void AddWarning(const ModuleType moduleType, const char * const entryData);
			void AddWarning(const ModuleType moduleType, const std::string &entryData);
			void AddError(const ModuleType moduleType, const char * const entryData);
			void AddError(const ModuleType moduleType, const std::string &entryData);

			void IterateLast(std::function<bool(const EntryType, const ModuleType, const bool, const char * const)> logEntryCb, unsigned int offset);

			unsigned int GetTotalEntries() const;

			void FlushToFile();
		};

	} //Engine
} //HorseRadish
