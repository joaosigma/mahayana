#pragma once
#ifndef __HPATH__
#define __HPATH__

#include "String.hpp"

namespace HorseRadish
{
namespace IO
{

class Path : private HorseRadish::String
{
public:
	enum KnownPath { SystemFolder, CurrentFolder };

private:
	void validadePath();

public:
	Path();
	explicit Path(const HorseRadish::hChar * const path);
	explicit Path(const char * const path);
	explicit Path(const HorseRadish::String &path);
	explicit Path(const KnownPath &knownPath);
	~Path();

	bool IsEmpty() const { return HorseRadish::String::IsEmpty(); }

	const char* GetData() const {return HorseRadish::String::GetData();}

	void Clear();
	void Set(const HorseRadish::hChar * const path);
	void Set(const HorseRadish::String &path);
	void Set(const Path &path);
	void Set(const Path &path1, const Path &path2);
	void Set(const KnownPath &knownPath);
	void Combine(const Path &pathToAppend);
	void Combine(const HorseRadish::hChar * const pathToAppend);
	void Combine(const HorseRadish::String &pathToAppend);
	void RemoveLastComponent();
	void RemoveComponents(const HorseRadish::hUInt8 &numComponents = 0);

	void RemoveFile();

	Path& operator+=(const Path& path);
	Path& operator+=(const HorseRadish::hChar * const path);

	operator const HorseRadish::hChar*() const { return ((const HorseRadish::hChar*)HorseRadish::String::GetData()); }
};

}//namespace IO
}//namespace HorseRadish

#endif