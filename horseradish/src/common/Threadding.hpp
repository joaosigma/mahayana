#pragma once
#ifndef __HTHREADDING__
#define __HTHREADDING__

#include "Platform.hpp"
#include "Types.hpp"
#include <windows.h>

namespace HorseRadish
{

namespace Threadding
{

//§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§ Classe Lock		§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§§
class Lock
{
	CRITICAL_SECTION osObject;

	friend class LockAccess;
public:
	Lock();
	~Lock();
};

//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§ Classe LockAccess		§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
class LockAccess
{
	Lock *lockTarget;

public:
	LockAccess(Lock &lockTarget);
	~LockAccess();
};

}//namespace Threadding
}//namespace HorseRadish

#endif