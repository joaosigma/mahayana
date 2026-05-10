module;

#include <minizip/unzip.h>

export module core:fileSystem;

import std;

import :stream;

namespace hr::io
{
    export
    class FileSystem
    {
    public:
        enum class MountType
        {
            Path,
            ZIP
        };
        enum ChangeType : size_t
        {
            FileName = (1 << 0),
            FileSize = (1 << 1),
            FileLastWrite = (1 << 2)
        };

        static constexpr int FolderNameLength = 128;
        static constexpr int FileNameLength = 256;
        static constexpr int PathLength = 16383;

    private:
        class WatchChangeData
        {
        public:
            int changeID = 0;
            void* changeHandle = nullptr;

            WatchChangeData() = default;

            WatchChangeData(int changeID, void* changeHandle)
              : changeID(changeID), changeHandle(changeHandle)
            {}
        };

        class MountData
        {
        protected:
            std::string m_mountPoint;

        public:
            MountData(std::string mountPoint) noexcept
              : m_mountPoint{std::move(mountPoint)}
            {}
            virtual ~MountData() = default;

            virtual FileSystem::MountType mountType() const = 0;
            virtual void filesEnumerate() const = 0;
            virtual std::unique_ptr<streams::Stream> fileRead(const std::filesystem::path& filePath) const = 0;
            virtual bool fileExists(const std::filesystem::path& filePath) const = 0;
        };

        class MountDataPath final: public MountData
        {
            std::filesystem::path m_baseFolder;

        public:
            MountDataPath(std::filesystem::path baseFolder, std::string mountPoint) noexcept
              : MountData(std::move(mountPoint)), m_baseFolder{std::move(baseFolder)}
            {}

            FileSystem::MountType mountType() const override
            {
                return FileSystem::MountType::Path;
            }

            void filesEnumerate() const override
            {}

            std::unique_ptr<streams::Stream> fileRead(const std::filesystem::path& filePath) const override;

            bool fileExists(const std::filesystem::path& filePath) const override;
        };

        class MountDataZip final: public MountData
        {
        private:
            struct ZipEntry
            {
                unz_file_pos filePos;
                size_t fileSize;
            };
            unzFile m_zipFile{nullptr};
            std::filesystem::path m_zipPath;
            std::unordered_map<std::filesystem::path, ZipEntry> m_fileEntries;

        public:
            MountDataZip(std::filesystem::path zipPath, std::string mountPoint);
            ~MountDataZip();

            FileSystem::MountType mountType() const;
            size_t numberFiles() const;

            void filesEnumerate() const override;
            std::unique_ptr<streams::Stream> fileRead(const std::filesystem::path& filePath) const override;
            bool fileExists(const std::filesystem::path& filePath) const override;
        };

        size_t m_maxNumMounts = 0;
        std::vector<std::unique_ptr<MountData>> m_listMounts;
        std::vector<WatchChangeData> m_listWatchChange;

    public:
        struct FindFileData
        {
            std::filesystem::path path;
            std::uint64_t size;
        };

        static std::generator<FindFileData> findFiles(std::filesystem::path baseFolder, std::filesystem::path filter, bool returnFullPath);
        static bool fileExists(const std::filesystem::path& path);

    public:
        FileSystem(size_t maxNumMounts)
          : m_maxNumMounts(std::clamp(maxNumMounts, 1uz, 10uz))
        {}

        ~FileSystem() noexcept;

        bool mountPath(const std::filesystem::path& baseFolder, std::string mountPoint);
        bool mountZip(const std::filesystem::path& zipPath, std::string mountPoint, size_t* const numFilesZip = nullptr);

        std::unique_ptr<streams::Stream> fileRead(std::string_view filePath) const;
        std::unique_ptr<streams::Stream> fileRead(std::string_view filePath, MountType mountType) const;

        std::string readFileAsString(std::string_view filePath) const;

        int watchChangeCreate(const std::filesystem::path& baseFolder, bool includeSubFolders, ChangeType changeType);
        void watchChangeDelete(const int watchChangeID);
        bool watchChanged(const int watchChangeID);
    };
}
