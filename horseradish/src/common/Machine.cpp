#include "Machine.hpp"

#include "Math.hpp"

#include <intrin.h>

namespace HorseRadish
{
	bool Machine::CPUGetVendorID(String &outputValue)
	{
		int cpuInfo[4];
		char cpuString[128];

		memset(cpuInfo, 0, sizeof(cpuInfo));
		__cpuid(cpuInfo, 0x0);

		memset(cpuString, 0, sizeof(cpuString));
		memcpy(cpuString + 0, cpuInfo + 1, sizeof(int));
		memcpy(cpuString + 4, cpuInfo + 3, sizeof(int));
		memcpy(cpuString + 8, cpuInfo + 2, sizeof(int));

		outputValue.Set(String::Encoding::ASCII, cpuString);
		return true;
	}

	bool Machine::CPUGetProcessorName(String &outputValue)
	{
		int cpuInfo[4];
		char cpuString[128];
		char *walker, *write;

		memset(cpuInfo, 0, sizeof(cpuInfo));
		__cpuid(cpuInfo, 0x80000000);

		if (cpuInfo[0] < 0x80000004)
		{
			outputValue.Set(String::Encoding::UTF8, "<empty>");
			return true;
		}

		memset(cpuString, 0, sizeof(cpuString));

		__cpuid(cpuInfo, 0x80000002);
		memcpy(cpuString + 0, cpuInfo + 0, sizeof(int));
		memcpy(cpuString + 4, cpuInfo + 1, sizeof(int));
		memcpy(cpuString + 8, cpuInfo + 2, sizeof(int));
		memcpy(cpuString + 12, cpuInfo + 3, sizeof(int));
		__cpuid(cpuInfo, 0x80000003);
		memcpy(cpuString + 16, cpuInfo + 0, sizeof(int));
		memcpy(cpuString + 20, cpuInfo + 1, sizeof(int));
		memcpy(cpuString + 24, cpuInfo + 2, sizeof(int));
		memcpy(cpuString + 28, cpuInfo + 3, sizeof(int));
		__cpuid(cpuInfo, 0x80000004);
		memcpy(cpuString + 32, cpuInfo + 0, sizeof(int));
		memcpy(cpuString + 36, cpuInfo + 1, sizeof(int));
		memcpy(cpuString + 40, cpuInfo + 2, sizeof(int));
		memcpy(cpuString + 44, cpuInfo + 3, sizeof(int));

		outputValue.Set(String::Encoding::ASCII, cpuString);
		outputValue.RemoveDoubles(' ');
		outputValue.Trim();

		return true;
	}

	bool Machine::CPUCheckFeatures(const CPUFeature &featuresCheck)
	{
		int cpuInfo[4];

		memset(cpuInfo, 0, sizeof(cpuInfo));
		__cpuid(cpuInfo, 0x1);

		if ((featuresCheck & CPUFeature::SSE) == CPUFeature::SSE)
		{
			if ((cpuInfo[3] & 0x2000000) != 0x2000000)
				return false;
		}

		if ((featuresCheck & CPUFeature::SSE2) == CPUFeature::SSE2)
		{
			if ((cpuInfo[3] & 0x4000000) != 0x4000000)
				return false;
		}

		if ((featuresCheck & CPUFeature::HyperThreading) == CPUFeature::HyperThreading)
		{
			if ((cpuInfo[3] & 0x10000000) != 0x10000000)
				return false;
		}

		if ((featuresCheck & CPUFeature::CMov) == CPUFeature::CMov)
		{
			if ((cpuInfo[3] & 0x8000) != 0x8000)
				return false;
		}

		return true;
	}

	hUInt32 Machine::FastCompressGetSize(const hUInt32 uncompressedSize)
	{
		return (((uncompressedSize * 9 + 7) >> 3) + 1);
	}

