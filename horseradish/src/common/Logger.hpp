#pragma once
#ifndef __HLOGGER__
#define __HLOGGER__

#include "Types.hpp"
#include "Path.hpp"
#include "Threadding.hpp"

#include <time.h>

namespace HorseRadish
{

namespace Logging
{

class Logger{

public:
	enum EntryType{
		Normal,
		Info,
		Warning,
		Error
		};

	static const int DefaultCapacityKB;

private:
	struct EntryHeader{
		hInt32 prevEntrySize, entrySize, metadataSize;
		hInt32 entryType;
		time_t entryTimestamp;
		hUInt8 entryCRC8;
		};
	struct EntryFooter{
		hInt32 entrySize;
		hUInt8 entryCRC8;
		};

	void *dataMain, *dataEnd, *dataNext;
	HorseRadish::IO::Path filePath;
	bool appendToFile;
	mutable HorseRadish::Threadding::Lock syncLock;
	int numberTotalEntries, numberCurrentEntries;

	static const EntryHeader* getNextEntryHeader(const EntryHeader *entryHeader);
	static const EntryHeader* getPreviousEntryHeader(const EntryHeader *entryHeader);
	static const void* getEntryMetada(const EntryHeader *entryHeader);
	static const void* getEntryData(const EntryHeader *entryHeader);
	static int getEntryMetadataSize(const EntryHeader *entryHeader);
	static int getEntryDataSize(const EntryHeader *entryHeader);

	int getFreeSpace() const;
	int getUsedSpace() const;
	void writeToFile(bool resetData);
	bool addEntry(const EntryType &entryType, const char * const entryData, const void * const metadata, int metadataSize);
	
public:
	Logger(int logCapacityKB);
	Logger(int logCapacityKB, const HorseRadish::IO::Path &filePath, bool appendToFile = false);
	~Logger();

	void Log(const EntryType &entryType, const char * const entryData);
	void Log(const EntryType &entryType, const void * const metadata, const int metadataSize, const char * const entryData);
	void Log(const char * const entryData);
	void Log(const void * const metadata, const int metadataSize, const char * const entryData);
	void LogInfo(const char * const entryData);
	void LogInfo(const void * const metadata, const int metadataSize, const char * const entryData);
	void LogWarning(const char * const entryData);
	void LogWarning(const void * const metadata, const int metadataSize, const char * const entryData);
	void LogError(const char * const entryData);
	void LogError(const void * const metadata, const int metadataSize, const char * const entryData);

	void Reset();
	void WriteToFile(bool resetData);

	int GetNumberTotalEntries() const { return this->numberTotalEntries; }
	int GetNumberCurrentEntries() const { return this->numberCurrentEntries; }

	void Iterate(bool fromBottom, std::function<bool (const char * const data, const int dataSize)> funcCallback, int numberOffset = 0) const;
	void Iterate(bool fromBottom, std::function<bool (const void * const metadata, const int metadataSize, const char * const data, const int dataSize)> funcCallback, int numberOffset = 0) const;
};

}//namespace Logging
}//namespace HorseRadish

#endif