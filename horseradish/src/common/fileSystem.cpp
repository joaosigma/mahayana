#include "fileSystem.hpp"

#include "math.hpp"
#include "hashing.hpp"
#include "sorting.hpp"
#include "types.hpp"
#include "stringUtils.hpp"

#include <algorithm>

namespace hr::io
{
	namespace
	{
		uLong ZCALLBACK zwrite(voidpf, voidpf, const void*, uLong)
		{
			return 0;
		}

		int ZCALLBACK zerror(voidpf, voidpf)
		{
			return 0;
		}

		voidpf ZCALLBACK zopen(voidpf, const char* filename, int)
		{
			auto fileStream = new hr::streams::FileStream(filename, true, false);

			return ((voidpf)fileStream);
		}

		uLong ZCALLBACK zread(voidpf, voidpf stream, void* buf, uLong size)
		{
			auto fileStream = reinterpret_cast<hr::streams::FileStream*>(stream);

			return fileStream->read(buf, size);
		}

		long ZCALLBACK ztell(voidpf, voidpf stream)
		{
			auto fileStream = reinterpret_cast<hr::streams::FileStream*>(stream);

			return fileStream->position();
		}

		long ZCALLBACK zseek(voidpf, voidpf stream, uLong offset, int origin)
		{
			auto fileStream = reinterpret_cast<hr::streams::FileStream*>(stream);

			if (origin == ZLIB_FILEFUNC_SEEK_CUR)
				fileStream->seek(hr::streams::Stream::SeekOrigin::Current, offset);
			else if (origin == ZLIB_FILEFUNC_SEEK_END)
				fileStream->seek(hr::streams::Stream::SeekOrigin::End, offset);
			else if (origin == ZLIB_FILEFUNC_SEEK_SET)
				fileStream->seek(hr::streams::Stream::SeekOrigin::Begin, offset);
			else
				return 1;

			return 0;
		}

		int ZCALLBACK zclose(voidpf, voidpf stream)
		{
			auto fileStream = reinterpret_cast<hr::streams::FileStream*>(stream);

			fileStream->close();
			delete fileStream;

			return 0;
		}

		void overloadZLibIO(zlib_filefunc_def * const zlibFileFunc)
		{
			if (!zlibFileFunc)
				return;

			zlibFileFunc->zclose_file = zclose;
			zlibFileFunc->zopen_file = zopen;
			zlibFileFunc->zread_file = zread;
			zlibFileFunc->zseek_file = zseek;
			zlibFileFunc->ztell_file = ztell;

			zlibFileFunc->zerror_file = zerror;
			zlibFileFunc->zwrite_file = zwrite;
		}
	}

	FileSystem::MountData::MountData(std::string_view mountPoint)
	{
		mMountPoint.set(mountPoint);
	}
	
	FileSystem::MountDataPath::MountDataPath(std::string_view baseFolder, std::string_view mountPoint)
		: MountData(mountPoint)
	{
		mBaseFolder.set(baseFolder);
	}

	FileSystem::MountType FileSystem::MountDataPath::mountType() const
	{
		return FileSystem::MountType::Path;
	}

	void FileSystem::MountDataPath::filesEnumerate()
	{
	}

	std::unique_ptr<streams::Stream> FileSystem::MountDataPath::fileRead(std::string_view filePath)
	{
		auto pathFinal = mBaseFolder;
		pathFinal += filePath;

		auto fileStream = std::unique_ptr<streams::FileStream>(new streams::FileStream(pathFinal.str(), true, false));

		if (!fileStream->isValid())
			std::unique_ptr<streams::FileStream>();

		return std::move(fileStream);
	}

	bool FileSystem::MountDataPath::fileExists(std::string_view filePath)
	{
		auto pathFinal = mBaseFolder;
		pathFinal += filePath;

		return FileSystem::fileExists(pathFinal.str().c_str());
	}

