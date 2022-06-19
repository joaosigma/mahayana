#pragma once

#include <cassert>
#include <stdint.h>

namespace hr
{
    template <typename T, int AlignedTo>
    class TaggedPointer final
    {
    private:
        static_assert((AlignedTo != 0) && (((AlignedTo & (AlignedTo - 1)) == 0)), "Alignment parameter must be power of two");

        // for 8 byte alignment TagMask = AlignedTo - 1 = 8 - 1 = 7 = 0b111
        // i.e. the lowest three bits are set, which is where the tag is stored
        static const intptr_t TagMask = AlignedTo - 1;

        // PointerMask is the exact contrary: 0b...11111000
        // i.e. all bits apart from the three lowest are set, which is where the pointer is stored
        static const intptr_t PointerMask = ~TagMask;

        // save us some reinterpret_casts with a union
        union
        {
            T* asPointer;
            intptr_t asBits;
        };

    public:
        inline explicit TaggedPointer(T* pointer = nullptr, int tag = 0)
        {
            set(pointer, tag);
        }

        inline void set(T* pointer, int tag = 0)
        {
            assert((reinterpret_cast<intptr_t>(pointer) & TagMask) == 0); // make sure that the pointer really is aligned
            assert((tag & PointerMask) == 0); // make sure that the tag isn't too large

            asPointer = pointer;
            asBits |= tag;
        }

        inline T* pointer() const noexcept
        {
            return reinterpret_cast<T*>(asBits & PointerMask);
        }

        inline int tag() const noexcept
        {
            return (asBits & TagMask);
        }
    };
}