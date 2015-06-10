#include "Path.hpp"
#include "Platform.hpp"
#include "stringUtils.hpp"

namespace HorseRadish
{
	namespace IO
	{
		void Path::cleanPath()
		{
			if (HorseRadish::Platform::DirectorySeparatorChar != '/')
				HorseRadish::StringUtils::replace(mPath, '/', HorseRadish::Platform::DirectorySeparatorChar);
				
			HorseRadish::StringUtils::replace(mPath, "//", HorseRadish::StringUtils::conv2UTF8(HorseRadish::Platform::DirectorySeparatorChar));

			HorseRadish::StringUtils::trim(mPath);
		}

		Path::Path()
		{ }

		Path::Path(const char* const path)
			: mPath(path)
		{
			cleanPath();
		}

		Path::Path(const std::string& path)
			: mPath(path)
		{
			cleanPath();
		}

		Path::Path(const KnownPath &knownPath)
		{
			this->Set(knownPath);
		}

		void Path::Clear()
		{
			mPath.clear();
		}

		void Path::Set(const std::string& path)
		{
			mPath = path;
			cleanPath();
		}

		void Path::Set(const char* const path)
		{
			mPath.clear();

			if (path != nullptr)
				mPath = path;

			cleanPath();
		}

		void Path::Set(const Path &path1, const Path &path2)
		{
			mPath = path1.mPath;
			this->Combine(path2);
		}

		void Path::Set(const Path::KnownPath &knownPath)
		{
			if (knownPath == Path::KnownPath::SystemFolder)
			{
				std::string path;
				HorseRadish::Platform::GetSystemInfo(HorseRadish::Platform::SystemInfo::SystemFolder, path);

				mPath = path;
				return;
			}

			if (knownPath == Path::KnownPath::CurrentFolder)
			{
				std::string path;
				HorseRadish::Platform::GetSystemInfo(HorseRadish::Platform::SystemInfo::CurrentFolder, path);

				mPath = path;
				return;
			}

			mPath.clear();
		}

		void Path::Combine(const Path &pathToAppend)
		{
			if (pathToAppend.IsEmpty() == true)
				return;

			if (!mPath.empty())
			{
				auto separator = HorseRadish::StringUtils::conv2UTF8(HorseRadish::Platform::DirectorySeparatorChar);

				auto pos = mPath.find_last_of(separator);
				if ((pos != std::string::npos) && (pos != (mPath.size() - separator.size())))
					mPath += separator;
			}

			mPath += pathToAppend.mPath;
			HorseRadish::StringUtils::replace(mPath, "//", HorseRadish::StringUtils::conv2UTF8(HorseRadish::Platform::DirectorySeparatorChar));
		}

		void Path::Combine(const char* const pathToAppend)
		{
			if ((pathToAppend == nullptr) || (*pathToAppend == '\0'))
				return;

			if (!mPath.empty())
			{
				auto separator = HorseRadish::StringUtils::conv2UTF8(HorseRadish::Platform::DirectorySeparatorChar);

				auto pos = mPath.find_last_of(separator);
				if ((pos != std::string::npos) && (pos != (mPath.size() - separator.size())))
					mPath += separator;
			}		

			mPath += pathToAppend;

			cleanPath();
		}

		void Path::Combine(const std::string& pathToAppend)
		{
			this->Combine(pathToAppend.c_str());
		}

		void Path::RemoveLastComponent()
		{
			this->RemoveComponents(1);
		}

		void Path::RemoveComponents(const unsigned int numComponents)
		{
			if (numComponents <= 0)
				return;

			auto reversed = HorseRadish::StringUtils::reverseCopy(mPath);

			unsigned int charPos = 0, componentsRemoved = 0;
			for (const auto& curChar : HorseRadish::StringUtils::utf8Wrapper(reversed))
			{
				if (curChar == HorseRadish::Platform::DirectorySeparatorChar)
				{
					componentsRemoved++;
					if (componentsRemoved >= numComponents)
						break;
				}

				charPos++;
			}

			std::string finalPath(mPath, 0, reversed.size() - charPos - 1);
			this->Set(finalPath);
		}

		void Path::RemoveFile()
		{
			auto reversed = HorseRadish::StringUtils::reverseCopy(mPath);

			unsigned int charPos = 0;
			for (const auto& curChar : HorseRadish::StringUtils::utf8Wrapper(reversed))
			{
				if (curChar == HorseRadish::Platform::DirectorySeparatorChar)
					break;
				charPos++;
			}
				
			std::string finalPath(mPath, 0, reversed.size() - charPos - 1);
			this->Set(finalPath);
		}

		Path& Path::operator+=(const Path& path)
		{
			this->Combine(path);
			return *this;
		}

		Path& Path::operator+=(const char* const path)
		{
			this->Combine(path);
			return *this;
		}

		Path& Path::operator+=(const std::string& path)
		{
			this->Combine(path);
			return *this;
		}

	} //IO
} //HorseRadish