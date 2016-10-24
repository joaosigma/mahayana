#include "FileSystem.hpp"

#include "Math.hpp"
#include "Hashing.hpp"
#include "Sorting.hpp"
#include "Types.hpp"
#include "stringUtils.hpp"

#include <algorithm>

static
uLong ZCALLBACK zwrite(voidpf opaque, voidpf stream, const void* buf, uLong size)
{	return 0; }

static
int ZCALLBACK zerror(voidpf opaque, voidpf stream)
{	return 0; }

static
voidpf ZCALLBACK zopen(voidpf opaque, const char* filename, int mode)
{
	auto fileStream = new HorseRadish::Streams::FileStream(filename, true, false);
	
	return ((voidpf)fileStream);
}

static
uLong ZCALLBACK zread(voidpf opaque, voidpf stream, void* buf, uLong size)
{
	auto fileStream = reinterpret_cast<HorseRadish::Streams::FileStream*>(stream);

	return fileStream->read(buf,size);
}

static
long ZCALLBACK ztell(voidpf opaque, voidpf stream)
{
	auto fileStream = reinterpret_cast<HorseRadish::Streams::FileStream*>(stream);

	return fileStream->position();
}

static
long ZCALLBACK zseek(voidpf opaque, voidpf stream, uLong offset, int origin)
{
	auto fileStream = reinterpret_cast<HorseRadish::Streams::FileStream*>(stream);

	if (origin == ZLIB_FILEFUNC_SEEK_CUR)
		fileStream->seek(HorseRadish::Streams::Stream::SeekOrigin::Current, offset);
	else if (origin == ZLIB_FILEFUNC_SEEK_END)
		fileStream->seek(HorseRadish::Streams::Stream::SeekOrigin::End, offset);
	else if (origin == ZLIB_FILEFUNC_SEEK_SET)
		fileStream->seek(HorseRadish::Streams::Stream::SeekOrigin::Begin, offset);
	else
		return 1;

	return 0;
}

static
int ZCALLBACK zclose(voidpf opaque, voidpf stream)
{
	auto fileStream = reinterpret_cast<HorseRadish::Streams::FileStream*>(stream);

	fileStream->close();
	delete fileStream;

	return 0;
}

static
void overloadZLibIO(zlib_filefunc_def * const zlibFileFunc)
{
	if (zlibFileFunc == nullptr)
		return;

	zlibFileFunc->zclose_file = zclose;
	zlibFileFunc->zopen_file = zopen;
	zlibFileFunc->zread_file = zread;
	zlibFileFunc->zseek_file = zseek;
	zlibFileFunc->ztell_file = ztell;

	zlibFileFunc->zerror_file = zerror;
	zlibFileFunc->zwrite_file = zwrite;
}

namespace HorseRadish
{
	namespace IO
	{
		FileSystem::MountData::MountData(const char* const mountPoint)
		{
			mMountPoint.set(mountPoint);
		}
		FileSystem::MountData::~MountData()
		{
			mMountPoint.clear();
		}

		FileSystem::MountDataPath::MountDataPath(const char* const baseFolder, const char* const mountPoint)
			: MountData(mountPoint)
		{
			mBaseFolder.set(baseFolder);
		}

		FileSystem::MountDataPath::~MountDataPath()
		{
			mBaseFolder.clear();
		}

		FileSystem::MountType FileSystem::MountDataPath::mountType() const
		{
			return FileSystem::MountType::Path;
		}

		void FileSystem::MountDataPath::filesEnumerate()
		{
		}

		std::unique_ptr<Streams::Stream> FileSystem::MountDataPath::fileRead(const char* const filePath)
		{
			auto pathFinal = mBaseFolder;
			pathFinal += filePath;

			auto fileStream = std::unique_ptr<Streams::FileStream>(new Streams::FileStream(pathFinal.str(), true, false));

			if (fileStream->isValid() == false)
				std::unique_ptr<Streams::FileStream>();

			return std::move(fileStream);
		}

		bool FileSystem::MountDataPath::fileExists(const char* const filePath)
		{
			auto pathFinal = mBaseFolder;
			pathFinal += filePath;

			return FileSystem::fileExists(pathFinal.str().c_str());
		}

