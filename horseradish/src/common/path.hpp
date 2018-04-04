#pragma once

#include <string>

namespace hr::io
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
		Path() = default;

		explicit Path(std::string_view path)
			: mPath(path)
		{
			cleanPath();
		}

		explicit Path(const KnownPath &knownPath)
		{
			set(knownPath);
		}

		Path& operator+=(const Path& path);
		Path& operator+=(std::string_view path);

		bool isEmpty() const
		{
			return mPath.empty();
		}

		const std::string& str() const
		{
			return mPath;
		}

		void clear();

		void set(std::string_view path);
		void set(const Path &path1, const Path &path2);
		void set(const KnownPath &knownPath);

		void combine(const Path &pathToAppend);
		void combine(std::string_view pathToAppend);

		void removeLastComponent();
		void removeComponents(size_t numComponents = 0);

		void removeFile();
	};
}
