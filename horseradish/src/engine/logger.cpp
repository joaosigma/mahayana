#include "logger.hpp"

#include "common\Stream.hpp"

#include <ctime>

namespace HorseRadish { namespace Engine {

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

void Logger::processAsyncBuffer()
{
	if (mAsyncBuffer.empty())
		return;

	if (mMaxBufferSize > 0)
	{
		std::unique_lock<std::mutex> lock(mBufferLock);

		size_t bufferIndex = 0;

		if (mAsyncBuffer.size() >= mMaxBufferSize)
		{
			mBuffer.clear();
			bufferIndex = mAsyncBuffer.size() - mMaxBufferSize;
		}
		else if ((mBuffer.size() + mAsyncBuffer.size()) >= mMaxBufferSize)
		{
			while ((mMaxBufferSize - mBuffer.size()) < mAsyncBuffer.size())
				mBuffer.pop_back();
		}

		for (; bufferIndex < mAsyncBuffer.size(); bufferIndex++)
			mBuffer.push_front(mAsyncBuffer[bufferIndex]);

		assert(mBuffer.size() <= mMaxBufferSize);
	}

	for (const auto& entry : mAsyncBuffer)
		writeToFile(entry);

	mAsyncBuffer.clear();
}

void Logger::writeToFile(const EntryData& entry)
{
	if (!mOutFileStream)
		return;

	HorseRadish::Streams::StreamWriter streamWriter(*mOutFileStream);

	switch (entry.moduleType)
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

	switch (entry.entryType)
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

		std::time_t tmT = std::chrono::system_clock::to_time_t(entry.timestamp);
		std::tm* tmUTC = std::gmtime(&tmT);

		strftime(bufferTmp, sizeof(bufferTmp), "%Y-%m-%d %H:%M:%S", tmUTC);

		streamWriter.Write(bufferTmp, strlen(bufferTmp) - 1);
		streamWriter.WriteString("}\t", false);
	}

	if (!entry.isMsgFormated)
	{
		streamWriter.WriteString(entry.msg.c_str(), false);
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
				mOutFileStream->Flush();
		}
	}
}

bool Logger::addEntry(const EntryType entryType, const ModuleType moduleType, const char * const entryData)
{
	if (entryData == nullptr)
		return true;

	bool hasFormattedText;
	unsigned int entryDataSize;
	if (!Logger::checkEntryData(entryData, hasFormattedText, entryDataSize))
		return false;

	EntryData entry(entryType, moduleType);
	entry.msg = std::string(entryData, entryDataSize);
	entry.isMsgFormated = hasFormattedText;
	entry.timestamp = std::chrono::system_clock::now();

	std::lock_guard<std::mutex> lock(mASyncLock);

	if (mMaxAsyncBufferSize == 0)
	{
		if (mMaxBufferSize > 0)
		{
			std::unique_lock<std::mutex> lock(mBufferLock);

			if (mBuffer.size() >= mMaxBufferSize)
				mBuffer.pop_back();

			mBuffer.push_front(entry);
			assert(mBuffer.size() <= mMaxBufferSize);
		}

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

Logger::Logger(unsigned int asyncMaxEntries)
	: mMaxBufferSize(0), mMaxAsyncBufferSize(asyncMaxEntries)
	, mThreadFlush(nullptr), mThreadFlushExit(false)
{
	if (mMaxAsyncBufferSize > 0)
		mThreadFlush = new std::thread(&Logger::threadFlushFunc, this);
}

Logger::Logger(unsigned int asyncMaxEntries, unsigned int maxBufferedEntries)
	: Logger(asyncMaxEntries)
{
	mMaxBufferSize = maxBufferedEntries;
}

Logger::Logger(unsigned int asyncMaxEntries, unsigned int maxBufferedEntries, const HorseRadish::IO::Path &filePath)
	: Logger(asyncMaxEntries, maxBufferedEntries)
{
	mOutFileStream = std::shared_ptr<HorseRadish::Streams::FileStream>(new HorseRadish::Streams::FileStream(filePath.str(), false, true));
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

void Logger::iterateBuffer(std::function<bool(const EntryType, const ModuleType, const bool, const std::string&)> logEntryCb, unsigned int offset)
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
