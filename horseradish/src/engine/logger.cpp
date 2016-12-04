#include "logger.hpp"

#include "common/stream.hpp"

#include <ctime>

namespace hr { namespace engine
{
	bool Logger::checkEntryData(const char * const entryData, bool &hasFormattedText, size_t &dataSize)
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

	void Logger::processAsyncBuffer()
	{
		if (mAsyncBuffer.empty())
			return;

		for (const auto& entry : mAsyncBuffer)
			writeToFile(entry);

		mAsyncBuffer.clear();
	}

	void Logger::writeToFile(const EntryData& entry)
	{
		if (!mOutFileStream)
			return;

		hr::streams::StreamWriter streamWriter(*mOutFileStream);

		switch (entry.moduleType)
		{
		case Logger::ModuleType::SysRuntime:
			streamWriter.writeString("sysRuntime\t");
			break;
		case Logger::ModuleType::FileSystem:
			streamWriter.writeString("fileSystem\t");
			break;
		case Logger::ModuleType::Graphics:
			streamWriter.writeString("graphics\t");
			break;
		case Logger::ModuleType::Audio:
			streamWriter.writeString("audio\t");
			break;
		case Logger::ModuleType::Network:
			streamWriter.writeString("network\t");
			break;
		case Logger::ModuleType::PlayRuntime:
			streamWriter.writeString("playRuntime\t");
			break;
		case Logger::ModuleType::Misc:
		default:
			streamWriter.writeString("misc\t");
			break;
		}

		switch (entry.entryType)
		{
		case Logger::EntryType::Error:
			streamWriter.writeString("error\t{");
			break;
		case Logger::EntryType::Info:
			streamWriter.writeString("info\t{");
			break;
		case Logger::EntryType::Warning:
			streamWriter.writeString("warning\t{");
			break;
		default:
			streamWriter.writeString("????\t{");
			break;
		}

		{
			char bufferTmp[256];

			std::time_t tmT = std::chrono::system_clock::to_time_t(entry.timestamp);
			std::tm* tmUTC = std::gmtime(&tmT);

			strftime(bufferTmp, sizeof(bufferTmp), "%Y-%m-%d %H:%M:%S", tmUTC);

			streamWriter.write(bufferTmp, strlen(bufferTmp) - 1);
			streamWriter.writeString("}\t");
		}

		if (!entry.isMsgFormated)
		{
			streamWriter.writeString(entry.msg.c_str());
		}
		else
		{
			auto walker = reinterpret_cast<const char*>(entry.msg.c_str());
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
						streamWriter.write(walker, walkerNext - walker);

					for (; (*walkerNext != '\0') && (*walkerNext != '}'); walkerNext++);
					if (*walkerNext == '}')
						walkerNext++;

					walker = walkerNext;
					if (*walkerNext == '\0')
						break;
				}
			}

			if ((walkerNext - walker) > 0)
				streamWriter.write(walker, walkerNext - walker);
		}

		streamWriter.write(hr::platform::Platform::NewLine, hr::platform::Platform::NewLineSize);
	}

	void Logger::threadFlushFunc()
	{
		while (!mThreadFlushExit)
		{
			std::unique_lock<std::mutex> lock(mASyncLock);

			auto waitReson = mThreadFlushCondition.wait_for(lock, std::chrono::milliseconds(1500));
			if (waitReson == std::cv_status::timeout)
			{
				processAsyncBuffer();

				if (mOutFileStream)
					mOutFileStream->flush();
			}
		}
	}

	bool Logger::addEntry(const EntryType entryType, const ModuleType moduleType, const char * const entryData)
	{
		if (!entryData)
			return true;

		bool hasFormattedText;
		size_t entryDataSize;
		if (!Logger::checkEntryData(entryData, hasFormattedText, entryDataSize))
			return false;

		EntryData entry(entryType, moduleType);
		entry.msg = std::string(entryData, entryDataSize);
		entry.isMsgFormated = hasFormattedText;
		entry.timestamp = std::chrono::system_clock::now();

		std::lock_guard<std::mutex> lock(mASyncLock);

		if (mMaxBufferSize > 0)
		{
			std::lock_guard<std::mutex> lock(mBufferLock);

			if (mBuffer.size() >= mMaxBufferSize)
				mBuffer.pop_back();

			mBuffer.push_front(entry);
			assert(mBuffer.size() <= mMaxBufferSize);
		}

		if (mMaxAsyncBufferSize == 0)
		{
			writeToFile(entry);
		}
		else
		{
			if (mAsyncBuffer.size() >= mMaxAsyncBufferSize)
				processAsyncBuffer();

			mAsyncBuffer.push_back(entry);
		}

		return true;
	}

	Logger::Logger(size_t asyncMaxEntries)
		: mMaxAsyncBufferSize(asyncMaxEntries)
	{
		if (mMaxAsyncBufferSize > 0)
			mThreadFlush = new std::thread(&Logger::threadFlushFunc, this);
	}

	Logger::Logger(size_t asyncMaxEntries, size_t maxBufferedEntries)
		: Logger(asyncMaxEntries)
	{
		mMaxBufferSize = maxBufferedEntries;
	}

	Logger::Logger(size_t asyncMaxEntries, size_t maxBufferedEntries, const hr::io::Path &filePath)
		: Logger(asyncMaxEntries, maxBufferedEntries)
	{
		mOutFileStream = std::shared_ptr<hr::streams::FileStream>(new hr::streams::FileStream(filePath.str(), false, true));
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

		if (mOutFileStream)
		{
			for (const auto& entry : mAsyncBuffer)
				writeToFile(entry);

			mOutFileStream.reset();
		}
	}

	void Logger::iterateBuffer(std::function<bool(const EntryType, const ModuleType, const bool, const std::string&)> logEntryCb, size_t offset) const
	{
		if ((mMaxBufferSize == 0) || !logEntryCb)
			return;

		std::lock_guard<std::mutex> lock(mBufferLock);

		for (size_t i = offset; i < mBuffer.size(); i++)
		{
			const EntryData& entry = mBuffer[i];
			if (!logEntryCb(entry.entryType, entry.moduleType, entry.isMsgFormated, entry.msg))
				break;
		}
	}
} }
