#pragma once

#include <cstdint>
#include <immintrin.h>
#include <type_traits>
#include <xmmintrin.h>

#define HRESTRICT __restrict
#define HFUNC_RESTRICT __declspec(restrict)

namespace hr::types
{
    namespace detail
    {
        void pack(const float* const in, int16_t* const out, size_t numValues);
        void pack(const float* const in, uint16_t* const out, size_t numValues);
        void unpack(const int16_t* const in, float* const out, size_t numValues);
        void unpack(const uint16_t* const in, float* const out, size_t numValues);
    }

    typedef signed char hChar;

    union hSplitUInt32
    {
        struct PacketBytes
        {
            uint8_t byte0;
            uint8_t byte1;
            uint8_t byte2;
            uint8_t byte3;
        } piecesBytes;

        struct PacketShorts
        {
            uint16_t short0;
            uint16_t short1;
        } piecesShort;

        uint32_t valueWord;

        hSplitUInt32(uint32_t valueWord)
          : valueWord(valueWord)
        {}
    };

    template<typename T>
    T packFloat(const float value)
    {
        static_assert(std::is_arithmetic_v<T>);

        T out;
        detail::pack(&value, &out, 1);

        return out;
    }

    template<typename T>
    float unpackFloat(const T value)
    {
        static_assert(std::is_arithmetic_v<T>);

        float out;
        detail::unpack(&value, &out, 1);

        return out;
    }

    template<typename T>
    void packFloat(const float* const in, T* const out, size_t numValues)
    {
        static_assert(std::is_arithmetic_v<T>);
        detail::pack(in, out, numValues);
    }

    template<typename T>
    void unpackFloat(const T* const in, float* const out, size_t numValues)
    {
        static_assert(std::is_arithmetic_v<T>);
        detail::unpack(in, out, numValues);
    }

#pragma warning(push)
#pragma warning(disable : 4324)

    typedef union
    {
        float f32[4];
        double d64[2];
        __int8 i8[16];
        __int16 i16[8];
        __int32 i32[4];
        __int64 i64[2];
        unsigned __int8 ui8[16];
        unsigned __int16 ui16[8];
        unsigned __int32 ui32[4];
        unsigned __int64 ui64[2];
        __m128 m128;
    } hData128;
    static_assert(sizeof(hData128) == 16);

    typedef union
    {
        float f32[8];
        double d64[4];
        __int8 i8[32];
        __int16 i16[16];
        __int32 i32[8];
        __int64 i64[4];
        unsigned __int8 ui8[32];
        unsigned __int16 ui16[16];
        unsigned __int32 ui32[8];
        unsigned __int64 ui64[4];
        hData128 h128[2];
        __m256 m256;
    } hData256;
    static_assert(sizeof(hData256) == 32);

#pragma warning(pop)
}
