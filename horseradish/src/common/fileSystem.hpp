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

namespace hr::io
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

			WatchChangeData() = default;

			WatchChangeData(int changeID, HANDLE changeHandle)
				: changeID(changeID)
				, changeHandle(changeHandle)
			{ }
		};

		class MountData
		{
		protected:
			hr::io::Path mMountPoint;

		public:
			MountData(std::string_view mountPoint);
			virtual ~MountData() = default;

			virtual FileSystem::MountType mountType() const = 0;
			virtual void filesEnumerate() = 0;
			virtual std::unique_ptr<streams::Stream> fileRead(std::string_view filePath) = 0;
			virtual bool fileExists(std::string_view filePath) = 0;
		};

		class MountDataPath : public MountData
		{
			hr::io::Path mBaseFolder;

		public:
			MountDataPath(std::string_view baseFolder, std::string_view mountPoint);

			FileSystem::MountType mountType() const;

			void filesEnumerate();
			std::unique_ptr<streams::Stream> fileRead(std::string_view filePath);
			bool fileExists(std::string_view filePath);
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
			hr::io::Path mZipPath;
			std::unordered_map<std::string, ZipEntry> mFileEntries;

		public:
			MountDataZip(std::string_view zipPath, std::string_view mountPoint);
			~MountDataZip();

			FileSystem::MountType mountType() const;
			size_t numberFiles() const;

			void filesEnumerate();
			std::unique_ptr<streams::Stream> fileRead(std::string_view filePath);
			bool fileExists(std::string_view filePath);
		};

		size_t mMaxNumMounts = 0;
		std::vector<std::unique_ptr<MountData>> mListMounts;
		std::vector<WatchChangeData> mListWatchChange;

	public:
		FileSystem(size_t maxNumMounts);
		~FileSystem();

		static void findFiles(const std::string& baseFolderAndFilter, const bool returnFilesFullPath, std::function<void(const hr::io::Path &filePath, const uint64_t &fileSize)> actionFileFound);
		static bool fileExists(std::string_view filePath);

		bool mountPath(const hr::io::Path &baseFolder, std::string_view mountPoint);
		bool mountZip(const hr::io::Path &zipPath, std::string_view mountPoint, size_t* const numFilesZip = nullptr);

		std::unique_ptr<streams::Stream> fileRead(std::string_view filePath);
		std::unique_ptr<streams::Stream> fileRead(std::string_view filePath, MountType mountType);

		std::string readFileAsString(std::string_view filePath);

		int watchChangeCreate(std::string_view baseFolder, bool includeSubFolders, ChangeType changeType);
		void watchChangeDelete(const int watchChangeID);
		bool watchChanged(const int watchChangeID);
	};
}
