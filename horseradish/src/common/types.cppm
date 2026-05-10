module;

#include <cstdint>
#include <immintrin.h>
#include <xmmintrin.h>

export module core:types;

import std;
import :math;

namespace hr::types
{
    namespace detail
    {
        namespace
        {
            constexpr float shortScaleTo = static_cast<float>(std::numeric_limits<int16_t>::max());
            constexpr float shortScaleFrom = 1.0f / static_cast<float>(std::numeric_limits<int16_t>::max());

            constexpr float ushortScaleTo = static_cast<float>(std::numeric_limits<uint16_t>::max());
            constexpr float ushortScaleFrom = 1.0f / static_cast<float>(std::numeric_limits<uint16_t>::max());
        }

        void pack(const float* const in, int16_t* const out, size_t numValues)
        {
            for (size_t i = 0; i < numValues; i++)
                out[i] = static_cast<int16_t>(Math::fClamp(in[i], -1.0f, 1.0f) * shortScaleTo);
        }

        void pack(const float* const in, uint16_t* const out, size_t numValues)
        {
            for (size_t i = 0; i < numValues; i++)
                out[i] = static_cast<uint16_t>(Math::fClamp(in[i], 0.0f, 1.0f) * ushortScaleTo);
        }

        void unpack(const int16_t* const in, float* const out, size_t numValues)
        {
            for (size_t i = 0; i < numValues; i++)
                out[i] = static_cast<float>(in[i]) * shortScaleFrom;
        }

        void unpack(const uint16_t* const in, float* const out, size_t numValues)
        {
            for (size_t i = 0; i < numValues; i++)
                out[i] = static_cast<float>(in[i]) * ushortScaleFrom;
        }
    }

    export
    typedef signed char hChar;

    export
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

    export
    template<typename T>
    T packFloat(const float value)
    {
        static_assert(std::is_arithmetic_v<T>);

        T out;
        detail::pack(&value, &out, 1);

        return out;
    }

    export
    template<typename T>
    float unpackFloat(const T value)
    {
        static_assert(std::is_arithmetic_v<T>);

        float out;
        detail::unpack(&value, &out, 1);

        return out;
    }

    export
    template<typename T>
    void packFloat(const float* const in, T* const out, size_t numValues)
    {
        static_assert(std::is_arithmetic_v<T>);
        detail::pack(in, out, numValues);
    }

    export
    template<typename T>
    void unpackFloat(const T* const in, float* const out, size_t numValues)
    {
        static_assert(std::is_arithmetic_v<T>);
        detail::unpack(in, out, numValues);
    }

#pragma warning(push)
#pragma warning(disable : 4324)

    export
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

    export
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