	FileSystem::MountDataZip::MountDataZip(std::string_view zipPath, std::string_view mountPoint)
		: MountData(mountPoint), mZipFile(nullptr)
	{
		zlib_filefunc_def zlibAPI;
		char zipFileCurFileName[1024];

		mZipPath.set(zipPath);

		overloadZLibIO(&zlibAPI);

		mZipFile = unzOpen2(mZipPath.str().c_str(), &zlibAPI);
		if (!mZipFile)
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

		if (mZipFile)
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

	std::unique_ptr<streams::Stream> FileSystem::MountDataZip::fileRead(std::string_view filePath)
	{
		if (filePath.empty())
			return nullptr;

		auto itFile = mFileEntries.find(std::string(filePath));
		if (itFile == mFileEntries.end())
			return nullptr;

		std::shared_ptr<unsigned char> fileData(new unsigned char[itFile->second.fileSize], std::default_delete<unsigned char[]>());

		unzGoToFilePos(mZipFile, &itFile->second.filePos);

		unzOpenCurrentFile(mZipFile);

		unzReadCurrentFile(mZipFile, fileData.get(), itFile->second.fileSize);

		unzCloseCurrentFile(mZipFile);

		auto memStream = std::unique_ptr<streams::Stream>(new streams::MemoryViewStream(fileData, itFile->second.fileSize));
		return std::move(memStream);
	}

	bool FileSystem::MountDataZip::fileExists(std::string_view filePath)
	{
		if (filePath.empty())
			return false;

		return (mFileEntries.find(std::string(filePath)) != mFileEntries.end());
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

	void FileSystem::findFiles(const std::string& baseFolderAndFilter, const bool returnFilesFullPath, std::function<void(const hr::io::Path &filePath, const hr::hUInt64 &fileSize)> actionFileFound)
	{
		if (!actionFileFound || baseFolderAndFilter.empty())
			return;

		HANDLE handleFind;
		WIN32_FIND_DATA findData;

		{
			auto baseFolderAndFilterWChar = hr::StringUtils::conv2UTF16(baseFolderAndFilter);

			handleFind = FindFirstFile(baseFolderAndFilterWChar.c_str(), &findData);
			if (handleFind == INVALID_HANDLE_VALUE)
				return;
		}

		hr::io::Path basePath;
		basePath.set(baseFolderAndFilter);
		basePath.removeFile();

		do
		{
			if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				continue;

			auto filePath = hr::StringUtils::conv2UTF8(findData.cFileName);

			ULARGE_INTEGER ul;
			ul.HighPart = findData.nFileSizeHigh;
			ul.LowPart = findData.nFileSizeLow;
			hr::hUInt64 fileSize = ul.QuadPart;

			hr::io::Path fileFinalPath;

			if (returnFilesFullPath)
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

	bool FileSystem::fileExists(std::string_view filePath)
	{
		if (filePath.empty())
			return false;

		DWORD fileAtributes;
		{
			auto filePathWChar = hr::StringUtils::conv2UTF16(std::string(filePath));

			fileAtributes = GetFileAttributes(filePathWChar.c_str());
		}

		if (fileAtributes == INVALID_FILE_ATTRIBUTES)
			return false;

		if ((fileAtributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
			return false;

		return true;
	}

	bool FileSystem::mountPath(const hr::io::Path &baseFolder, std::string_view mountPoint)
	{
		if (mListMounts.size() >= mMaxNumMounts)
			return false;

		mListMounts.push_back(std::unique_ptr<MountDataPath>(new MountDataPath(baseFolder.str().c_str(), mountPoint)));
		return true;
	}

	bool FileSystem::mountZip(const hr::io::Path &zipPath, std::string_view mountPoint, size_t* const numFilesZip)
	{
		if (mListMounts.size() >= mMaxNumMounts)
			return false;

		mListMounts.push_back(std::unique_ptr<MountDataZip>(new MountDataZip(zipPath.str().c_str(), mountPoint)));

		if (numFilesZip)
			*numFilesZip = static_cast<MountDataZip*>(mListMounts[mListMounts.size() - 1].get())->numberFiles();

		return true;
	}

	std::unique_ptr<streams::Stream> FileSystem::fileRead(std::string_view filePath)
	{
		if (filePath.empty())
			return nullptr;

		for (auto& curMount : mListMounts)
		{
			if (!curMount->fileExists(filePath))
				continue;

			return curMount->fileRead(filePath);
		}

		return nullptr;
	}

	std::unique_ptr<streams::Stream> FileSystem::fileRead(std::string_view filePath, FileSystem::MountType mountType)
	{
		if (filePath.empty())
			return nullptr;

		for (auto& curMount : mListMounts)
		{
			if (curMount->mountType() != mountType)
				continue;

			if (!curMount->fileExists(filePath))
				continue;

			return curMount->fileRead(filePath);
		}

		return nullptr;
	}

	std::string FileSystem::readFileAsString(std::string_view filePath)
	{
		auto fileStream = fileRead(filePath);
		if (!fileStream)
			return std::string();

		hr::streams::MemoryViewStream fileData;
		if (!fileStream->cloneAllContent(fileData))
			return std::string();

		return fileData.toStr();
	}

	int FileSystem::watchChangeCreate(std::string_view baseFolder, bool includeSubFolders, FileSystem::ChangeType changeType)
	{
		HANDLE handleChange;

		if (baseFolder.empty())
			return -1;

		{
			auto baseFolderWChar = hr::StringUtils::conv2UTF16(std::string(baseFolder));

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
}