		FileSystem::MountDataZip::MountDataZip(const char* const zipPath, const char* const mountPoint)
			: MountData(mountPoint), mZipFile(nullptr)
		{
			zlib_filefunc_def zlibAPI;
			char zipFileCurFileName[1024];

			mZipPath.set(zipPath);

			overloadZLibIO(&zlibAPI);

			mZipFile = unzOpen2(mZipPath.str().c_str(), &zlibAPI);
			if (mZipFile == nullptr)
				return;

			unsigned int numEntries = 0;
			{
				unz_global_info zipInfo;

				unzGetGlobalInfo(mZipFile, &zipInfo);
				numEntries = zipInfo.number_entry;
			}

			mFileEntries.rehash(numEntries);

			unzGoToFirstFile(mZipFile);
			for (unsigned int entryIndex = 0; entryIndex < numEntries; entryIndex++)
			{
				unz_file_info zipFileCurFileInfo;
				unzGetCurrentFileInfo(mZipFile, &zipFileCurFileInfo, zipFileCurFileName, sizeof(zipFileCurFileName), nullptr, 0, nullptr, 0);

				if (zipFileCurFileInfo.uncompressed_size > 0) //ignore folders
				{
					ZipEntry zipEntry;
					zipEntry.fileSize = zipFileCurFileInfo.uncompressed_size;
					unzGetFilePos(mZipFile, &zipEntry.filePos);

					mFileEntries[std::string(zipFileCurFileName, zipFileCurFileInfo.size_filename)] = zipEntry;
				}

				unzGoToNextFile(mZipFile);
			}
		}

		FileSystem::MountDataZip::~MountDataZip()
		{
			mFileEntries.clear();
			mZipPath.clear();

			if (mZipFile != nullptr)
				unzClose(mZipFile);
			mZipFile = nullptr;
		}

		FileSystem::MountType FileSystem::MountDataZip::mountType() const
		{
			return FileSystem::MountType::ZIP;
		}

		size_t FileSystem::MountDataZip::numberFiles() const
		{
			return mFileEntries.size();
		}

		void FileSystem::MountDataZip::filesEnumerate()
		{
		}

		std::unique_ptr<Streams::Stream> FileSystem::MountDataZip::fileRead(const char* const filePath)
		{
			if ((filePath == nullptr) || (*filePath == '\0'))
				return std::unique_ptr<Streams::Stream>();

			auto itFile = mFileEntries.find(filePath);
			if (itFile == mFileEntries.end())
				return nullptr;

			std::shared_ptr<unsigned char> fileData(new unsigned char[itFile->second.fileSize], std::default_delete<unsigned char[]>());

			unzGoToFilePos(mZipFile, &itFile->second.filePos);

			unzOpenCurrentFile(mZipFile);

			unzReadCurrentFile(mZipFile, fileData.get(), itFile->second.fileSize);

			unzCloseCurrentFile(mZipFile);

			auto memStream = std::unique_ptr<Streams::Stream>(new Streams::MemoryViewStream(fileData, itFile->second.fileSize));
			return std::move(memStream);
		}

		bool FileSystem::MountDataZip::fileExists(const char* const filePath)
		{
			if ((filePath == nullptr) || (*filePath == '\0'))
				return false;

			return (mFileEntries.find(filePath) != mFileEntries.end());
		}

		const int FileSystem::FolderNameLength = 128;
		const int FileSystem::FileNameLength = 256;
		const int FileSystem::PathLength = 16383;

		FileSystem::FileSystem(size_t maxNumMounts)
		{
			mMaxNumMounts = (maxNumMounts < 1) ? 1 : ((maxNumMounts > 10) ? 10 : maxNumMounts);

			mListMounts.reserve(mMaxNumMounts);
		}

		FileSystem::~FileSystem()
		{
			for (auto& change : mListWatchChange)
				FindCloseChangeNotification(change.changeHandle);
			mListWatchChange.clear();

			mListMounts.clear();
			mMaxNumMounts = 0;
		}

		void FileSystem::findFiles(const std::string& baseFolderAndFilter, const bool returnFilesFullPath, std::function<void(const HorseRadish::IO::Path &filePath, const HorseRadish::hUInt64 &fileSize)> actionFileFound)
		{
			HANDLE handleFind;
			WIN32_FIND_DATA findData;
			HorseRadish::IO::Path basePath, fileFinalPath;

			if (baseFolderAndFilter.empty() || (actionFileFound == nullptr))
				return;

			{
				auto baseFolderAndFilterWChar = HorseRadish::StringUtils::conv2UTF16(baseFolderAndFilter);

				handleFind = FindFirstFile(baseFolderAndFilterWChar.c_str(), &findData);
				if (handleFind == INVALID_HANDLE_VALUE)
					return;
			}

			basePath.set(baseFolderAndFilter);
			basePath.removeFile();

			do
			{
				if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
					continue;

				auto filePath = HorseRadish::StringUtils::conv2UTF8(findData.cFileName);

				HorseRadish::hUInt64 fileSize = (findData.nFileSizeHigh * (MAXDWORD + 1)) + findData.nFileSizeLow;

				if (returnFilesFullPath == true)
				{
					fileFinalPath = basePath;
					fileFinalPath.combine(filePath);
				}
				else
				{
					fileFinalPath.set(filePath);
				}

				actionFileFound(fileFinalPath, fileSize);

			} while (FindNextFile(handleFind, &findData) != 0);


			FindClose(handleFind);
		}

