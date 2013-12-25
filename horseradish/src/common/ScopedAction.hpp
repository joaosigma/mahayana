#pragma once
#ifndef __HSCOPED_ACTION__
#define __HSCOPED_ACTION__

#include "Types.hpp"

#include <assert.h>
#include <functional>

namespace HorseRadish
{

//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§ Classe ScopedAction		§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
class ScopedAction
{
    std::function<void ()> funcCallback;

	//isto impede cópias
    ScopedAction(ScopedAction const &);
    ScopedAction& operator=(ScopedAction const &);

public:

    explicit ScopedAction(std::function<void ()> funcCallback = nullptr) : funcCallback(funcCallback)
    {    }

    ~ScopedAction()
    {
        if (funcCallback != nullptr)
			funcCallback();
    }
};

}//namespace HorseRadish

#endif