#pragma once

#include "stream.hpp"
#include "types.hpp"

#include "libs/zlib/zlib.h"
#include "libs/zlib/minizip/unzip.h"

#include <vector>
#include <memory>
#include <filesystem>
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
			std::string m_mountPoint;

		public:
			MountData(std::string mountPoint) noexcept
			  : m_mountPoint{std::move(mountPoint)}
			{ }
			virtual ~MountData() = default;

			virtual FileSystem::MountType mountType() const = 0;
			virtual void filesEnumerate() = 0;
			virtual std::unique_ptr<streams::Stream> fileRead(const std::filesystem::path& filePath) = 0;
			virtual bool fileExists(const std::filesystem::path &filePath) = 0;
		};

		class MountDataPath : public MountData
		{
			std::filesystem::path m_baseFolder;

		public:
			MountDataPath(std::filesystem::path baseFolder, std::string mountPoint) noexcept
			  : MountData(std::move(mountPoint)), m_baseFolder{std::move(baseFolder)}
			{ }

			FileSystem::MountType mountType() const override
			{
				return FileSystem::MountType::Path;
			}

			void filesEnumerate() override
			{ }

			std::unique_ptr<streams::Stream> fileRead(const std::filesystem::path &filePath) override;

			bool fileExists(const std::filesystem::path &filePath) override;
		};

		class MountDataZip : public MountData
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

			void filesEnumerate() override;
			std::unique_ptr<streams::Stream> fileRead(const std::filesystem::path &filePath) override;
			bool fileExists(const std::filesystem::path &filePath) override;
		};

		size_t m_maxNumMounts = 0;
		std::vector<std::unique_ptr<MountData>> m_listMounts;
		std::vector<WatchChangeData> m_listWatchChange;

	public:
		FileSystem(size_t maxNumMounts);
		~FileSystem();

		static void findFiles(const std::filesystem::path &baseFolderAndFilter, const bool returnFilesFullPath, const std::function<void(const std::filesystem::path &filePath, const uint64_t &fileSize)> &actionFileFound);
		static bool fileExists(const std::filesystem::path &path);

		bool mountPath(const std::filesystem::path &baseFolder, std::string mountPoint);
		bool mountZip(const std::filesystem::path &zipPath, std::string mountPoint, size_t * const numFilesZip = nullptr);

		std::unique_ptr<streams::Stream> fileRead(std::string_view filePath);
		std::unique_ptr<streams::Stream> fileRead(std::string_view filePath, MountType mountType);

		std::string readFileAsString(std::string_view filePath);

		int watchChangeCreate(const std::filesystem::path &baseFolder, bool includeSubFolders, ChangeType changeType);
		void watchChangeDelete(const int watchChangeID);
		bool watchChanged(const int watchChangeID);
	};
}
