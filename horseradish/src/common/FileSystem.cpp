#include "FileSystem.hpp"

#include "Math.hpp"
#include "Hashing.hpp"
#include "Sorting.hpp"
#include "UTF.hpp"
#include "Types.hpp"

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
	auto fileStream = new HorseRadish::Streams::FileStream((const HorseRadish::hChar*)filename, true, false);
	
	return ((voidpf)fileStream);
}

static
uLong ZCALLBACK zread(voidpf opaque, voidpf stream, void* buf, uLong size)
{
	auto fileStream = reinterpret_cast<HorseRadish::Streams::FileStream*>(stream);

	return fileStream->Read(buf,size);
}

static
long ZCALLBACK ztell(voidpf opaque, voidpf stream)
{
	auto fileStream = reinterpret_cast<HorseRadish::Streams::FileStream*>(stream);

	return fileStream->GetPosition();
}

static
long ZCALLBACK zseek(voidpf opaque, voidpf stream, uLong offset, int origin)
{
	auto fileStream = reinterpret_cast<HorseRadish::Streams::FileStream*>(stream);

	if (origin == ZLIB_FILEFUNC_SEEK_CUR)
		fileStream->Seek(offset, HorseRadish::Streams::Stream::Current);
	else if (origin == ZLIB_FILEFUNC_SEEK_END)
		fileStream->Seek(offset, HorseRadish::Streams::Stream::End);
	else if (origin == ZLIB_FILEFUNC_SEEK_SET)
		fileStream->Seek(offset, HorseRadish::Streams::Stream::Begin);
	else
		return 1;

	return 0;
}