	hUInt32 Machine::FastCompress(unsigned char * const dest, const unsigned char * const source, const hUInt32 sourceSize)
	{
		if (sourceSize == 0 || dest == nullptr || source == nullptr)
			return 0;

		dest[0] = source[0];

		if (sourceSize == 1)
			return 1;

		hUInt32 controlOffset = 1;
		hUInt32 controlMask = 1;
		hUInt32 controlByte = 0;

		hUInt32 offset = 1;
		hUInt32 destOffset = 2;

		while (offset < sourceSize)
		{
			hUInt32 maxOffsetSearch = offset;
			if (maxOffsetSearch > 32)
				maxOffsetSearch = 32;

			hUInt32 bestSearchOffset = 0;
			hUInt32 bestSearchLen = 0;

			hUInt32 searchOffset;
			for (searchOffset = 1; searchOffset <= maxOffsetSearch; ++searchOffset)
			{
				hUInt32 maxSearchLen = sourceSize - offset + searchOffset;
				if (maxSearchLen > 9)
					maxSearchLen = 9;

				hUInt32 searchLen;
				for (searchLen = 0; searchLen < maxSearchLen; ++searchLen)
				{
					if (source[offset - searchOffset + searchLen] != source[offset + searchLen])
						break;
				}

				if (searchLen <= bestSearchLen)
					continue;

				bestSearchLen = searchLen;
				bestSearchOffset = searchOffset;

				if (searchLen == 9)
					break;
			}

			if (bestSearchLen > 1)
			{
				controlByte |= controlMask;

				dest[destOffset] = hUInt8(((bestSearchOffset - 1) << 3) | (bestSearchLen - 2));
				offset += bestSearchLen;
			}
			else
			{
				dest[destOffset] = source[offset];
				++offset;
			}

			++destOffset;

			controlMask += controlMask;

			if (controlMask < 0x100)
				continue;

			dest[controlOffset] = hUInt8(controlByte);

			controlByte = 0;
			controlMask = 1;
			controlOffset = destOffset;

			++destOffset;
		}

		if ((controlOffset + 1) == destOffset)
			--destOffset;
		else
			dest[controlOffset] = hUInt8(controlByte);
		return destOffset;
	}

	hUInt32 Machine::FastDecompress(unsigned char * const dest, const unsigned char * const source, const hUInt32 sourceSize)
	{
		if (sourceSize == 0 || dest == nullptr || source == nullptr)
			return 0;

		dest[0] = source[0];

		if (sourceSize == 1)
			return 1;

		hUInt32 controlMask = 1;
		hUInt32 controlByte = source[1];

		hUInt32 offset = 2;
		hUInt32 destOffset = 1;

		while (offset < sourceSize)
		{
			if ((controlByte & controlMask) == 0)
			{
				dest[destOffset] = source[offset];

				++destOffset;
			}
			else
			{
				hUInt32 copyOffset = source[offset];
				hUInt32 copyLen = (copyOffset & 0x7) + 2;
				copyOffset >>= 3;
				copyOffset += 1;

				hUInt32 o;
				for (o = 0; o < copyLen; ++o)
					dest[destOffset + o] = dest[destOffset - copyOffset + o];

				destOffset += copyLen;
			}

			++offset;

			controlMask += controlMask;

			if (controlMask < 0x100)
				continue;

			controlByte = source[offset];
			++offset;

			controlMask = 1;
		}

		return destOffset;
	}

	void Machine::AsmBufferClear(void* dest, size_t bytes)
	{

#ifdef _M_X64

		memset(dest, 0, bytes);

#else

		__asm
		{
			mov		ebx, [bytes]
			mov		edi, [dest]

			cmp		ebx, 128
			jb		$memclr_the_end

			xor		edx, edx
			mov		eax, edi
			mov		ecx, 16
			div		ecx

			pxor	xmm0, xmm0
			pxor	xmm1, xmm1
			pxor	xmm2, xmm2
			pxor	xmm3, xmm3

			cmp		edx, 0
			je		$memclr_align_done

			sub		ecx, edx
			sub		ebx, ecx
			xor		eax, eax

			rep		stos al

		$memclr_align_done :

			cmp		ebx, 128
			jb		$memclr_the_end

			sub		ebx, 128
			movntdq[edi + 0], xmm0
			movntdq[edi + 16], xmm1
			movntdq[edi + 32], xmm2
			movntdq[edi + 48], xmm3
			movntdq[edi + 64], xmm0
			movntdq[edi + 80], xmm1
			movntdq[edi + 96], xmm2
			movntdq[edi + 112], xmm3
			add		edi, 128

			jmp		$memclr_align_done

		$memclr_the_end :

			mov		ecx, ebx
			xor		eax, eax
			rep		stos al
		}

#endif

	}