		bool FileSystem::fileExists(const char* const filePath)
		{
			DWORD fileAtributes;

			if ((filePath == nullptr) || (*filePath == '\0'))
				return false;

			{
				auto filePathWChar = HorseRadish::StringUtils::conv2UTF16(filePath);

				fileAtributes = GetFileAttributes(filePathWChar.c_str());
			}

			if (fileAtributes == INVALID_FILE_ATTRIBUTES)
				return false;

			if ((fileAtributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
				return false;

			return true;
		}

		bool FileSystem::mountPath(const HorseRadish::IO::Path &baseFolder, const char* const mountPoint)
		{
			if (mListMounts.size() >= mMaxNumMounts)
				return false;

			mListMounts.push_back(std::unique_ptr<MountDataPath>(new MountDataPath(baseFolder.str().c_str(), mountPoint)));
			return true;
		}

		bool FileSystem::mountZip(const HorseRadish::IO::Path &zipPath, const char* const mountPoint, size_t* const numFilesZip)
		{
			if (mListMounts.size() >= mMaxNumMounts)
				return false;

			mListMounts.push_back(std::unique_ptr<MountDataZip>(new MountDataZip(zipPath.str().c_str(), mountPoint)));

			if (numFilesZip != nullptr)
				*numFilesZip = static_cast<MountDataZip*>(mListMounts[mListMounts.size() - 1].get())->numberFiles();

			return true;
		}

		std::unique_ptr<Streams::Stream> FileSystem::fileRead(const char * const filePath)
		{
			if ((filePath == nullptr) || (*filePath == '\0'))
				return std::unique_ptr<Streams::Stream>();

			for (auto& curMount : mListMounts)
			{
				if (!curMount->fileExists(filePath))
					continue;

				return curMount->fileRead(filePath);
			}

			return std::unique_ptr<Streams::Stream>();
		}

		std::unique_ptr<Streams::Stream> FileSystem::fileRead(const char * const filePath, FileSystem::MountType mountType)
		{
			if ((filePath == nullptr) || (*filePath == '\0'))
				return std::unique_ptr<Streams::Stream>();

			for (auto& curMount : mListMounts)
			{
				if (curMount->mountType() != mountType)
					continue;

				if (!curMount->fileExists(filePath))
					continue;

				return curMount->fileRead(filePath);
			}

			return std::unique_ptr<Streams::Stream>();
		}

		std::string FileSystem::readFileAsString(const char * const filePath)
		{
			auto fileStream = fileRead(filePath);
			if (!fileStream)
				return std::string();

			auto fileData = fileStream->readEntireContent();
			if (!fileData)
				return std::string();

			return fileData->toStr();
		}

		int FileSystem::watchChangeCreate(const char* const baseFolder, bool includeSubFolders, FileSystem::ChangeType changeType)
		{
			HANDLE handleChange;

			if ((baseFolder == nullptr) || (changeType == 0))
				return -1;

			{
				auto baseFolderWChar = HorseRadish::StringUtils::conv2UTF16(baseFolder);

				DWORD changeFlags = 0;
				if (changeType & FileName)
					changeFlags = FILE_NOTIFY_CHANGE_FILE_NAME;
				if (changeType & FileSize)
					changeFlags = FILE_NOTIFY_CHANGE_SIZE;
				if (changeType & FileLastWrite)
					changeFlags = FILE_NOTIFY_CHANGE_LAST_WRITE;

				handleChange = FindFirstChangeNotification(baseFolderWChar.c_str(), includeSubFolders ? TRUE : FALSE, changeFlags);
				if (handleChange == INVALID_HANDLE_VALUE)
					return 0;
			}

			auto changeID = mListWatchChange.size() + 13;
			mListWatchChange.push_back(WatchChangeData(changeID, handleChange));

			return changeID;
		}

		void FileSystem::watchChangeDelete(const int watchChangeID)
		{
			auto changeIndex = std::find_if(mListWatchChange.begin(), mListWatchChange.end(), [&watchChangeID](const WatchChangeData &data){ return data.changeID == watchChangeID; });
			if (changeIndex == mListWatchChange.end())
				return;

			FindCloseChangeNotification(changeIndex->changeHandle);
			mListWatchChange.erase(changeIndex);
		}

		bool FileSystem::watchChanged(const int watchChangeID)
		{
			auto changeIndex = std::find_if(mListWatchChange.begin(), mListWatchChange.end(), [&watchChangeID](const WatchChangeData &data){ return data.changeID == watchChangeID; });
			if (changeIndex == mListWatchChange.end())
				return false;

			auto didChange = (WaitForSingleObject(changeIndex->changeHandle, 0) == WAIT_OBJECT_0);
			if (didChange)
				FindNextChangeNotification(changeIndex->changeHandle);

			return didChange;
		}

	} //IO
} //HorseRadish