#pragma once

#include "Path.hpp"
#include "Stream.hpp"
#include "String.hpp"
#include "Types.hpp"

#include "libs\zlib\zlib.h"
#include "libs\zlib\minizip\unzip.h"

#include <vector>
#include <memory>
#include <functional>

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
				HorseRadish::IO::Path mountPoint;

			public:
				MountData(const HorseRadish::hChar * const mountPoint);
				virtual ~MountData();

				virtual FileSystem::MountType GetMountType() const = 0;
				virtual void FilesEnumerate() = 0;
				virtual Streams::Stream* FileRead(const HorseRadish::hChar * const filePath) = 0;
				virtual bool FileExists(const HorseRadish::hChar * const filePath) = 0;
			};

			class MountDataPath : public MountData
			{
				HorseRadish::IO::Path baseFolder;

			public:
				MountDataPath(const HorseRadish::hChar * const baseFolder, const HorseRadish::hChar * const mountPoint);
				~MountDataPath();

				FileSystem::MountType GetMountType() const;

				void FilesEnumerate();
				Streams::Stream* FileRead(const HorseRadish::hChar * const filePath);
				bool FileExists(const HorseRadish::hChar * const filePath);
			};

			class MountDataZip : public MountData
			{
			private:
				struct ZipEntry
				{
					unz_file_pos filePos;
					int fileSize;
					hData128 fileNameMD5;
				};
				unzFile zipFile;
				HorseRadish::IO::Path zipPath;
				int numFolders, numFiles;
				std::vector<ZipEntry> listaEntradas;

			public:
				MountDataZip(const HorseRadish::hChar * const zipPath, const HorseRadish::hChar * const mountPoint);
				~MountDataZip();

				FileSystem::MountType GetMountType() const;
				int GetNumberFiles() const;

				void FilesEnumerate();
				Streams::Stream* FileRead(const HorseRadish::hChar * const filePath);
				bool FileExists(const HorseRadish::hChar * const filePath);
			};

			unsigned int maxNumMounts;
			std::vector<std::unique_ptr<MountData>> listMounts;
			std::vector<WatchChangeData> listWatchChange;

		public:
			FileSystem(unsigned int maxNumMounts);
			~FileSystem();

			static void FindFiles(const HorseRadish::hChar * const baseFolderAndFilter, const bool returnFilesFullPath, std::function<void(const HorseRadish::IO::Path &filePath, const HorseRadish::hUInt64 &fileSize)> actionFileFound);
			static bool FileExists(const HorseRadish::hChar * const filePath);

			bool MountPath(const HorseRadish::IO::Path &baseFolder, const HorseRadish::hChar * const mountPoint);
			bool MountZip(const HorseRadish::IO::Path &zipPath, const HorseRadish::hChar * const mountPoint, int * const numFilesZip = nullptr);

			Streams::Stream* FileRead(const char * const filePath);
			Streams::Stream* FileRead(const char * const filePath, const MountType mountType);

			int WatchChangeCreate(const HorseRadish::hChar * const baseFolder, bool includeSubFolders, const ChangeType changeType);
			void WatchChangeDelete(const int watchChangeID);
			bool WatchChanged(const int watchChangeID);
		};

	} //IO
} //HorseRadish

