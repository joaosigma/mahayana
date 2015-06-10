#pragma once

#include <string>

namespace HorseRadish {	namespace IO {

class Path
{
public:
	enum class KnownPath { SystemFolder, CurrentFolder };

private:
	std::string mPath;

private:
	void cleanPath();

public:
	Path();
	explicit Path(const char* const path);
	explicit Path(const std::string& path);
	explicit Path(const KnownPath &knownPath);

	bool IsEmpty() const
	{
		return mPath.empty();
	}

	const std::string& str() const
	{
		return mPath;
	}

	void Clear();
	void Set(const std::string& path);
	void Set(const char* const path);
	void Set(const Path &path1, const Path &path2);
	void Set(const KnownPath &knownPath);
	void Combine(const Path &pathToAppend);
	void Combine(const char* const pathToAppend);
	void Combine(const std::string& pathToAppend);
	void RemoveLastComponent();
	void RemoveComponents(const unsigned int numComponents = 0);

	void RemoveFile();

	Path& operator+=(const Path& path);
	Path& operator+=(const char* const path);
	Path& operator+=(const std::string& path);
};

} }