	void Machine::AsmBufferCopy(void* dest, const void* src, size_t bytes)
	{

#ifdef _M_X64

		memcpy(dest, src, bytes);

#else

		__asm {

			// Copyright (C) 2009  Jan Boon (Kaetemi)
			// optimized on Intel Core 2 Duo T7500

			mov         ecx, bytes
			mov         edi, dest
			mov         esi, src
			add         ecx, edi

			prefetchnta[esi]
			prefetchnta[esi + 32]
			prefetchnta[esi + 64]
			prefetchnta[esi + 96]

			// handle nBytes lower than 128
			cmp         bytes, 512
			jge         fast
		slow :
			mov         bl, [esi]
			mov[edi], bl
			inc         edi
			inc         esi
			cmp         ecx, edi
			jnz         slow
			jmp         end

		fast :
			// align dstEnd to 128 bytes
			and         ecx, 0xFFFFFF80

			// get srcEnd aligned to dstEnd aligned to 128 bytes
			mov         ebx, esi
			sub         ebx, edi
			add         ebx, ecx

			// skip unaligned copy if dst is aligned
			mov         eax, edi
			and         edi, 0xFFFFFF80
			cmp         eax, edi
			jne         first
			jmp         more

		first :
			// copy the first 128 bytes unaligned
			movdqu      xmm0, [esi]
			movdqu      xmm1, [esi + 16]
			movdqu      xmm2, [esi + 32]
			movdqu      xmm3, [esi + 48]

			movdqu      xmm4, [esi + 64]
			movdqu      xmm5, [esi + 80]
			movdqu      xmm6, [esi + 96]
			movdqu      xmm7, [esi + 112]

			movdqu[eax], xmm0
			movdqu[eax + 16], xmm1
			movdqu[eax + 32], xmm2
			movdqu[eax + 48], xmm3

			movdqu[eax + 64], xmm4
			movdqu[eax + 80], xmm5
			movdqu[eax + 96], xmm6
			movdqu[eax + 112], xmm7

			// add 128 bytes to edi aligned earlier
			add         edi, 128

			// offset esi by the same value
			sub         eax, edi
			sub         esi, eax

			// last bytes if dst at dstEnd
			cmp         ecx, edi
			jnz         more
			jmp         last

		more :
			// handle equally aligned arrays
			mov         eax, esi
			and         eax, 0xFFFFFF80
			cmp         eax, esi
			jne         unaligned4k

		aligned4k :
			mov         eax, esi
			add         eax, 4096
			cmp         eax, ebx
			jle         aligned4kin
			cmp         ecx, edi
			jne         alignedlast
			jmp         last

		aligned4kin :
			prefetchnta[esi]
			prefetchnta[esi + 32]
			prefetchnta[esi + 64]
			prefetchnta[esi + 96]

			add         esi, 128

			cmp         eax, esi
			jne         aligned4kin

			sub         esi, 4096

		alinged4kout:
			movdqa      xmm0, [esi]
			movdqa      xmm1, [esi + 16]
			movdqa      xmm2, [esi + 32]
			movdqa      xmm3, [esi + 48]

			movdqa      xmm4, [esi + 64]
			movdqa      xmm5, [esi + 80]
			movdqa      xmm6, [esi + 96]
			movdqa      xmm7, [esi + 112]

			movntdq[edi], xmm0
			movntdq[edi + 16], xmm1
			movntdq[edi + 32], xmm2
			movntdq[edi + 48], xmm3

			movntdq[edi + 64], xmm4
			movntdq[edi + 80], xmm5
			movntdq[edi + 96], xmm6
			movntdq[edi + 112], xmm7

			add         esi, 128
			add         edi, 128

			cmp         eax, esi
			jne         alinged4kout
			jmp         aligned4k

		alignedlast :
			mov         eax, esi

		alignedlastin :
			prefetchnta[esi]
			prefetchnta[esi + 32]
			prefetchnta[esi + 64]
			prefetchnta[esi + 96]

			add         esi, 128

			cmp         ebx, esi
			jne         alignedlastin

			mov         esi, eax

		alignedlastout :
			movdqa      xmm0, [esi]
			movdqa      xmm1, [esi + 16]
			movdqa      xmm2, [esi + 32]
			movdqa      xmm3, [esi + 48]

			movdqa      xmm4, [esi + 64]
			movdqa      xmm5, [esi + 80]
			movdqa      xmm6, [esi + 96]
			movdqa      xmm7, [esi + 112]

			movntdq[edi], xmm0
			movntdq[edi + 16], xmm1
			movntdq[edi + 32], xmm2
			movntdq[edi + 48], xmm3

			movntdq[edi + 64], xmm4
			movntdq[edi + 80], xmm5
			movntdq[edi + 96], xmm6
			movntdq[edi + 112], xmm7

			add         esi, 128
			add         edi, 128

			cmp         ecx, edi
			jne         alignedlastout
			jmp         last

		unaligned4k :
			mov         eax, esi
			add         eax, 4096
			cmp         eax, ebx
			jle         unaligned4kin
			cmp         ecx, edi
			jne         unalignedlast
			jmp         last

		unaligned4kin :
			prefetchnta[esi]
			prefetchnta[esi + 32]
			prefetchnta[esi + 64]
			prefetchnta[esi + 96]

			add         esi, 128

			cmp         eax, esi
			jne         unaligned4kin

			sub         esi, 4096

		unalinged4kout:
			movdqu      xmm0, [esi]
			movdqu      xmm1, [esi + 16]
			movdqu      xmm2, [esi + 32]
			movdqu      xmm3, [esi + 48]

			movdqu      xmm4, [esi + 64]
			movdqu      xmm5, [esi + 80]
			movdqu      xmm6, [esi + 96]
			movdqu      xmm7, [esi + 112]

			movntdq[edi], xmm0
			movntdq[edi + 16], xmm1
			movntdq[edi + 32], xmm2
			movntdq[edi + 48], xmm3

			movntdq[edi + 64], xmm4
			movntdq[edi + 80], xmm5
			movntdq[edi + 96], xmm6
			movntdq[edi + 112], xmm7

			add         esi, 128
			add         edi, 128

			cmp         eax, esi
			jne         unalinged4kout
			jmp         unaligned4k

		unalignedlast :
			mov         eax, esi

		unalignedlastin :
			prefetchnta[esi]
			prefetchnta[esi + 32]
			prefetchnta[esi + 64]
			prefetchnta[esi + 96]

			add         esi, 128

			cmp         ebx, esi
			jne         unalignedlastin

			mov         esi, eax

		unalignedlastout :
			movdqu      xmm0, [esi]
			movdqu      xmm1, [esi + 16]
			movdqu      xmm2, [esi + 32]
			movdqu      xmm3, [esi + 48]

			movdqu      xmm4, [esi + 64]
			movdqu      xmm5, [esi + 80]
			movdqu      xmm6, [esi + 96]
			movdqu      xmm7, [esi + 112]

			movntdq[edi], xmm0
			movntdq[edi + 16], xmm1
			movntdq[edi + 32], xmm2
			movntdq[edi + 48], xmm3

			movntdq[edi + 64], xmm4
			movntdq[edi + 80], xmm5
			movntdq[edi + 96], xmm6
			movntdq[edi + 112], xmm7

			add         esi, 128
			add         edi, 128

			cmp         ecx, edi
			jne         unalignedlastout
			jmp         last

		last :
			// get the last 128 bytes
			mov         ecx, bytes
			mov         edi, dest
			mov         esi, src
			add         edi, ecx
			add         esi, ecx
			sub         edi, 128
			sub         esi, 128

			// copy the last 128 bytes unaligned
			movdqu      xmm0, [esi]
			movdqu      xmm1, [esi + 16]
			movdqu      xmm2, [esi + 32]
			movdqu      xmm3, [esi + 48]

			movdqu      xmm4, [esi + 64]
			movdqu      xmm5, [esi + 80]
			movdqu      xmm6, [esi + 96]
			movdqu      xmm7, [esi + 112]

			movdqu[edi], xmm0
			movdqu[edi + 16], xmm1
			movdqu[edi + 32], xmm2
			movdqu[edi + 48], xmm3

			movdqu[edi + 64], xmm4
			movdqu[edi + 80], xmm5
			movdqu[edi + 96], xmm6
			movdqu[edi + 112], xmm7

		end :
		}

#endif

	}

