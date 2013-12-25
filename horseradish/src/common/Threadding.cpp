#include "Threadding.hpp"

namespace HorseRadish
{

namespace Threadding
{

//§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§ Classe Lock		§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§§
Lock::Lock()
{
	//tenho de iniciar o objecto
	InitializeCriticalSection(&this->osObject);
}

Lock::~Lock()
{
	//removo o objecto
	DeleteCriticalSection(&this->osObject);
}

//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§ Classe LockAccess		§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
LockAccess::LockAccess(Lock &lockTarget)
{
	//guardo-o para o poder libertar mais tarde
	this->lockTarget = &lockTarget;

	//peço o lock
	EnterCriticalSection(&this->lockTarget->osObject);
}

LockAccess::~LockAccess()
{
	//liberto o lock
	LeaveCriticalSection(&this->lockTarget->osObject);
}

}//namespace Threadding
}//namespace HorseRadish