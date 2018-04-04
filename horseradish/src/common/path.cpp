#include "path.hpp"

#include "stringUtils.hpp"
#include "../platform/platform.hpp"

namespace hr::io
{
	void Path::cleanPath()
	{
		if (hr::platform::Platform::DirectorySeparatorChar != '/')
			hr::StringUtils::replace(mPath, '/', hr::platform::Platform::DirectorySeparatorChar);
				
		hr::StringUtils::replace(mPath, "//", hr::StringUtils::conv2UTF8(hr::platform::Platform::DirectorySeparatorChar));
		hr::StringUtils::trim(mPath);
	}

	Path& Path::operator+=(const Path& path)
	{
		combine(path);
		return *this;
	}

	Path& Path::operator+=(std::string_view path)
	{
		combine(path);
		return *this;
	}

	void Path::clear()
	{
		mPath.clear();
	}

	void Path::set(std::string_view path)
	{
		mPath = path;
		cleanPath();
	}

	void Path::set(const Path &path1, const Path &path2)
	{
		mPath = path1.mPath;
		combine(path2);
	}

	void Path::set(const Path::KnownPath &knownPath)
	{
		if (knownPath == Path::KnownPath::SystemFolder)
		{
			std::string path;
			hr::platform::Platform::systemInfo(hr::platform::Platform::SystemInfo::SystemFolder, path);

			mPath = path;
			return;
		}

		if (knownPath == Path::KnownPath::CurrentFolder)
		{
			std::string path;
			hr::platform::Platform::systemInfo(hr::platform::Platform::SystemInfo::CurrentFolder, path);

			mPath = path;
			return;
		}

		mPath.clear();
	}

	void Path::combine(const Path &pathToAppend)
	{
		if (pathToAppend.isEmpty())
			return;

		if (!mPath.empty())
		{
			auto separator = hr::StringUtils::conv2UTF8(hr::platform::Platform::DirectorySeparatorChar);

			auto pos = mPath.find_last_of(separator);
			if ((pos != std::string::npos) && (pos != (mPath.size() - separator.size())))
				mPath += separator;
		}

		mPath += pathToAppend.mPath;
		hr::StringUtils::replace(mPath, "//", hr::StringUtils::conv2UTF8(hr::platform::Platform::DirectorySeparatorChar));
	}

	void Path::combine(std::string_view pathToAppend)
	{
		if (pathToAppend.empty())
			return;

		if (!mPath.empty())
		{
			auto separator = hr::StringUtils::conv2UTF8(hr::platform::Platform::DirectorySeparatorChar);

			auto pos = mPath.find_last_of(separator);
			if ((pos != std::string::npos) && (pos != (mPath.size() - separator.size())))
				mPath += separator;
		}		

		mPath += pathToAppend;

		cleanPath();
	}

	void Path::removeLastComponent()
	{
		removeComponents(1);
	}

	void Path::removeComponents(size_t numComponents)
	{
		if (numComponents <= 0)
			return;

		auto reversed = hr::StringUtils::reverseCopy(mPath);

		size_t charPos = 0, componentsRemoved = 0;
		for (const auto& curChar : hr::StringUtils::utf8Wrapper(reversed))
		{
			if (curChar == hr::platform::Platform::DirectorySeparatorChar)
			{
				componentsRemoved++;
				if (componentsRemoved >= numComponents)
					break;
			}

			charPos++;
		}

		std::string finalPath(mPath, 0, reversed.size() - charPos - 1);
		set(finalPath);
	}

	void Path::removeFile()
	{
		auto reversed = hr::StringUtils::reverseCopy(mPath);

		size_t charPos = 0;
		for (const auto& curChar : hr::StringUtils::utf8Wrapper(reversed))
		{
			if (curChar == hr::platform::Platform::DirectorySeparatorChar)
				break;
			charPos++;
		}
				
		std::string finalPath(mPath, 0, reversed.size() - charPos - 1);
		set(finalPath);
	}
}