static
int ZCALLBACK zclose(voidpf opaque, voidpf stream)
{
	auto fileStream = reinterpret_cast<HorseRadish::Streams::FileStream*>(stream);

	fileStream->Close();
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
		FileSystem::MountData::MountData(const HorseRadish::hChar * const mountPoint)
		{
			this->mountPoint.Set(mountPoint);
		}
		FileSystem::MountData::~MountData()
		{
			this->mountPoint.Clear();
		}

		FileSystem::MountDataPath::MountDataPath(const HorseRadish::hChar * const baseFolder, const HorseRadish::hChar * const mountPoint)
			: MountData(mountPoint)
		{
			this->baseFolder.Set(baseFolder);
		}

		FileSystem::MountDataPath::~MountDataPath()
		{
			this->baseFolder.Clear();
		}

		FileSystem::MountType FileSystem::MountDataPath::GetMountType() const
		{
			return FileSystem::MountTypePath;
		}

		void FileSystem::MountDataPath::FilesEnumerate()
		{
		}

		Streams::Stream* FileSystem::MountDataPath::FileRead(const HorseRadish::hChar * const filePath)
		{
			HorseRadish::IO::Path pathFinal;

			pathFinal.Set(this->baseFolder);
			pathFinal += (const HorseRadish::hChar*)filePath;

			auto fileStream = new HorseRadish::Streams::FileStream(pathFinal, true, false);

			if (fileStream->IsValid() == false)
			{
				delete fileStream;
				return nullptr;
			}

			return fileStream;
		}

		bool FileSystem::MountDataPath::FileExists(const HorseRadish::hChar * const filePath)
		{
			HorseRadish::IO::Path pathFinal;

			pathFinal.Set(this->baseFolder);
			pathFinal += (const HorseRadish::hChar*)filePath;

			return FileSystem::FileExists(pathFinal);
		}

		FileSystem::MountDataZip::MountDataZip(const HorseRadish::hChar * const zipPath, const HorseRadish::hChar * const mountPoint)
			: MountData(mountPoint)
			, zipFile(nullptr), numFolders(0), numFiles(0)
		{
			zlib_filefunc_def zlibAPI;
			char zipFileCurFileName[1024];

			this->zipPath.Set(zipPath);

			overloadZLibIO(&zlibAPI);

			this->zipFile = unzOpen2(this->zipPath.GetData(), &zlibAPI);
			if (this->zipFile == nullptr)
				return;

			{
				unz_global_info zipInfo;

				unzGetGlobalInfo(this->zipFile, &zipInfo);
				this->listaEntradas.resize(zipInfo.number_entry);
			}

			unzGoToFirstFile(this->zipFile);

			for (auto &curEntry : this->listaEntradas)
			{
				curEntry.fileSize = 0;
				memset(&curEntry.filePos, 0, sizeof(curEntry.filePos));
				memset(&curEntry.fileNameMD5, 0, sizeof(curEntry.fileNameMD5));

				unz_file_info zipFileCurFileInfo;
				unzGetCurrentFileInfo(this->zipFile, &zipFileCurFileInfo, zipFileCurFileName, sizeof(zipFileCurFileName), nullptr, 0, nullptr, 0);

				if (zipFileCurFileInfo.uncompressed_size == 0)
				{
					this->numFolders++;
				}
				else
				{
					this->numFiles++;

					curEntry.fileSize = zipFileCurFileInfo.uncompressed_size;
					unzGetFilePos(this->zipFile, &curEntry.filePos);

					HorseRadish::Hashing::CalculateMD5(zipFileCurFileName, zipFileCurFileInfo.size_filename, &curEntry.fileNameMD5);
				}

				unzGoToNextFile(this->zipFile);
			}

			std::sort(this->listaEntradas.begin(), this->listaEntradas.end(), [](const ZipEntry &a, const ZipEntry &b) { return (a.fileNameMD5 < b.fileNameMD5); });

			//para todos os ficheiros que tenho
			/*for (int i=0; i < (this->numFiles - 1); i++)
			{
			if (this->listaEntradas[i].fileNameMD5.i64[0] != this->listaEntradas[i+1].fileNameMD5.i64[0])
			continue;
			if (this->listaEntradas[i].fileNameMD5.i64[1] != this->listaEntradas[i+1].fileNameMD5.i64[1])
			continue;

			this->numEntradas = 0;
			this->numFiles = 0;
			this->numFolders = 0;
			return;
			}*/
		}

		FileSystem::MountDataZip::~MountDataZip()
		{
			this->zipPath.Clear();
			this->listaEntradas.clear();
			this->numFolders = 0;
			this->numFiles = 0;

			if (this->zipFile != nullptr)
				unzClose(this->zipFile);
			this->zipFile = nullptr;
		}

		FileSystem::MountType FileSystem::MountDataZip::GetMountType() const
		{
			return FileSystem::MountTypeZIP;
		}

		int FileSystem::MountDataZip::GetNumberFiles() const
		{
			return this->numFiles;
		}

		void FileSystem::MountDataZip::FilesEnumerate()
		{
		}

		Streams::Stream* FileSystem::MountDataZip::FileRead(const HorseRadish::hChar * const filePath)
		{
			if ((filePath == nullptr) || (*filePath == '\0'))
				return nullptr;

			ZipEntry fileProxy;
			memset(&fileProxy, 0, sizeof(ZipEntry));
			HorseRadish::Hashing::CalculateMD5(filePath, strlen((const char*)filePath), &fileProxy.fileNameMD5);

			auto fileZipIndex = std::lower_bound(this->listaEntradas.begin(), this->listaEntradas.end(), fileProxy, [](const ZipEntry &a, const ZipEntry &b) { return (a.fileNameMD5 < b.fileNameMD5); });

			if (fileZipIndex == this->listaEntradas.end())
				return nullptr;

			auto fileData = malloc(fileZipIndex->fileSize);
			if (fileData == nullptr)
				return nullptr;

			unzGoToFilePos(this->zipFile, &fileZipIndex->filePos);

			unzOpenCurrentFile(this->zipFile);

			unzReadCurrentFile(this->zipFile, fileData, fileZipIndex->fileSize);

			unzCloseCurrentFile(this->zipFile);

			auto memStream = new Streams::MemoryStream(fileData, fileZipIndex->fileSize, false, Streams::MemoryStream::ManagementType::None);
			if (memStream == nullptr)
			{
				free(fileData);
				return nullptr;
			}

			return memStream;
		}

		bool FileSystem::MountDataZip::FileExists(const HorseRadish::hChar * const filePath)
		{
			if ((filePath == nullptr) || (*filePath == '\0'))
				return false;

			ZipEntry fileProxy;
			memset(&fileProxy, 0, sizeof(ZipEntry));
			HorseRadish::Hashing::CalculateMD5(filePath, strlen((const char*)filePath), &fileProxy.fileNameMD5);

			auto fileZipIndex = std::lower_bound(this->listaEntradas.begin(), this->listaEntradas.end(), fileProxy, [](const ZipEntry &a, const ZipEntry &b) { return (a.fileNameMD5 < b.fileNameMD5); });
			return (fileZipIndex != this->listaEntradas.end());
		}

		const int FileSystem::FolderNameLength = 128;
		const int FileSystem::FileNameLength = 256;
		const int FileSystem::PathLength = 16383;

		FileSystem::FileSystem(unsigned int maxNumMounts)
			: maxNumMounts(0)
		{
			this->maxNumMounts = Math::iClamp(maxNumMounts, 1, 10);

			this->listMounts.reserve(this->maxNumMounts);
		}

		FileSystem::~FileSystem()
		{
			for (auto& change : this->listWatchChange)
				FindCloseChangeNotification(change.changeHandle);
			this->listWatchChange.clear();

			this->listMounts.clear();
			this->maxNumMounts = 0;
		}

		void FileSystem::FindFiles(const HorseRadish::hChar * const baseFolderAndFilter, const bool returnFilesFullPath, std::function<void(const HorseRadish::IO::Path &filePath, const HorseRadish::hUInt64 &fileSize)> actionFileFound)
		{
			HANDLE handleFind;
			WIN32_FIND_DATA findData;
			HorseRadish::String filePath;
			HorseRadish::IO::Path basePath, fileFinalPath;

			if ((baseFolderAndFilter == nullptr) || (actionFileFound == nullptr))
				return;

			{
				wchar_t baseFolderAndFilterWChar[256];
				HorseRadish::UTF::ConvertUTF8To(baseFolderAndFilter, HorseRadish::UTF::Encoding::Windows, baseFolderAndFilterWChar, sizeof(baseFolderAndFilterWChar));

				handleFind = FindFirstFile(baseFolderAndFilterWChar, &findData);
				if (handleFind == INVALID_HANDLE_VALUE)
					return;
			}

			basePath.Set(baseFolderAndFilter);
			basePath.RemoveFile();

			do
			{
				if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
					continue;

				filePath.Set(HorseRadish::String::Encoding::Windows, findData.cFileName);

				HorseRadish::hUInt64 fileSize = (findData.nFileSizeHigh * (MAXDWORD + 1)) + findData.nFileSizeLow;

				if (returnFilesFullPath == true)
				{
					fileFinalPath.Set(basePath);
					fileFinalPath.Combine(filePath);
				}
				else
				{
					fileFinalPath.Set(filePath);
				}

				actionFileFound(fileFinalPath, fileSize);

			} while (FindNextFile(handleFind, &findData) != 0);


			FindClose(handleFind);
		}

		bool FileSystem::FileExists(const HorseRadish::hChar * const filePath)
		{
			DWORD fileAtributes;

			if ((filePath == nullptr) || (*filePath == '\0'))
				return false;

			{
				wchar_t filePathWChar[256];
				HorseRadish::UTF::ConvertUTF8To(filePath, HorseRadish::UTF::Encoding::Windows, filePathWChar, sizeof(filePathWChar));

				fileAtributes = GetFileAttributes(filePathWChar);
			}

			if (fileAtributes == INVALID_FILE_ATTRIBUTES)
				return false;

			if ((fileAtributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
				return false;

			return true;
		}

		bool FileSystem::MountPath(const HorseRadish::IO::Path &baseFolder, const HorseRadish::hChar * const mountPoint)
		{
			if (this->listMounts.size() >= this->maxNumMounts)
				return false;

			this->listMounts.push_back(std::unique_ptr<MountDataPath>(new MountDataPath(baseFolder, mountPoint)));
			return true;
		}

		bool FileSystem::MountZip(const HorseRadish::IO::Path &zipPath, const HorseRadish::hChar * const mountPoint, int * const numFilesZip)
		{
			if (this->listMounts.size() >= this->maxNumMounts)
				return false;

			this->listMounts.push_back(std::unique_ptr<MountDataZip>(new MountDataZip(zipPath, mountPoint)));

			if (numFilesZip != nullptr)
				*numFilesZip = static_cast<MountDataZip*>(this->listMounts[this->listMounts.size() - 1].get())->GetNumberFiles();

			return true;
		}

		Streams::Stream* FileSystem::FileRead(const char * const filePath)
		{
			if ((filePath == nullptr) || (*filePath == '\0'))
				return nullptr;

			for (auto& curMount : this->listMounts)
			{
				if ((curMount->GetMountType() == FileSystem::MountTypePath) && (curMount->FileExists(reinterpret_cast<const HorseRadish::hChar*>(filePath)) == false))
					continue;

				auto fileStream = curMount->FileRead(reinterpret_cast<const HorseRadish::hChar*>(filePath));
				if (fileStream != nullptr)
					return fileStream;
			}

			return nullptr;
		}

		Streams::Stream* FileSystem::FileRead(const char * const filePath, const FileSystem::MountType mountType)
		{
			if ((filePath == nullptr) || (*filePath == '\0'))
				return nullptr;

			for (auto& curMount : this->listMounts)
			{
				if (curMount->GetMountType() != mountType)
					continue;

				if ((curMount->GetMountType() == FileSystem::MountTypePath) && (curMount->FileExists(reinterpret_cast<const HorseRadish::hChar*>(filePath)) == false))
					continue;

				auto fileStream = curMount->FileRead(reinterpret_cast<const HorseRadish::hChar*>(filePath));
				if (fileStream != nullptr)
					return fileStream;
			}

			return nullptr;
		}

		int FileSystem::WatchChangeCreate(const HorseRadish::hChar * const baseFolder, bool includeSubFolders, const FileSystem::ChangeType changeType)
		{
			HANDLE handleChange;

			if ((baseFolder == nullptr) || (changeType == 0))
				return -1;

			{
				wchar_t baseFolderWChar[256];
				HorseRadish::UTF::ConvertUTF8To(baseFolder, HorseRadish::UTF::Encoding::Windows, baseFolderWChar, sizeof(baseFolderWChar));

				DWORD changeFlags = 0;
				if (changeType & FileName)
					changeFlags = FILE_NOTIFY_CHANGE_FILE_NAME;
				if (changeType & FileSize)
					changeFlags = FILE_NOTIFY_CHANGE_SIZE;
				if (changeType & FileLastWrite)
					changeFlags = FILE_NOTIFY_CHANGE_LAST_WRITE;

				handleChange = FindFirstChangeNotification(baseFolderWChar, includeSubFolders ? TRUE : FALSE, changeFlags);
				if (handleChange == INVALID_HANDLE_VALUE)
					return 0;
			}

			auto changeID = this->listWatchChange.size() + 13;
			this->listWatchChange.push_back(WatchChangeData(changeID, handleChange));

			return changeID;
		}

		void FileSystem::WatchChangeDelete(const int watchChangeID)
		{
			auto changeIndex = std::find_if(this->listWatchChange.begin(), this->listWatchChange.end(), [&watchChangeID](const WatchChangeData &data){ return data.changeID == watchChangeID; });
			if (changeIndex == this->listWatchChange.end())
				return;

			FindCloseChangeNotification(changeIndex->changeHandle);
			this->listWatchChange.erase(changeIndex);
		}

		bool FileSystem::WatchChanged(const int watchChangeID)
		{
			auto changeIndex = std::find_if(this->listWatchChange.begin(), this->listWatchChange.end(), [&watchChangeID](const WatchChangeData &data){ return data.changeID == watchChangeID; });
			if (changeIndex == this->listWatchChange.end())
				return false;

			auto didChange = (WaitForSingleObject(changeIndex->changeHandle, 0) == WAIT_OBJECT_0);
			if (didChange)
				FindNextChangeNotification(changeIndex->changeHandle);

			return didChange;
		}

	} //IO
} //HorseRadish