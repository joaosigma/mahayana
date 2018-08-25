#pragma once

#include <xmmintrin.h>

#define HRESTRICT __restrict
#define HFUNC_RESTRICT __declspec(restrict)

namespace hr
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

	union hSplitUInt32{
		struct PacketBytes{
			hUInt8 byte0;
			hUInt8 byte1;
			hUInt8 byte2;
			hUInt8 byte3;
		} piecesBytes;

		struct PacketShorts{
			hUInt16 short0;
			hUInt16 short1;
		} piecesShort;

		hUInt32 valueWord;

		hSplitUInt32(hUInt32 valueWord)
			: valueWord(valueWord)
		{ }
	};

#pragma warning( push )
#pragma warning( disable : 4324)

	typedef union
	{
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
	static_assert(sizeof(hData128) == 16);

	typedef union 
	{
		float               f32[8];
		__int8              i8[32];
		__int16             i16[16];
		__int32             i32[8];
		unsigned __int8     ui8[32];
		unsigned __int16    ui16[16];
		unsigned __int32    ui32[8];
	} hData256;
	static_assert(sizeof(hData256) == 32);

#pragma warning( pop ) 

	bool operator == (const hData128& a, const hData128& b);
	bool operator != (const hData128& a, const hData128& b);
	bool operator < (const hData128& a, const hData128& b);
	bool operator > (const hData128& a, const hData128& b);
	bool operator <= (const hData128& a, const hData128& b);
	bool operator >= (const hData128& a, const hData128& b);

	bool operator == (const hData256& a, const hData256& b);
	bool operator != (const hData256& a, const hData256& b);
	bool operator < (const hData256& a, const hData256& b);
	bool operator > (const hData256& a, const hData256& b);
	bool operator <= (const hData256& a, const hData256& b);
	bool operator >= (const hData256& a, const hData256& b);
}