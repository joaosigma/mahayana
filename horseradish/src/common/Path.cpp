#include "Path.hpp"
#include "Platform.hpp"

namespace HorseRadish
{
	namespace IO
	{
		void Path::validadePath()
		{
			if (HorseRadish::Platform::DirectorySeparatorChar != '/')
				HorseRadish::String::Replace('/', HorseRadish::Platform::DirectorySeparatorChar);

			HorseRadish::String::RemoveDoubles(HorseRadish::Platform::DirectorySeparatorChar);

			HorseRadish::String::Trim(' ');
		}

		Path::Path()
		{
			HorseRadish::String::SetEmpty();
		}

		Path::Path(const HorseRadish::hChar * const path)
		{
			this->Set(path);
		}

		Path::Path(const char * const path)
		{
			this->Set((const HorseRadish::hChar *)path);
		}

		Path::Path(const HorseRadish::String &path)
		{
			this->Set((const HorseRadish::hChar *)path.GetData());
		}

		Path::Path(const KnownPath &knownPath)
		{
			this->Set(knownPath);
		}

		Path::~Path()
		{
			HorseRadish::String::SetEmpty();
		}

		void Path::Clear()
		{
			HorseRadish::String::SetEmpty();
		}

		void Path::Set(const HorseRadish::hChar * const path)
		{
			HorseRadish::String::SetEmpty();

			if ((path != nullptr) && (*path != '\0'))
				HorseRadish::String::Set(path);

			validadePath();
		}

		void Path::Set(const HorseRadish::String &path)
		{
			HorseRadish::String::Set(path);

			validadePath();
		}

		void Path::Set(const Path &path)
		{
			HorseRadish::String::Set(path);
		}

		void Path::Set(const Path &path1, const Path &path2)
		{
			HorseRadish::String::Set(path1);
			this->Combine(path2);
		}

		void Path::Set(const Path::KnownPath &knownPath)
		{
			if (knownPath == Path::KnownPath::SystemFolder)
			{
				HorseRadish::Platform::GetSystemInfo(HorseRadish::Platform::SystemInfo::SystemFolder, *this);
				return;
			}

			if (knownPath == Path::KnownPath::CurrentFolder)
			{
				HorseRadish::Platform::GetSystemInfo(HorseRadish::Platform::SystemInfo::CurrentFolder, *this);
				return;
			}

			HorseRadish::String::SetEmpty();
		}

		void Path::Combine(const Path &pathToAppend)
		{
			if (pathToAppend.IsEmpty() == true)
				return;

			if (HorseRadish::String::IsEmpty() == true)
			{
				HorseRadish::String::Set(pathToAppend);
				return;
			}

			if (HorseRadish::String::EndsWith(HorseRadish::Platform::DirectorySeparatorChar) == false)
				HorseRadish::String::operator+=(HorseRadish::Platform::DirectorySeparatorChar);

			HorseRadish::String::operator+=(pathToAppend);

			HorseRadish::String::RemoveDoubles(HorseRadish::Platform::DirectorySeparatorChar);
		}

		void Path::Combine(const HorseRadish::hChar * const pathToAppend)
		{
			if ((pathToAppend == nullptr) || (*pathToAppend == '\0'))
				return;

			if (HorseRadish::String::IsEmpty(true) == true)
			{
				HorseRadish::String::Set(pathToAppend);
				validadePath();
				return;
			}

			if (HorseRadish::String::EndsWith(HorseRadish::Platform::DirectorySeparatorChar) == false)
				HorseRadish::String::operator+=(HorseRadish::Platform::DirectorySeparatorChar);

			HorseRadish::String::operator+=(pathToAppend);

			validadePath();
		}

		void Path::Combine(const HorseRadish::String &pathToAppend)
		{
			if (HorseRadish::String::IsEmpty(true) == true)
			{
				HorseRadish::String::Set(pathToAppend);
				validadePath();
				return;
			}

			if (HorseRadish::String::EndsWith(HorseRadish::Platform::DirectorySeparatorChar) == false)
				HorseRadish::String::operator+=(HorseRadish::Platform::DirectorySeparatorChar);

			HorseRadish::String::operator+=(pathToAppend);

			validadePath();
		}

		void Path::RemoveLastComponent()
		{
			this->RemoveComponents(1);
		}

		void Path::RemoveComponents(const HorseRadish::hUInt8 &numComponents)
		{
			if (numComponents <= 0)
				return;

			auto iterator = HorseRadish::String::Iterator(*this);
			iterator.Last();

			auto componentsRemoved = 0;
			for (; iterator.IsFirst() == false; iterator--)
			{
				if (*iterator != HorseRadish::Platform::DirectorySeparatorChar)
					continue;

				componentsRemoved++;
				if (componentsRemoved >= numComponents)
					break;
			}

			HorseRadish::String::CloseAt(iterator.GetCaracterPosition());
		}

		void Path::RemoveFile()
		{
			auto iterator = HorseRadish::String::Iterator(*this);
			iterator.Last();

			while ((*iterator != HorseRadish::Platform::DirectorySeparatorChar) && (iterator.IsFirst() == false))
				iterator--;

			HorseRadish::String::CloseAt(iterator.GetCaracterPosition());
		}

		Path& Path::operator+=(const Path& path)
		{
			this->Combine(path);
			return *this;
		}

		Path& Path::operator+=(const HorseRadish::hChar * const path)
		{
			this->Combine(path);
			return *this;
		}

	} //IO
} //HorseRadish