	void Machine::AsmBufferCopyAligned(void* dest, const void* src, size_t multiple128Bytes)
	{

#ifdef _M_X64

		memcpy(dest, src, multiple128Bytes * 128);

#else

		__asm
		{
			mov esi, src;    //src pointer
			mov edi, dest;   //dest pointer

			mov ebx, multiple128Bytes; //ebx is our counter 

		loop_copy:
			prefetchnta 128[ESI]; //SSE2 prefetch
			prefetchnta 160[ESI];
			prefetchnta 192[ESI];
			prefetchnta 224[ESI];

			movdqa xmm0, 0[ESI]; //move data from src to registers
			movdqa xmm1, 16[ESI];
			movdqa xmm2, 32[ESI];
			movdqa xmm3, 48[ESI];
			movdqa xmm4, 64[ESI];
			movdqa xmm5, 80[ESI];
			movdqa xmm6, 96[ESI];
			movdqa xmm7, 112[ESI];

			movntdq 0[EDI], xmm0; //move data from registers to dest
			movntdq 16[EDI], xmm1;
			movntdq 32[EDI], xmm2;
			movntdq 48[EDI], xmm3;
			movntdq 64[EDI], xmm4;
			movntdq 80[EDI], xmm5;
			movntdq 96[EDI], xmm6;
			movntdq 112[EDI], xmm7;

			add esi, 128;
			add edi, 128;
			dec ebx;

			jnz loop_copy; //loop please
		loop_copy_end:
		}

#endif

	}

