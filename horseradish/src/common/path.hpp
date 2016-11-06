#pragma once

#include <string>

namespace hr { namespace io
{
	class Path
	{
	public:
		enum class KnownPath { SystemFolder, CurrentFolder };

	private:
		std::string mPath;

	private:
		void cleanPath();

	public:
		Path()
		{ }

		explicit Path(const char* const path)
			: mPath(path)
		{
			cleanPath();
		}

		explicit Path(const std::string& path)
			: mPath(path)
		{
			cleanPath();
		}

		explicit Path(const KnownPath &knownPath)
		{
			set(knownPath);
		}

		Path& operator+=(const Path& path);
		Path& operator+=(const char* const path);
		Path& operator+=(const std::string& path);

		bool isEmpty() const
		{
			return mPath.empty();
		}

		const std::string& str() const
		{
			return mPath;
		}

		void clear();

		void set(const std::string& path);
		void set(const char* const path);
		void set(const Path &path1, const Path &path2);
		void set(const KnownPath &knownPath);

		void combine(const Path &pathToAppend);
		void combine(const char* const pathToAppend);
		void combine(const std::string& pathToAppend);

		void removeLastComponent();
		void removeComponents(size_t numComponents = 0);

		void removeFile();
	};
} }
