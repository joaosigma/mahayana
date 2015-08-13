#pragma once

#include "common\Stream.hpp"

#include <libs\cppformat\format.h>

#include <vector>
#include <deque>
#include <mutex>
#include <chrono>
#include <thread>
#include <atomic>
#include <condition_variable>

namespace HorseRadish { namespace Engine {
	
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
		bool isMsgFormated;
		EntryType entryType;
		ModuleType moduleType;
		std::chrono::time_point<std::chrono::system_clock> timestamp;

		EntryData()
			: isMsgFormated(false), entryType(EntryType::Error), moduleType(ModuleType::Misc)
		{ }

		EntryData(EntryType entryType, ModuleType moduleType)
			: isMsgFormated(false), entryType(entryType), moduleType(moduleType)
		{ }
	};

	static bool checkEntryData(const char * const entryData, bool &hasFormattedText, unsigned int &dataSize);

	mutable std::mutex mASyncLock;
	mutable std::mutex mBufferLock;

	std::deque<EntryData> mBuffer;
	std::vector<EntryData> mAsyncBuffer;
	unsigned int mMaxBufferSize;
	unsigned int mMaxAsyncBufferSize;

	std::thread *mThreadFlush;
	std::atomic<bool> mThreadFlushExit;
	std::condition_variable mThreadFlushCondition;
	std::shared_ptr<HorseRadish::Streams::FileStream> mOutFileStream;

	void processAsyncBuffer();
	void writeToFile(const EntryData& entry);
	void threadFlushFunc();
	bool addEntry(const EntryType entryType, const ModuleType moduleType, const char * const entryData);

public:
	Logger(unsigned int asyncMaxEntries);
	Logger(unsigned int asyncMaxEntries, unsigned int maxBufferedEntries);
	Logger(unsigned int asyncMaxEntries, unsigned int maxBufferedEntries, const HorseRadish::IO::Path &filePath);
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
		log(entryType, moduleType, fmt::format(entryData, std::forward<TValues>(params)...));
	}

	template<typename... TValues>
	void log(const EntryType entryType, const ModuleType moduleType, const std::string &entryData, TValues&&... params)
	{
		log(entryType, moduleType, fmt::format(entryData, std::forward<TValues>(params)...));
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

	void iterateBuffer(std::function<bool(const EntryType, const ModuleType, const bool, const std::string&)> logEntryCb, unsigned int offset) const;
};

} }