	void Machine::AsmBufferSetUBYTE(void* dest, unsigned char val, size_t bytes)
	{

#ifdef _M_X64

		memset(dest, val, bytes);

#else

		__asm 
		{

			mov		ebx, bytes
			mov		edi, dest

			cmp		ebx, 128
			jb		$memset_the_end

			mov		al, val
			mov		ah, al
			shl		eax, 16
			mov		al, val
			mov		ah, al
			movd	xmm0, eax
			shufps	xmm0, xmm0, 0
			movdqa	xmm1, xmm0
			movdqa	xmm2, xmm0
			movdqa	xmm3, xmm0

			xor		edx, edx
			mov		eax, edi
			mov		ecx, 16
			div		ecx

			cmp		edx, 0
			je		$memset_align_done

			sub		ecx, edx
			sub		ebx, ecx
			mov		al, val

			rep		stos al

		$memset_align_done :

			cmp		ebx, 128
			jb		$memset_the_end

			sub		ebx, 128
			movntdq[edi + 0], xmm0
			movntdq[edi + 16], xmm1
			movntdq[edi + 32], xmm2
			movntdq[edi + 48], xmm3
			movntdq[edi + 64], xmm0
			movntdq[edi + 80], xmm1
			movntdq[edi + 96], xmm2
			movntdq[edi + 112], xmm3
			add		edi, 128
			jmp		$memset_align_done

		$memset_the_end :
			mov		ecx, ebx
			mov		al, val
			rep		stos al
		}

#endif

	}

	void Machine::AsmBufferSetUI32(void* dest, unsigned int val, size_t bytes)
	{

#ifdef _M_X64

		for(size_t multiplosInts = bytes / sizeof(unsigned int); multiplosInts > 0; multiplosInts--, bytes -= sizeof(unsigned int), dest = ((unsigned int*)dest) + 1)
			*((unsigned int*)dest) = val;

		auto resto = bytes % sizeof(unsigned int);
		if (resto > 0)
			memcpy(dest, &val, resto);

#else

		__asm 
		{
			mov		ebx, bytes
			mov		edi, dest

			cmp		ebx, 128
			jb		$memset_i32_the_end

			xor		edx, edx
			mov		eax, edi
			mov		ecx, 8
			div		ecx

			movd	xmm0, val
			shufps	xmm0, xmm0, 0
			movdqa	xmm1, xmm0
			movdqa	xmm2, xmm0
			movdqa	xmm3, xmm0

			cmp		edx, 0
			je		$memset_i32_align

		$memset_i32_no_align :

			cmp		ebx, 128
			jb		$memset_i32_the_end

			sub		ebx, 128
			movdqu[edi + 0], xmm0
			movdqu[edi + 16], xmm1
			movdqu[edi + 32], xmm2
			movdqu[edi + 48], xmm3
			movdqu[edi + 64], xmm0
			movdqu[edi + 80], xmm1
			movdqu[edi + 96], xmm2
			movdqu[edi + 112], xmm3
			add		edi, 128
			jmp		$memset_i32_no_align

		$memset_i32_align :

			cmp		ebx, 128
			jb		$memset_i32_the_end

			sub		ebx, 128
			movntdq[edi + 0], xmm0
			movntdq[edi + 16], xmm1
			movntdq[edi + 32], xmm2
			movntdq[edi + 48], xmm3
			movntdq[edi + 64], xmm0
			movntdq[edi + 80], xmm1
			movntdq[edi + 96], xmm2
			movntdq[edi + 112], xmm3
			add		edi, 128
			jmp		$memset_i32_align

		$memset_i32_the_end :

			mov		ecx, ebx
			shr		ecx, 2
			mov		eax, val
			rep		stos eax

			and		ebx, 0x3

			cmp		ebx, 0
			je		$memset_i32_finally
			mov		eax, 0xff
			shl		eax, 0
			and		eax, val
			shr		eax, 0
			and		eax, 0xff
			mov		BYTE PTR[edi], al
			dec		ebx

			cmp		ebx, 0
			je		$memset_i32_finally
			mov		eax, 0xff
			shl		eax, 8
			and		eax, val
			shr		eax, 8
			and		eax, 0xff
			mov		BYTE PTR[edi + 1], al
			dec		ebx

			cmp		ebx, 0
			je		$memset_i32_finally
			mov		eax, 0xff
			shl		eax, 16
			and		eax, val
			shr		eax, 16
			and		eax, 0xff
			mov		BYTE PTR[edi + 2], al

		$memset_i32_finally :
		}

#endif

	}

