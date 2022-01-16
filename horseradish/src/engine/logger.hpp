#pragma once

#include "common/stream.hpp"
#include "libs/fmt/format.h"

#include <vector>
#include <deque>
#include <mutex>
#include <chrono>
#include <thread>
#include <atomic>
#include <condition_variable>

namespace hr { namespace engine
{
	class Logger
	{
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

			template<typename... TValues>
			void log(const EntryType entryType, const char * const entryData, TValues&&... params)
			{
				mLogger.log(entryType, mTargetModule, entryData, std::forward<TValues>(params)...);
			}

			template<typename... TValues>
			void info(const char * const entryData, TValues&&... params)
			{
				mLogger.logInfo(mTargetModule, entryData, std::forward<TValues>(params)...);
			}

			template<typename... TValues>
			void warn(const char * const entryData, TValues&&... params)
			{
				mLogger.logWarning(mTargetModule, entryData, std::forward<TValues>(params)...);
			}

			template<typename... TValues>
			void error(const char * const entryData, TValues&&... params)
			{
				mLogger.logError(mTargetModule, entryData, std::forward<TValues>(params)...);
			}

			template<typename... TValues>
			void log(const EntryType entryType, const std::string &entryData, TValues&&... params)
			{
				log(entryType, entryData.c_str(), std::forward<TValues>(params)...);
			}

			template<typename... TValues>
			void info(const std::string &entryData, TValues&&... params)
			{
				info(entryData.c_str(), std::forward<TValues>(params)...);
			}

			template<typename... TValues>
			void warn(const std::string &entryData, TValues&&... params)
			{
				warn(entryData.c_str(), std::forward<TValues>(params)...);
			}

			template<typename... TValues>
			void error(const std::string &entryData, TValues&&... params)
			{
				error(entryData.c_str(), std::forward<TValues>(params)...);
			}

		private:
			Logger &mLogger;
			Logger::ModuleType mTargetModule;
		};

	private:
		struct EntryData
		{
			std::string msg;
			bool isMsgFormated{ false };
			EntryType entryType{ EntryType::Error };
			ModuleType moduleType{ ModuleType::Misc };
			std::chrono::time_point<std::chrono::system_clock> timestamp;

			EntryData() = default;

			EntryData(EntryType entryType, ModuleType moduleType) noexcept
				: entryType{ entryType }, moduleType{ moduleType }
			{ }
		};

		static bool checkEntryData(const char * const entryData, bool &hasFormattedText, size_t &dataSize);

		mutable std::mutex mASyncLock;
		mutable std::mutex mBufferLock;

		std::deque<EntryData> mBuffer;
		std::vector<EntryData> mAsyncBuffer;
		size_t mMaxBufferSize = 0;
		size_t mMaxAsyncBufferSize = 0;

		std::thread *mThreadFlush = nullptr;
		std::atomic<bool> mThreadFlushExit = {false};
		std::condition_variable mThreadFlushCondition;
		std::shared_ptr<hr::streams::FileStream> mOutFileStream;

		void processAsyncBuffer();
		void writeToFile(const EntryData& entry);
		void threadFlushFunc();
		bool addEntry(const EntryType entryType, const ModuleType moduleType, const char * const entryData);

	public:
		Logger(size_t asyncMaxEntries);
		Logger(size_t asyncMaxEntries, size_t maxBufferedEntries);
		Logger(size_t asyncMaxEntries, size_t maxBufferedEntries, const hr::io::Path &filePath);
		~Logger();

		Logger(const Logger&) = delete;
		Logger& operator=(const Logger&) = delete;

		void log(const EntryType entryType, const ModuleType moduleType, const char * const entryData)
		{
			addEntry(entryType, moduleType, entryData);
		}

		void log(const EntryType entryType, const ModuleType moduleType, const std::string& entryData)
		{
			addEntry(entryType, moduleType, entryData.c_str());
		}
		
		template<typename... TValues>
		void log(const EntryType entryType, const ModuleType moduleType, const char * const entryData, TValues&&... params)
		{
			log(entryType, moduleType, fmt::format(fmt::runtime(entryData), std::forward<TValues>(params)...));
		}

		template<typename... TValues>
		void log(const EntryType entryType, const ModuleType moduleType, const std::string &entryData, TValues&&... params)
		{
			log(entryType, moduleType, fmt::format(fmt::runtime(entryData), std::forward<TValues>(params)...));
		}

		template<typename... TValues>
		void logInfo(const ModuleType moduleType, const char * const entryData, TValues&&... params)
		{
			log(EntryType::Info, moduleType, entryData, std::forward<TValues>(params)...);
		}

		template<typename... TValues>
		void logInfo(const ModuleType moduleType, const std::string &entryData, TValues&&... params)
		{
			log(EntryType::Info, moduleType, entryData.c_str(), std::forward<TValues>(params)...);
		}

		template<typename... TValues>
		void logWarning(const ModuleType moduleType, const char * const entryData, TValues&&... params)
		{
			log(EntryType::Warning, moduleType, entryData, std::forward<TValues>(params)...);
		}

		template<typename... TValues>
		void logWarning(const ModuleType moduleType, const std::string &entryData, TValues&&... params)
		{
			log(EntryType::Warning, moduleType, entryData.c_str(), std::forward<TValues>(params)...);
		}

		template<typename... TValues>
		void logError(const ModuleType moduleType, const char * const entryData, TValues&&... params)
		{
			log(EntryType::Error, moduleType, entryData, std::forward<TValues>(params)...);
		}

		template<typename... TValues>
		void logError(const ModuleType moduleType, const std::string &entryData, TValues&&... params)
		{
			log(EntryType::Error, moduleType, entryData.c_str(), std::forward<TValues>(params)...);
		}

		void iterateBuffer(std::function<bool(const EntryType, const ModuleType, const bool, const std::string&)> logEntryCb, size_t offset) const;
	};
} }
