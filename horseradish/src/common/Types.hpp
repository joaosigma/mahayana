#pragma once

#include <xmmintrin.h>

#define HRESTRICT __restrict
#define HFUNC_RESTRICT __declspec(restrict)
#define HALIGN_16BYTES __declspec(align(16))

namespace HorseRadish
{
	typedef size_t hSize;
	typedef ptrdiff_t hPrtDiff;

	typedef signed char hChar;

	typedef signed __int8 hInt8;
	typedef signed __int16 hInt16;
	typedef signed __int32 hInt32;
	typedef signed __int64 hInt64;

	typedef unsigned __int8 hUInt8;
	typedef unsigned __int16 hUInt16;
	typedef unsigned __int32 hUInt32;
	typedef unsigned __int64 hUInt64;

	typedef float hFloat;
	typedef double hDouble;

	typedef HALIGN_16BYTES union{
		 float               f32[4];
		 double              d64[2];
		 __int8              i8[16];
		 __int16             i16[8];
		 __int32             i32[4];
		 __int64             i64[2];
		 unsigned __int8     ui8[16];
		 unsigned __int16    ui16[8];
		 unsigned __int32    ui32[4];
		 unsigned __int64    ui64[2];
		 __m128              m128;
	} hData128;

	typedef HALIGN_16BYTES union{
		 float               f32[5];
		 __int8              i8[20];
		 __int16             i16[10];
		 __int32             i32[5];
		 unsigned __int8     ui8[20];
		 unsigned __int16    ui16[10];
		 unsigned __int32    ui32[5];
	} hData160;
}