	void Machine::AsmFloat2UByte(unsigned char *dest, const float *src, const unsigned int num, const float mulVal, const float addVal)
	{

#ifdef _M_X64

		for(int i = 0; i < num; i++)
			dest[i] = Math::iClampZero((Math::ftoi(src[i]) * mulVal) + addVal, 255);

#else

		static const __declspec(align(16)) float _255 = 255.0f;
		int intVal;

		__asm
		{
			mov ebx, num
			mov edi, dest
			mov esi, src

			cmp		ebx, 32
			jbe		$convF_just_last

			xor		edx, edx
			mov		eax, esi
			mov		ecx, 16
			div		ecx
			cmp		edx, 0
			je		$convF_align_done

			cmp		edx, 4
			je		$convF_align_cando
			cmp		edx, 8
			je		$convF_align_cando
			cmp		edx, 12
			je		$convF_align_cando

			movd	xmm4, mulVal
			movd	xmm5, addVal

			shufps	xmm4, xmm4, 0x00
			shufps	xmm5, xmm5, 0x00

			jmp		$convF_loopUA_16

		$convF_align_cando :
			shr		edx, 2
			sub		ecx, edx
			sub		ebx, ecx

			fld		mulVal
			fld		addVal
			fldz
			fld		_255

		$convF_align_loop :
			cmp		ecx, 0
			je		$convF_align_done_pop

			fld[esi]
			fmul	st(0), st(4)
			fadd	st(0), st(3)
			fcomi	st(0), st(1)
			fcmovnb	st(0), st(1)
			fcomi	st(0), st(2)
			fcmovb	st(0), st(2)
			fistp[edi]

			sub		ecx, 1
			add		esi, 4
			add		edi, 1
			jmp		$convF_align_loop

		$convF_align_done_pop :

			ffreep st(0)
			ffreep st(0)
			ffreep st(0)
			ffreep st(0)

		$convF_align_done :

			movd	xmm4, mulVal
			movd	xmm5, addVal

			shufps	xmm4, xmm4, 0x00
			shufps	xmm5, xmm5, 0x00

		$convF_loopA_16:
			cmp ebx, 16
			jb $convF_loop_1

			prefetchnta[esi + 64]

			movaps xmm0, [esi]
			movaps xmm1, [esi + 16]
			movaps xmm2, [esi + 32]
			movaps xmm3, [esi + 48]
			mulps xmm0, xmm4
			mulps xmm1, xmm4
			mulps xmm2, xmm4
			mulps xmm3, xmm4
			addps xmm0, xmm5
			addps xmm1, xmm5
			addps xmm2, xmm5
			addps xmm3, xmm5

			cvtps2pi mm0, xmm0
			cvtps2pi mm2, xmm1
			movhlps xmm0, xmm0
			movhlps xmm1, xmm1
			cvtps2pi mm1, xmm0
			cvtps2pi mm3, xmm1

			packssdw mm0, mm1
			packuswb mm0, mm0

			packssdw mm2, mm3
			packuswb mm2, mm2

			movd[edi + 0], mm0
			movd[edi + 4], mm2

			cvtps2pi mm0, xmm2
			cvtps2pi mm2, xmm3
			movhlps xmm2, xmm2
			movhlps xmm3, xmm3
			cvtps2pi mm1, xmm2
			cvtps2pi mm3, xmm3

			packssdw mm0, mm1
			packuswb mm0, mm0

			packssdw mm2, mm3
			packuswb mm2, mm2

			movd[edi + 8], mm0
			movd[edi + 12], mm2

			sub ebx, 16
			add esi, 64
			add edi, 16
			jmp $convF_loopA_16

		$convF_loopUA_16 :
			cmp ebx, 16
			jb $convF_just_last_usedMMX

			prefetchnta[esi + 64]

			movups xmm0, [esi]
			movups xmm1, [esi + 16]
			movups xmm2, [esi + 32]
			movups xmm3, [esi + 48]
			mulps xmm0, xmm4
			mulps xmm1, xmm4
			mulps xmm2, xmm4
			mulps xmm3, xmm4
			addps xmm0, xmm5
			addps xmm1, xmm5
			addps xmm2, xmm5
			addps xmm3, xmm5

			cvtps2pi mm0, xmm0
			cvtps2pi mm2, xmm1
			movhlps xmm0, xmm0
			movhlps xmm1, xmm1
			cvtps2pi mm1, xmm0
			cvtps2pi mm3, xmm1

			packssdw mm0, mm1
			packuswb mm0, mm0

			packssdw mm2, mm3
			packuswb mm2, mm2

			movd[edi + 0], mm0
			movd[edi + 4], mm2

			cvtps2pi mm0, xmm2
			cvtps2pi mm2, xmm3
			movhlps xmm2, xmm2
			movhlps xmm3, xmm3
			cvtps2pi mm1, xmm2
			cvtps2pi mm3, xmm3

			packssdw mm0, mm1
			packuswb mm0, mm0

			packssdw mm2, mm3
			packuswb mm2, mm2

			movd[edi + 8], mm0
			movd[edi + 12], mm2

			sub ebx, 16
			add esi, 64
			add edi, 16
			jmp $convF_loopUA_16

		$convF_loop_1 :
			cmp ebx, 4
			jb $convF_just_last_usedMMX

			movaps xmm0, [esi]
			mulps xmm0, xmm4
			addps xmm0, xmm5
			cvtps2pi mm0, xmm0
			movhlps xmm0, xmm0
			cvtps2pi mm1, xmm0

			packssdw mm0, mm1
			packuswb mm0, mm0

			movd[edi], mm0

			sub ebx, 4
			add esi, 16
			add edi, 4
			jmp	$convF_loop_1

		$convF_just_last_usedMMX :
			emms

		$convF_just_last :
			cmp ebx, 0
			je $convF_finalEnd

			fld		mulVal
			fld		addVal
			fldz
			fld		_255

		$convF_just_last_do1 :
			cmp		ebx, 0
			je		$convF_just_last_do1_end

			fld[esi]
			fmul	st(0), st(4)
			fadd	st(0), st(3)
			fcomi	st(0), st(1)
			fcmovnb	st(0), st(1)
			fcomi	st(0), st(2)
			fcmovb	st(0), st(2)
			fistp	intVal
			mov		eax, intVal
			mov[edi], al

			sub		ebx, 1
			add		esi, 4
			add		edi, 1
			jmp		$convF_just_last_do1

		$convF_just_last_do1_end :
			ffreep st(0)
			ffreep st(0)
			ffreep st(0)
			ffreep st(0)

		$convF_finalEnd :
		}

#endif

	}

