#include "path.hpp"
#include "platform.hpp"
#include "stringUtils.hpp"

namespace HorseRadish { namespace IO {

void Path::cleanPath()
{
	if (HorseRadish::Platform::DirectorySeparatorChar != '/')
		HorseRadish::StringUtils::replace(mPath, '/', HorseRadish::Platform::DirectorySeparatorChar);
				
	HorseRadish::StringUtils::replace(mPath, "//", HorseRadish::StringUtils::conv2UTF8(HorseRadish::Platform::DirectorySeparatorChar));
	HorseRadish::StringUtils::trim(mPath);
}

Path& Path::operator+=(const Path& path)
{
	combine(path);
	return *this;
}

Path& Path::operator+=(const char* const path)
{
	combine(path);
	return *this;
}

Path& Path::operator+=(const std::string& path)
{
	combine(path);
	return *this;
}

void Path::clear()
{
	mPath.clear();
}

void Path::set(const std::string& path)
{
	mPath = path;
	cleanPath();
}

void Path::set(const char* const path)
{
	mPath.clear();

	if (path != nullptr)
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
		HorseRadish::Platform::systemInfo(HorseRadish::Platform::SystemInfo::SystemFolder, path);

		mPath = path;
		return;
	}

	if (knownPath == Path::KnownPath::CurrentFolder)
	{
		std::string path;
		HorseRadish::Platform::systemInfo(HorseRadish::Platform::SystemInfo::CurrentFolder, path);

		mPath = path;
		return;
	}

	mPath.clear();
}

void Path::combine(const Path &pathToAppend)
{
	if (pathToAppend.isEmpty() == true)
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

void Path::combine(const char* const pathToAppend)
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

void Path::combine(const std::string& pathToAppend)
{
	combine(pathToAppend.c_str());
}

void Path::removeLastComponent()
{
	removeComponents(1);
}

void Path::removeComponents(size_t numComponents)
{
	if (numComponents <= 0)
		return;

	auto reversed = HorseRadish::StringUtils::reverseCopy(mPath);

	size_t charPos = 0, componentsRemoved = 0;
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
	set(finalPath);
}

void Path::removeFile()
{
	auto reversed = HorseRadish::StringUtils::reverseCopy(mPath);

	size_t charPos = 0;
	for (const auto& curChar : HorseRadish::StringUtils::utf8Wrapper(reversed))
	{
		if (curChar == HorseRadish::Platform::DirectorySeparatorChar)
			break;
		charPos++;
	}
				
	std::string finalPath(mPath, 0, reversed.size() - charPos - 1);
	set(finalPath);
}

}}