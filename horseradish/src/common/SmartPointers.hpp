#pragma once
#ifndef __HSMART_POINTERS__
#define __HSMART_POINTERS__

#include <assert.h>

#include "Types.hpp"

namespace HorseRadish
{

namespace Memory
{

//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§ Classe ScopedPtr		§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
template<class T>
class ScopedPtr 
{
    T *protectedPtr;

	//isto impede cópias
    ScopedPtr(ScopedPtr const &);
    ScopedPtr& operator=(ScopedPtr const &);

	//impede comparações directas
    void operator==(ScopedPtr const &) const;
    void operator!=(ScopedPtr const &) const;

public:

    explicit ScopedPtr(T *p = nullptr): protectedPtr(p)
    {    }

    ~ScopedPtr()
    {
        delete this->protectedPtr;
    }

    T& operator*() const
    {
        assert(this->protectedPtr != nullptr);
        return *this->protectedPtr;
    }

    T* operator->() const
    {
		assert(this->protectedPtr != nullptr);
        return this->protectedPtr;
    }

    T* Get() const
    {
        return this->protectedPtr;
    }

    void Swap(ScopedPtr &otherScopedPtr)
    {
        T* tmpPtr = otherScopedPtr.protectedPtr;
        otherScopedPtr.protectedPtr = this->protectedPtr;
        this->protectedPtr = tmpPtr;
    }
};

//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§ Classe ScopedArray		§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
template<class T>
class ScopedArray 
{
    T *protectedArray;

	//isto impede cópias
    ScopedArray(ScopedArray const &);
    ScopedArray& operator=(ScopedArray const &);

	//impede comparações directas
    void operator==(ScopedArray const &) const;
    void operator!=(ScopedArray const &) const;

public:

    explicit ScopedArray(T *p = nullptr): protectedArray(p)
    {    }

    ~ScopedArray()
    {
        delete[] this->protectedArray;
    }

    T& operator[](const hPrtDiff &index) const
    {
        assert(this->protectedArray != nullptr);
        assert(index >= 0);
        return this->protectedArray[index];
    }

    T* Get() const
    {
        return this->protectedArray;
    }

    void Swap(ScopedArray &otherScopedArray)
    {
        T* tmpArray = otherScopedArray.protectedArray;
        otherScopedArray.protectedArray = this->protectedArray;
        this->protectedArray = tmpArray;
    }
};

}//namespace Memory
}//namespace HorseRadish

#endif