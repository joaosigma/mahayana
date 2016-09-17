#pragma once

#include "Path.hpp"
#include "Stream.hpp"
#include "Types.hpp"

#include "libs\zlib\zlib.h"
#include "libs\zlib\minizip\unzip.h"

#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>

namespace HorseRadish
{
	namespace IO
	{
		class FileSystem
		{
		public:
			enum MountType { MountTypePath, MountTypeZIP };
			enum ChangeType { FileName = (1 << 0), FileSize = (1 << 1), FileLastWrite = (1 << 2) };

			static const int FolderNameLength;
			static const int FileNameLength;
			static const int PathLength;

		private:
			class WatchChangeData
			{
			public:
				int changeID;
				HANDLE changeHandle;

				WatchChangeData() : changeID(0), changeHandle(nullptr) { }
				WatchChangeData(int changeID, HANDLE changeHandle) : changeID(changeID), changeHandle(changeHandle) { }
			};

			class MountData
			{
			protected:
				HorseRadish::IO::Path mMountPoint;

			public:
				MountData(const char* const mountPoint);
				virtual ~MountData();

				virtual FileSystem::MountType GetMountType() const = 0;
				virtual void FilesEnumerate() = 0;
				virtual std::unique_ptr<Streams::Stream> FileRead(const char* const filePath) = 0;
				virtual bool FileExists(const char* const filePath) = 0;
			};

			class MountDataPath : public MountData
			{
				HorseRadish::IO::Path mBaseFolder;

			public:
				MountDataPath(const char* const baseFolder, const char* const mountPoint);
				~MountDataPath();

				FileSystem::MountType GetMountType() const;

				void FilesEnumerate();
				std::unique_ptr<Streams::Stream> FileRead(const char* const filePath);
				bool FileExists(const char* const filePath);
			};

			class MountDataZip : public MountData
			{
			private:
				struct ZipEntry
				{
					unz_file_pos filePos;
					int fileSize;
				};
				unzFile mZipFile;
				HorseRadish::IO::Path mZipPath;
				std::unordered_map<std::string, ZipEntry> mFileEntries;

			public:
				MountDataZip(const char* const zipPath, const char* const mountPoint);
				~MountDataZip();

				FileSystem::MountType GetMountType() const;
				size_t GetNumberFiles() const;

				void FilesEnumerate();
				std::unique_ptr<Streams::Stream> FileRead(const char* const filePath);
				bool FileExists(const char* const filePath);
			};

			unsigned int mMaxNumMounts;
			std::vector<std::unique_ptr<MountData>> mListMounts;
			std::vector<WatchChangeData> mListWatchChange;

		public:
			FileSystem(unsigned int maxNumMounts);
			~FileSystem();

			static void FindFiles(const std::string& baseFolderAndFilter, const bool returnFilesFullPath, std::function<void(const HorseRadish::IO::Path &filePath, const HorseRadish::hUInt64 &fileSize)> actionFileFound);
			static bool FileExists(const char* const filePath);

			bool MountPath(const HorseRadish::IO::Path &baseFolder, const char* const mountPoint);
			bool MountZip(const HorseRadish::IO::Path &zipPath, const char* const mountPoint, size_t* const numFilesZip = nullptr);

			std::unique_ptr<Streams::Stream> FileRead(const char * const filePath);
			std::unique_ptr<Streams::Stream> FileRead(const char * const filePath, const MountType mountType);

			std::string readFileAsString(const char* const filePath);

			int WatchChangeCreate(const char* const baseFolder, bool includeSubFolders, const ChangeType changeType);
			void WatchChangeDelete(const int watchChangeID);
			bool WatchChanged(const int watchChangeID);
		};

	} //IO
} //HorseRadish

