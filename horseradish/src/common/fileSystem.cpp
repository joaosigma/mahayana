#include "fileSystem.hpp"

#include "scopedAction.hpp"
#include "stringUtils.hpp"

#include <algorithm>
#include <cstddef>
#include <format>

#include <windows.h>

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
            auto fileStream = new hr::streams::FileStream(std::filesystem::path{filename, std::filesystem::path::generic_format}, true, false);

            return ((voidpf)fileStream);
        }

        uLong ZCALLBACK zread(voidpf, voidpf stream, void* buf, uLong size)
        {
            auto fileStream = reinterpret_cast<hr::streams::FileStream*>(stream);

            return static_cast<uLong>(fileStream->read({reinterpret_cast<std::byte*>(buf), static_cast<size_t>(size)}));
        }

        long ZCALLBACK ztell(voidpf, voidpf stream)
        {
            auto fileStream = reinterpret_cast<hr::streams::FileStream*>(stream);

            return static_cast<long>(fileStream->position());
        }

        long ZCALLBACK zseek(voidpf, voidpf stream, uLong offset, int origin)
        {
            auto fileStream = reinterpret_cast<hr::streams::FileStream*>(stream);

            switch (origin)
            {
                case ZLIB_FILEFUNC_SEEK_CUR:
                    fileStream->seek(hr::streams::Stream::SeekOrigin::Current, offset);
                    break;
                case ZLIB_FILEFUNC_SEEK_END:
                    fileStream->seek(hr::streams::Stream::SeekOrigin::End, offset);
                    break;
                case ZLIB_FILEFUNC_SEEK_SET:
                    fileStream->seek(hr::streams::Stream::SeekOrigin::Begin, offset);
                    break;
                default:
                    return 1;
            }

            return 0;
        }

        int ZCALLBACK zclose(voidpf, voidpf stream)
        {
            auto fileStream = reinterpret_cast<hr::streams::FileStream*>(stream);

            fileStream->close();
            delete fileStream;

            return 0;
        }

        void overloadZLibIO(zlib_filefunc_def* const zlibFileFunc)
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

    std::unique_ptr<streams::Stream> FileSystem::MountDataPath::fileRead(const std::filesystem::path& filePath) const
    {
        auto pathFinal = m_baseFolder;
        pathFinal /= filePath;

        auto fileStream = std::unique_ptr<streams::FileStream>(new streams::FileStream(pathFinal, true, false));

        if (!fileStream->isValid())
            return nullptr;

        return std::move(fileStream);
    }

    bool FileSystem::MountDataPath::fileExists(const std::filesystem::path& filePath) const
    {
        auto pathFinal = m_baseFolder;
        pathFinal /= filePath;

        return FileSystem::fileExists(pathFinal);
    }

    FileSystem::MountDataZip::MountDataZip(std::filesystem::path zipPath, std::string mountPoint)
      : MountData(std::move(mountPoint)), m_zipPath{std::move(zipPath)}
    {
        zlib_filefunc_def zlibAPI;
        char zipFileCurFileName[1024];

        overloadZLibIO(&zlibAPI);

        m_zipFile = unzOpen2(reinterpret_cast<const char*>(m_zipPath.u8string().c_str()), &zlibAPI);
        if (!m_zipFile)
            return;

        unsigned int numEntries = 0;
        {
            unz_global_info zipInfo;

            unzGetGlobalInfo(m_zipFile, &zipInfo);
            numEntries = zipInfo.number_entry;
        }

        m_fileEntries.rehash(numEntries);

        unzGoToFirstFile(m_zipFile);
        for (unsigned int entryIndex = 0; entryIndex < numEntries; entryIndex++)
        {
            unz_file_info zipFileCurFileInfo;
            unzGetCurrentFileInfo(m_zipFile, &zipFileCurFileInfo, zipFileCurFileName, sizeof(zipFileCurFileName), nullptr, 0, nullptr, 0);

            if (zipFileCurFileInfo.uncompressed_size > 0) // ignore folders
            {
                ZipEntry zipEntry;
                zipEntry.fileSize = zipFileCurFileInfo.uncompressed_size;
                unzGetFilePos(m_zipFile, &zipEntry.filePos);

                m_fileEntries[std::string(zipFileCurFileName, zipFileCurFileInfo.size_filename)] = zipEntry;
            }

            unzGoToNextFile(m_zipFile);
        }
    }

    FileSystem::MountDataZip::~MountDataZip()
    {
        m_fileEntries.clear();
        m_zipPath.clear();

        if (m_zipFile)
            unzClose(m_zipFile);
        m_zipFile = nullptr;
    }

    FileSystem::MountType FileSystem::MountDataZip::mountType() const
    {
        return FileSystem::MountType::ZIP;
    }

    size_t FileSystem::MountDataZip::numberFiles() const
    {
        return m_fileEntries.size();
    }

    void FileSystem::MountDataZip::filesEnumerate() const
    {}

    std::unique_ptr<streams::Stream> FileSystem::MountDataZip::fileRead(const std::filesystem::path& filePath) const
    {
        if (filePath.empty())
            return nullptr;

        auto it = m_fileEntries.find(filePath);
        if (it == m_fileEntries.end())
            return nullptr;

        streams::MemoryStream memStream;
        memStream.truncate(it->second.fileSize);

        unz_file_pos file_pos = it->second.filePos;
        unzGoToFilePos(m_zipFile, &file_pos);

        unzOpenCurrentFile(m_zipFile);

        unzReadCurrentFile(m_zipFile, memStream.data().data(), static_cast<unsigned int>(it->second.fileSize));

        unzCloseCurrentFile(m_zipFile);

        return std::make_unique<streams::MemoryStream>(std::move(memStream));
    }

    bool FileSystem::MountDataZip::fileExists(const std::filesystem::path& filePath) const
    {
        if (filePath.empty())
            return false;

        return m_fileEntries.contains(filePath);
    }

    std::generator<FileSystem::FindFileData> FileSystem::findFiles(std::filesystem::path baseFolder, std::filesystem::path filter, bool returnFullPath)
    {
        if (baseFolder.empty() || filter.empty())
            co_return;

        WIN32_FIND_DATA findData;
        HANDLE handleFind = FindFirstFile((baseFolder / filter).c_str(), &findData);
        if (handleFind == INVALID_HANDLE_VALUE)
            co_return;

        ScopedAction _([&]() { FindClose(handleFind); });

        do
        {
            if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                continue;

            auto filePath = std::filesystem::path{hr::StringUtils::conv2UTF8(findData.cFileName)};
            if (returnFullPath)
                filePath = baseFolder / filePath;

            ULARGE_INTEGER ul;
            ul.HighPart = findData.nFileSizeHigh;
            ul.LowPart = findData.nFileSizeLow;

            co_yield FileSystem::FindFileData{std::move(filePath), static_cast<uint64_t>(ul.QuadPart)};

        } while (FindNextFile(handleFind, &findData) != 0);
    }

    bool FileSystem::fileExists(const std::filesystem::path& path)
    {
        auto status = std::filesystem::status(path);
        return std::filesystem::is_regular_file(status) && std::filesystem::exists(status);
    }

    FileSystem::~FileSystem() noexcept
    {
        for (auto& change : m_listWatchChange)
            FindCloseChangeNotification(change.changeHandle);
        m_listWatchChange.clear();

        m_listMounts.clear();
        m_maxNumMounts = 0;
    }

    bool FileSystem::mountPath(const std::filesystem::path& baseFolder, std::string mountPoint)
    {
        if (m_listMounts.size() >= m_maxNumMounts)
            return false;

        m_listMounts.push_back(std::unique_ptr<MountDataPath>(new MountDataPath(baseFolder, std::move(mountPoint))));
        return true;
    }

    bool FileSystem::mountZip(const std::filesystem::path& zipPath, std::string mountPoint, size_t* const numFilesZip)
    {
        if (m_listMounts.size() >= m_maxNumMounts)
            return false;

        auto mPoint = std::make_unique<MountDataZip>(zipPath, std::move(mountPoint));

        if (numFilesZip)
            *numFilesZip = mPoint->numberFiles();

        m_listMounts.push_back(std::move(mPoint));

        return true;
    }

    std::unique_ptr<streams::Stream> FileSystem::fileRead(std::string_view filePath)
    {
        if (filePath.empty())
            return nullptr;

        for (auto& curMount : m_listMounts)
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

        for (auto& curMount : m_listMounts)
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

        std::string fileData;
        auto res = fileStream->cloneAllContent(
          [&fileData](size_t size) -> std::span<std::byte>
          {
              fileData.resize(size);
              return {reinterpret_cast<std::byte*>(fileData.data()), fileData.size()};
          });

        return res ? fileData : std::string{};
    }

    int FileSystem::watchChangeCreate(const std::filesystem::path& baseFolder, bool includeSubFolders, FileSystem::ChangeType changeType)
    {
        HANDLE handleChange;

        if (baseFolder.empty())
            return -1;

        {
            DWORD changeFlags = 0;
            if (changeType & FileName)
                changeFlags = FILE_NOTIFY_CHANGE_FILE_NAME;
            if (changeType & FileSize)
                changeFlags = FILE_NOTIFY_CHANGE_SIZE;
            if (changeType & FileLastWrite)
                changeFlags = FILE_NOTIFY_CHANGE_LAST_WRITE;

            handleChange = FindFirstChangeNotification(baseFolder.c_str(), includeSubFolders ? TRUE : FALSE, changeFlags);
            if (handleChange == INVALID_HANDLE_VALUE)
                return 0;
        }

        auto changeID = static_cast<int>(m_listWatchChange.size() + 13);
        m_listWatchChange.push_back(WatchChangeData(changeID, handleChange));

        return changeID;
    }

    void FileSystem::watchChangeDelete(const int watchChangeID)
    {
        auto it = std::find_if(m_listWatchChange.begin(), m_listWatchChange.end(), [&watchChangeID](const WatchChangeData& data) { return data.changeID == watchChangeID; });
        if (it == m_listWatchChange.end())
            return;

        FindCloseChangeNotification(it->changeHandle);
        m_listWatchChange.erase(it);
    }

    bool FileSystem::watchChanged(const int watchChangeID)
    {
        auto changeIndex =
          std::find_if(m_listWatchChange.begin(), m_listWatchChange.end(), [&watchChangeID](const WatchChangeData& data) { return data.changeID == watchChangeID; });
        if (changeIndex == m_listWatchChange.end())
            return false;

        auto didChange = (WaitForSingleObject(changeIndex->changeHandle, 0) == WAIT_OBJECT_0);
        if (didChange)
            FindNextChangeNotification(changeIndex->changeHandle);

        return didChange;
    }
}
