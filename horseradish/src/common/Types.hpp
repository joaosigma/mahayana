#pragma once
#ifndef __HTYPES__
#define __HTYPES__

#include <xmmintrin.h>

#define HRESTRICT __restrict
#define HFUNC_RESTRICT __declspec(restrict)
#define HALIGN_16BYTES __declspec(align(16))

namespace HorseRadish
{
	//o tamanho do sizeof (e para aritmética de ponteiros)
	typedef size_t hSize;
	typedef ptrdiff_t hPrtDiff;

	//o que é usado para caracteres
	typedef signed char hChar;

	//inteiros com sinal
	typedef signed __int8 hInt8;
	typedef signed __int16 hInt16;
	typedef signed __int32 hInt32;
	typedef signed __int64 hInt64;

	//inteiros sem sinal
	typedef unsigned __int8 hUInt8;
	typedef unsigned __int16 hUInt16;
	typedef unsigned __int32 hUInt32;
	typedef unsigned __int64 hUInt64;

	//virgula flutuante
	typedef float hFloat;
	typedef double hDouble;

	//para guardar dados com 128 bits
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

	//para guardar dados com 160 bits
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

#endif