	void Machine::AsmUByte2Float(float *dest, const unsigned char *src, const unsigned int num, const float mulVal, const float addVal)
	{

#ifdef _M_X64

		for(int i = 0; i < num; i++)
			dest[i] = ((float)src[i]) * mulVal + addVal;

#else

		static __declspec(align(16)) int auxBuffer;

		__asm
		{
			mov ebx, num
			mov edi, dest
			mov esi, src

			cmp		ebx, 32
			jbe		$convF_just_last

			xor		edx, edx
			mov		eax, edi
			mov		ecx, 16
			div		ecx
			cmp		edx, 0
			je		$convF_align_done

			cmp		edx, 4
			je		$convF_align_cando
			cmp		edx, 8
			je		$convF_align_cando
			cmp		edx, 12
			je		$convF_align_cando

			pxor	mm7, mm7
			pxor	xmm5, xmm5
			movd	xmm6, mulVal
			movd	xmm7, addVal
			shufps	xmm6, xmm6, 0x00
			shufps	xmm7, xmm7, 0x00

			jmp		$convF_loopUA_16

		$convF_align_cando :
			shr		edx, 2
			sub		ecx, edx
			sub		ebx, ecx

			fld		addVal
			fld		mulVal

		$convF_align_loop :
			cmp		ecx, 0
			je		$convF_align_done_pop

			movzx	eax, BYTE PTR[esi]
			mov		DWORD PTR[auxBuffer], eax
			fild[auxBuffer]
			fmul	st(0), st(1)
			fadd	st(0), st(2)
			fstp[edi]

			sub		ecx, 1
			add		esi, 1
			add		edi, 4
			jmp		$convF_align_loop

		$convF_align_done_pop :

			ffreep st(0)
			ffreep st(0)

		$convF_align_done :

			pxor	mm7, mm7
			pxor	xmm5, xmm5
			movd	xmm6, mulVal
			movd	xmm7, addVal
			shufps	xmm6, xmm6, 0x00
			shufps	xmm7, xmm7, 0x00

		$convF_loopA_16:
			cmp ebx, 20
			jb $convF_just_last_usedMMX

			prefetchnta[esi + 24]

			movd	mm0, [esi + 0]
			movd	mm1, [esi + 4]
			movd	mm2, [esi + 8]
			movd	mm3, [esi + 12]
			movd	mm4, [esi + 16]

			punpcklbw	mm0, mm7
			punpcklbw	mm1, mm7
			punpcklbw	mm2, mm7
			punpcklbw	mm3, mm7
			punpcklbw	mm4, mm7

			movq2dq		xmm0, mm0
			movq2dq		xmm1, mm1
			movq2dq		xmm2, mm2
			movq2dq		xmm3, mm3
			movq2dq		xmm4, mm4

			punpcklwd	xmm0, xmm5
			punpcklwd	xmm1, xmm5
			punpcklwd	xmm2, xmm5
			punpcklwd	xmm3, xmm5
			punpcklwd	xmm4, xmm5

			cvtdq2ps xmm0, xmm0
			cvtdq2ps xmm1, xmm1
			cvtdq2ps xmm2, xmm2
			cvtdq2ps xmm3, xmm3
			cvtdq2ps xmm4, xmm4

			mulps	xmm0, xmm6
			mulps	xmm1, xmm6
			mulps	xmm2, xmm6
			mulps	xmm3, xmm6
			mulps	xmm4, xmm6

			addps	xmm0, xmm7
			addps	xmm1, xmm7
			addps	xmm2, xmm7
			addps	xmm3, xmm7
			addps	xmm4, xmm7

			movntps[edi + 0], xmm0
			movntps[edi + 16], xmm1
			movntps[edi + 32], xmm2
			movntps[edi + 48], xmm3
			movntps[edi + 64], xmm4

			sub ebx, 20
			add esi, 20
			add edi, 80
			jmp $convF_loopA_16

		$convF_loopUA_16 :
			cmp ebx, 20
			jb $convF_just_last_usedMMX

			prefetchnta[esi + 24]

			movd	mm0, [esi + 0]
			movd	mm1, [esi + 4]
			movd	mm2, [esi + 8]
			movd	mm3, [esi + 12]
			movd	mm4, [esi + 16]

			punpcklbw	mm0, mm7
			punpcklbw	mm1, mm7
			punpcklbw	mm2, mm7
			punpcklbw	mm3, mm7
			punpcklbw	mm4, mm7

			movq2dq		xmm0, mm0
			movq2dq		xmm1, mm1
			movq2dq		xmm2, mm2
			movq2dq		xmm3, mm3
			movq2dq		xmm4, mm4

			punpcklwd	xmm0, xmm5
			punpcklwd	xmm1, xmm5
			punpcklwd	xmm2, xmm5
			punpcklwd	xmm3, xmm5
			punpcklwd	xmm4, xmm5

			cvtdq2ps xmm0, xmm0
			cvtdq2ps xmm1, xmm1
			cvtdq2ps xmm2, xmm2
			cvtdq2ps xmm3, xmm3
			cvtdq2ps xmm4, xmm4

			mulps	xmm0, xmm6
			mulps	xmm1, xmm6
			mulps	xmm2, xmm6
			mulps	xmm3, xmm6
			mulps	xmm4, xmm6

			addps	xmm0, xmm7
			addps	xmm1, xmm7
			addps	xmm2, xmm7
			addps	xmm3, xmm7
			addps	xmm4, xmm7

			movups[edi + 0], xmm0
			movups[edi + 16], xmm1
			movups[edi + 32], xmm2
			movups[edi + 48], xmm3
			movups[edi + 64], xmm4

			sub ebx, 20
			add esi, 20
			add edi, 80
			jmp $convF_loopUA_16

		$convF_just_last_usedMMX :
			emms

		$convF_just_last :
			cmp ebx, 0
			je $convF_finalEnd

			fld		addVal
			fld		mulVal

		$convF_just_last_do1 :
			cmp		ebx, 0
			je		$convF_just_last_do1_end

			movzx	eax, BYTE PTR[esi]
			mov		DWORD PTR[auxBuffer], eax
			fild[auxBuffer]
			fmul	st(0), st(1)
			fadd	st(0), st(2)
			fstp[edi]

			sub		ebx, 1
			add		esi, 1
			add		edi, 4
			jmp		$convF_just_last_do1

		$convF_just_last_do1_end :
			ffreep st(0)
			ffreep st(0)

		$convF_finalEnd :
		}

#endif

	}

} //HorseRadish