#pragma once

#include "path.hpp"
#include "stream.hpp"
#include "types.hpp"

#include "libs/zlib/zlib.h"
#include "libs/zlib/minizip/unzip.h"

#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>

namespace HorseRadish { namespace IO
{
	class FileSystem
	{
	public:
		enum class MountType { Path, ZIP };
		enum ChangeType : size_t { FileName = (1 << 0), FileSize = (1 << 1), FileLastWrite = (1 << 2) };

		static const int FolderNameLength;
		static const int FileNameLength;
		static const int PathLength;

	private:
		class WatchChangeData
		{
		public:
			int changeID = 0;
			HANDLE changeHandle = nullptr;

			WatchChangeData()
			{ }

			WatchChangeData(int changeID, HANDLE changeHandle)
				: changeID(changeID)
				, changeHandle(changeHandle)
			{ }
		};

		class MountData
		{
		protected:
			HorseRadish::IO::Path mMountPoint;

		public:
			MountData(const char* const mountPoint);
			virtual ~MountData();

			virtual FileSystem::MountType mountType() const = 0;
			virtual void filesEnumerate() = 0;
			virtual std::unique_ptr<Streams::Stream> fileRead(const char* const filePath) = 0;
			virtual bool fileExists(const char* const filePath) = 0;
		};

		class MountDataPath : public MountData
		{
			HorseRadish::IO::Path mBaseFolder;

		public:
			MountDataPath(const char* const baseFolder, const char* const mountPoint);
			~MountDataPath();

			FileSystem::MountType mountType() const;

			void filesEnumerate();
			std::unique_ptr<Streams::Stream> fileRead(const char* const filePath);
			bool fileExists(const char* const filePath);
		};

		class MountDataZip : public MountData
		{
		private:
			struct ZipEntry
			{
				unz_file_pos filePos;
				size_t fileSize;
			};
			unzFile mZipFile;
			HorseRadish::IO::Path mZipPath;
			std::unordered_map<std::string, ZipEntry> mFileEntries;

		public:
			MountDataZip(const char* const zipPath, const char* const mountPoint);
			~MountDataZip();

			FileSystem::MountType mountType() const;
			size_t numberFiles() const;

			void filesEnumerate();
			std::unique_ptr<Streams::Stream> fileRead(const char* const filePath);
			bool fileExists(const char* const filePath);
		};

		size_t mMaxNumMounts = 0;
		std::vector<std::unique_ptr<MountData>> mListMounts;
		std::vector<WatchChangeData> mListWatchChange;

	public:
		FileSystem(size_t maxNumMounts);
		~FileSystem();

		static void findFiles(const std::string& baseFolderAndFilter, const bool returnFilesFullPath, std::function<void(const HorseRadish::IO::Path &filePath, const HorseRadish::hUInt64 &fileSize)> actionFileFound);
		static bool fileExists(const char* const filePath);

		bool mountPath(const HorseRadish::IO::Path &baseFolder, const char* const mountPoint);
		bool mountZip(const HorseRadish::IO::Path &zipPath, const char* const mountPoint, size_t* const numFilesZip = nullptr);

		std::unique_ptr<Streams::Stream> fileRead(const char * const filePath);
		std::unique_ptr<Streams::Stream> fileRead(const char * const filePath, MountType mountType);

		std::string readFileAsString(const char* const filePath);

		int watchChangeCreate(const char* const baseFolder, bool includeSubFolders, ChangeType changeType);
		void watchChangeDelete(const int watchChangeID);
		bool watchChanged(const int watchChangeID);
	};
} }
