#include "Hashing.hpp"
#include "Types.hpp"

#include <windows.h>

/*
   MD5 Message Digest Algorithm. (RFC1321)

This code implements the MD5 message-digest algorithm.
The algorithm is due to Ron Rivest.  This code was
written by Colin Plumb in 1993, no copyright is claimed.
This code is in the public domain; do with it what you wish.

Equivalent code is available from RSA Data Security, Inc.
This code has been tested against that, and is equivalent,
except that you don't need to include two pages of legalese
with every copy.

To compute the message digest of a chunk of bytes, declare an
MD5Context structure, pass it to MD5Init, call MD5Update as
needed on buffers full of bytes, and then call MD5Final, which
will fill a supplied 16-byte array with the digest.

*/

//#define MD5F1(x, y, z) (x & y | ~x & z)
#define MD5F1(x, y, z) (z ^ (x & (y ^ z)))
#define MD5F2(x, y, z) MD5F1(z, x, y)
#define MD5F3(x, y, z) (x ^ y ^ z)
#define MD5F4(x, y, z) (y ^ (x | ~z))

#define MD5STEP(f, w, x, y, z, data, s) ( w += f(x, y, z) + data,  w = w<<s | w>>(32-s),  w += x )

#define SUPERFAST_GET16BITS(d) (*((const hUInt16 *) (d)))

namespace HorseRadish
{

void Hashing::initMD5(DataMD5 * const dataMD5)
{
	dataMD5->state[0] = 0x67452301;
    dataMD5->state[1] = 0xefcdab89;
    dataMD5->state[2] = 0x98badcfe;
    dataMD5->state[3] = 0x10325476;
    dataMD5->bits[0] = 0;
    dataMD5->bits[1] = 0;
}
void Hashing::updateMD5(DataMD5 * const dataMD5, const unsigned char *buffer, unsigned int bufferSize)
{
	unsigned int t;

    /* Update bitcount */
    t = dataMD5->bits[0];
	if ( ( dataMD5->bits[0] = t + ( (unsigned int) bufferSize << 3 ) ) < t )
        dataMD5->bits[1]++;
    dataMD5->bits[1] += bufferSize >> 29;
    t = ( t >> 3 ) & 0x3f;        /* Bytes already in shsInfo->data */

    /* Handle any leading odd-sized chunks */
    if (t)
		{
        unsigned char *p = (unsigned char *) dataMD5->in + t;

        t = 64 - t;
        if ( bufferSize < t )
			{
            memcpy(p, buffer, bufferSize );
            return;
			}

        memcpy( p, buffer, t );
        transformMD5( dataMD5->state, (unsigned int *) dataMD5->in );
        buffer += t;
        bufferSize -= t;
		}

    /* Process data in 64-byte chunks */
    while( bufferSize >= 64 )
		{
        memcpy( dataMD5->in, buffer, 64 );
        transformMD5( dataMD5->state, (unsigned int *) dataMD5->in );
        buffer += 64;
        bufferSize -= 64;
		}

    /* Handle any remaining bytes of data. */
    memcpy( dataMD5->in, buffer, bufferSize );
}
void Hashing::transformMD5(unsigned int state[4], unsigned int const in[16])
{
	unsigned int a, b, c, d;

    a = state[0];
    b = state[1];
    c = state[2];
    d = state[3];

    MD5STEP(MD5F1, a, b, c, d, in[0] + 0xd76aa478, 7);
    MD5STEP(MD5F1, d, a, b, c, in[1] + 0xe8c7b756, 12);
    MD5STEP(MD5F1, c, d, a, b, in[2] + 0x242070db, 17);
    MD5STEP(MD5F1, b, c, d, a, in[3] + 0xc1bdceee, 22);
    MD5STEP(MD5F1, a, b, c, d, in[4] + 0xf57c0faf, 7);
    MD5STEP(MD5F1, d, a, b, c, in[5] + 0x4787c62a, 12);
    MD5STEP(MD5F1, c, d, a, b, in[6] + 0xa8304613, 17);
    MD5STEP(MD5F1, b, c, d, a, in[7] + 0xfd469501, 22);
    MD5STEP(MD5F1, a, b, c, d, in[8] + 0x698098d8, 7);
    MD5STEP(MD5F1, d, a, b, c, in[9] + 0x8b44f7af, 12);
    MD5STEP(MD5F1, c, d, a, b, in[10] + 0xffff5bb1, 17);
    MD5STEP(MD5F1, b, c, d, a, in[11] + 0x895cd7be, 22);
    MD5STEP(MD5F1, a, b, c, d, in[12] + 0x6b901122, 7);
    MD5STEP(MD5F1, d, a, b, c, in[13] + 0xfd987193, 12);
    MD5STEP(MD5F1, c, d, a, b, in[14] + 0xa679438e, 17);
    MD5STEP(MD5F1, b, c, d, a, in[15] + 0x49b40821, 22);

    MD5STEP(MD5F2, a, b, c, d, in[1] + 0xf61e2562, 5);
    MD5STEP(MD5F2, d, a, b, c, in[6] + 0xc040b340, 9);
    MD5STEP(MD5F2, c, d, a, b, in[11] + 0x265e5a51, 14);
    MD5STEP(MD5F2, b, c, d, a, in[0] + 0xe9b6c7aa, 20);
    MD5STEP(MD5F2, a, b, c, d, in[5] + 0xd62f105d, 5);
    MD5STEP(MD5F2, d, a, b, c, in[10] + 0x02441453, 9);
    MD5STEP(MD5F2, c, d, a, b, in[15] + 0xd8a1e681, 14);
    MD5STEP(MD5F2, b, c, d, a, in[4] + 0xe7d3fbc8, 20);
    MD5STEP(MD5F2, a, b, c, d, in[9] + 0x21e1cde6, 5);
    MD5STEP(MD5F2, d, a, b, c, in[14] + 0xc33707d6, 9);
    MD5STEP(MD5F2, c, d, a, b, in[3] + 0xf4d50d87, 14);
    MD5STEP(MD5F2, b, c, d, a, in[8] + 0x455a14ed, 20);
    MD5STEP(MD5F2, a, b, c, d, in[13] + 0xa9e3e905, 5);
    MD5STEP(MD5F2, d, a, b, c, in[2] + 0xfcefa3f8, 9);
    MD5STEP(MD5F2, c, d, a, b, in[7] + 0x676f02d9, 14);
    MD5STEP(MD5F2, b, c, d, a, in[12] + 0x8d2a4c8a, 20);

    MD5STEP(MD5F3, a, b, c, d, in[5] + 0xfffa3942, 4);
    MD5STEP(MD5F3, d, a, b, c, in[8] + 0x8771f681, 11);
    MD5STEP(MD5F3, c, d, a, b, in[11] + 0x6d9d6122, 16);
    MD5STEP(MD5F3, b, c, d, a, in[14] + 0xfde5380c, 23);
    MD5STEP(MD5F3, a, b, c, d, in[1] + 0xa4beea44, 4);
    MD5STEP(MD5F3, d, a, b, c, in[4] + 0x4bdecfa9, 11);
    MD5STEP(MD5F3, c, d, a, b, in[7] + 0xf6bb4b60, 16);
    MD5STEP(MD5F3, b, c, d, a, in[10] + 0xbebfbc70, 23);
    MD5STEP(MD5F3, a, b, c, d, in[13] + 0x289b7ec6, 4);
    MD5STEP(MD5F3, d, a, b, c, in[0] + 0xeaa127fa, 11);
    MD5STEP(MD5F3, c, d, a, b, in[3] + 0xd4ef3085, 16);
    MD5STEP(MD5F3, b, c, d, a, in[6] + 0x04881d05, 23);
    MD5STEP(MD5F3, a, b, c, d, in[9] + 0xd9d4d039, 4);
    MD5STEP(MD5F3, d, a, b, c, in[12] + 0xe6db99e5, 11);
    MD5STEP(MD5F3, c, d, a, b, in[15] + 0x1fa27cf8, 16);
    MD5STEP(MD5F3, b, c, d, a, in[2] + 0xc4ac5665, 23);

    MD5STEP(MD5F4, a, b, c, d, in[0] + 0xf4292244, 6);
    MD5STEP(MD5F4, d, a, b, c, in[7] + 0x432aff97, 10);
    MD5STEP(MD5F4, c, d, a, b, in[14] + 0xab9423a7, 15);
    MD5STEP(MD5F4, b, c, d, a, in[5] + 0xfc93a039, 21);
    MD5STEP(MD5F4, a, b, c, d, in[12] + 0x655b59c3, 6);
    MD5STEP(MD5F4, d, a, b, c, in[3] + 0x8f0ccc92, 10);
    MD5STEP(MD5F4, c, d, a, b, in[10] + 0xffeff47d, 15);
    MD5STEP(MD5F4, b, c, d, a, in[1] + 0x85845dd1, 21);
    MD5STEP(MD5F4, a, b, c, d, in[8] + 0x6fa87e4f, 6);
    MD5STEP(MD5F4, d, a, b, c, in[15] + 0xfe2ce6e0, 10);
    MD5STEP(MD5F4, c, d, a, b, in[6] + 0xa3014314, 15);
    MD5STEP(MD5F4, b, c, d, a, in[13] + 0x4e0811a1, 21);
    MD5STEP(MD5F4, a, b, c, d, in[4] + 0xf7537e82, 6);
    MD5STEP(MD5F4, d, a, b, c, in[11] + 0xbd3af235, 10);
    MD5STEP(MD5F4, c, d, a, b, in[2] + 0x2ad7d2bb, 15);
    MD5STEP(MD5F4, b, c, d, a, in[9] + 0xeb86d391, 21);

    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
}
void Hashing::finishMD5(DataMD5 * const dataMD5, unsigned char digest[16])
{
	unsigned count;
    unsigned char *p;

    /* Compute number of bytes mod 64 */
    count = ( dataMD5->bits[0] >> 3 ) & 0x3F;

    /* Set the first char of padding to 0x80.  This is safe since there is
       always at least one byte free */
    p = dataMD5->in + count;
    *p++ = 0x80;

    /* Bytes of padding needed to make 64 bytes */
    count = 64 - 1 - count;

    /* Pad out to 56 mod 64 */
    if ( count < 8 )
		{
        /* Two lots of padding:  Pad the first block to 64 bytes */
        memset( p, 0, count );
        transformMD5( dataMD5->state, (unsigned int *) dataMD5->in );

        /* Now fill the next block with 56 bytes */
        memset( dataMD5->in, 0, 56 );
		}
	else
		{
        /* Pad block to 56 bytes */
        memset( p, 0, count - 8 );
		}

    /* Append length in bits and transform */
    ((unsigned int *) dataMD5->in)[14] = dataMD5->bits[0];
    ((unsigned int *) dataMD5->in)[15] = dataMD5->bits[1];

    transformMD5( dataMD5->state, (unsigned int *) dataMD5->in );
    memcpy( digest, dataMD5->state, 16 );
    memset( dataMD5, 0, sizeof( dataMD5 ) );        /* In case it's sensitive */
}



/* ================ sha1.c ================ */
/*
SHA-1 in C
By Steve Reid <steve@edmweb.com>
100% Public Domain

Test Vectors (from FIPS PUB 180-1)
"abc"
  A9993E36 4706816A BA3E2571 7850C26C 9CD0D89D
"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"
  84983E44 1C3BD26E BAAE4AA1 F95129E5 E54670F1
A million repetitions of "a"
  34AA973C D4C4DAA4 F61EEB2B DBAD2731 6534016F
*/

/* #define LITTLE_ENDIAN * This should be #define'd already, if true. */
/* #define SHA1HANDSOFF * Copies data before messing with it. */

#define SHA1HANDSOFF

#include <stdio.h>
#include <string.h>
#include <sys/types.h>	/* for u_int*_t */
#if defined(__sun)
	#include "solarisfixes.h"
#endif

#ifndef BYTE_ORDER
	#if (BSD >= 199103)
		# include <machine/endian.h>
	#else
		#if defined(linux) || defined(__linux__)
			# include <endian.h>
		#else
			#define	LITTLE_ENDIAN	1234	/* least-significant byte first (vax, pc) */
			#define	BIG_ENDIAN	4321	/* most-significant byte first (IBM, net) */
			#define	PDP_ENDIAN	3412	/* LSB first in word, MSW first in long (pdp)*/

			#if defined(vax) || defined(ns32000) || defined(sun386) || defined(__i386__) || \
				defined(MIPSEL) || defined(_MIPSEL) || defined(BIT_ZERO_ON_RIGHT) || \
				defined(__alpha__) || defined(__alpha)
				#define BYTE_ORDER	LITTLE_ENDIAN
			#endif

			#if defined(sel) || defined(pyr) || defined(mc68000) || defined(sparc) || \
				defined(is68k) || defined(tahoe) || defined(ibm032) || defined(ibm370) || \
				defined(MIPSEB) || defined(_MIPSEB) || defined(_IBMR2) || defined(DGUX) ||\
				defined(apollo) || defined(__convex__) || defined(_CRAY) || \
				defined(__hppa) || defined(__hp9000) || \
				defined(__hp9000s300) || defined(__hp9000s700) || \
				defined (BIT_ZERO_ON_LEFT) || defined(m68k) || defined(__sparc)

				#define BYTE_ORDER	BIG_ENDIAN
			#endif
		#endif /* linux */
	#endif /* BSD */
#endif /* BYTE_ORDER */

#if defined(__BYTE_ORDER) && !defined(BYTE_ORDER)
	#if (__BYTE_ORDER == __LITTLE_ENDIAN)
		#define BYTE_ORDER LITTLE_ENDIAN
	#else
		#define BYTE_ORDER BIG_ENDIAN
	#endif
#endif

#ifdef _MSC_VER
    #ifndef BYTE_ORDER
		#define BYTE_ORDER LITTLE_ENDIAN
    #endif
#endif

#if !defined(BYTE_ORDER) || (BYTE_ORDER != BIG_ENDIAN && BYTE_ORDER != LITTLE_ENDIAN && BYTE_ORDER != PDP_ENDIAN)
	/* you must determine what the correct bit order is for
	 * your compiler - the next line is an intentional error
	 * which will force your compiles to bomb until you fix
	 * the above macros.
	 */
	#error "Undefined or invalid BYTE_ORDER"
#endif

#define rol(value, bits) (((value) << (bits)) | ((value) >> (32 - (bits))))

/* blk0() and blk() perform the initial expand. */
/* I got the idea of expanding during the round function from SSLeay */
#if BYTE_ORDER == LITTLE_ENDIAN
	#define blk0(i) (block->l[i] = (rol(block->l[i],24)&0xFF00FF00)|(rol(block->l[i],8)&0x00FF00FF))
#elif BYTE_ORDER == BIG_ENDIAN
	#define blk0(i) block->l[i]
#else
	#error "Endianness not defined!"
#endif

#define blk(i) (block->l[i&15] = rol(block->l[(i+13)&15]^block->l[(i+8)&15]^block->l[(i+2)&15]^block->l[i&15],1))

/* (R0+R1), R2, R3, R4 are the different operations used in SHA1 */
#define R0(v,w,x,y,z,i) z+=((w&(x^y))^y)+blk0(i)+0x5A827999+rol(v,5);w=rol(w,30);
#define R1(v,w,x,y,z,i) z+=((w&(x^y))^y)+blk(i)+0x5A827999+rol(v,5);w=rol(w,30);
#define R2(v,w,x,y,z,i) z+=(w^x^y)+blk(i)+0x6ED9EBA1+rol(v,5);w=rol(w,30);
#define R3(v,w,x,y,z,i) z+=(((w|x)&y)|(w&x))+blk(i)+0x8F1BBCDC+rol(v,5);w=rol(w,30);
#define R4(v,w,x,y,z,i) z+=(w^x^y)+blk(i)+0xCA62C1D6+rol(v,5);w=rol(w,30);

void Hashing::transformSHA1(unsigned int state[5], const unsigned char buffer[64])
{
	unsigned int a, b, c, d, e;
	typedef union
	{
		unsigned char c[64];
		unsigned int l[16];
	} CHAR64LONG16;

#ifdef SHA1HANDSOFF
	CHAR64LONG16 block[1];  /* use array to appear as a pointer */
	memcpy(block, buffer, 64);
#else
	/* The following had better never be used because it causes the
		* pointer-to-const buffer to be cast into a pointer to non-const.
		* And the result is written through.  I threw a "const" in, hoping
		* this will cause a diagnostic.
		*/
	CHAR64LONG16* block = (const CHAR64LONG16*)buffer;
#endif

	/* Copy context->state[] to working vars */
	a = state[0];
	b = state[1];
	c = state[2];
	d = state[3];
	e = state[4];
	/* 4 rounds of 20 operations each. Loop unrolled. */
	R0(a,b,c,d,e, 0); R0(e,a,b,c,d, 1); R0(d,e,a,b,c, 2); R0(c,d,e,a,b, 3);
	R0(b,c,d,e,a, 4); R0(a,b,c,d,e, 5); R0(e,a,b,c,d, 6); R0(d,e,a,b,c, 7);
	R0(c,d,e,a,b, 8); R0(b,c,d,e,a, 9); R0(a,b,c,d,e,10); R0(e,a,b,c,d,11);
	R0(d,e,a,b,c,12); R0(c,d,e,a,b,13); R0(b,c,d,e,a,14); R0(a,b,c,d,e,15);
	R1(e,a,b,c,d,16); R1(d,e,a,b,c,17); R1(c,d,e,a,b,18); R1(b,c,d,e,a,19);
	R2(a,b,c,d,e,20); R2(e,a,b,c,d,21); R2(d,e,a,b,c,22); R2(c,d,e,a,b,23);
	R2(b,c,d,e,a,24); R2(a,b,c,d,e,25); R2(e,a,b,c,d,26); R2(d,e,a,b,c,27);
	R2(c,d,e,a,b,28); R2(b,c,d,e,a,29); R2(a,b,c,d,e,30); R2(e,a,b,c,d,31);
	R2(d,e,a,b,c,32); R2(c,d,e,a,b,33); R2(b,c,d,e,a,34); R2(a,b,c,d,e,35);
	R2(e,a,b,c,d,36); R2(d,e,a,b,c,37); R2(c,d,e,a,b,38); R2(b,c,d,e,a,39);
	R3(a,b,c,d,e,40); R3(e,a,b,c,d,41); R3(d,e,a,b,c,42); R3(c,d,e,a,b,43);
	R3(b,c,d,e,a,44); R3(a,b,c,d,e,45); R3(e,a,b,c,d,46); R3(d,e,a,b,c,47);
	R3(c,d,e,a,b,48); R3(b,c,d,e,a,49); R3(a,b,c,d,e,50); R3(e,a,b,c,d,51);
	R3(d,e,a,b,c,52); R3(c,d,e,a,b,53); R3(b,c,d,e,a,54); R3(a,b,c,d,e,55);
	R3(e,a,b,c,d,56); R3(d,e,a,b,c,57); R3(c,d,e,a,b,58); R3(b,c,d,e,a,59);
	R4(a,b,c,d,e,60); R4(e,a,b,c,d,61); R4(d,e,a,b,c,62); R4(c,d,e,a,b,63);
	R4(b,c,d,e,a,64); R4(a,b,c,d,e,65); R4(e,a,b,c,d,66); R4(d,e,a,b,c,67);
	R4(c,d,e,a,b,68); R4(b,c,d,e,a,69); R4(a,b,c,d,e,70); R4(e,a,b,c,d,71);
	R4(d,e,a,b,c,72); R4(c,d,e,a,b,73); R4(b,c,d,e,a,74); R4(a,b,c,d,e,75);
	R4(e,a,b,c,d,76); R4(d,e,a,b,c,77); R4(c,d,e,a,b,78); R4(b,c,d,e,a,79);
	/* Add the working vars back into context.state[] */
	state[0] += a;
	state[1] += b;
	state[2] += c;
	state[3] += d;
	state[4] += e;
	/* Wipe variables */
	a = b = c = d = e = 0;
	#ifdef SHA1HANDSOFF
	memset(block, '\0', sizeof(block));
	#endif
}
void Hashing::initSHA1(DataSHA1 * const context)
{
	context->state[0] = 0x67452301;
	context->state[1] = 0xEFCDAB89;
	context->state[2] = 0x98BADCFE;
	context->state[3] = 0x10325476;
	context->state[4] = 0xC3D2E1F0;
	context->count[0] = context->count[1] = 0;
}
void Hashing::updateSHA1(DataSHA1 * const context, const unsigned char* data, unsigned int len)
{
	unsigned int i;
	unsigned int j;

	j = context->count[0];
	if ((context->count[0] += len << 3) < j)
		context->count[1]++;
	context->count[1] += (len>>29);
	j = (j >> 3) & 63;
	if ((j + len) > 63) 
	{
		memcpy(&context->buffer[j], data, (i = 64-j));
		Hashing::transformSHA1(context->state, context->buffer);
		for ( ; i + 63 < len; i += 64)
			Hashing::transformSHA1(context->state, &data[i]);
		j = 0;
	}
	else
		i = 0;

	memcpy(&context->buffer[j], &data[i], len - i);
}
void Hashing::finishSHA1(unsigned char digest[20], DataSHA1 * const context)
{
	unsigned i;
	unsigned char finalcount[8];
	unsigned char c;

#if 0	
	/* untested "improvement" by DHR */
	/* Convert context->count to a sequence of bytes
	* in finalcount.  Second element first, but
	* big-endian order within element.
	* But we do it all backwards.
	*/
	unsigned char *fcp = &finalcount[8];

	for (i = 0; i < 2; i++)
	{
	u_int32_t t = context->count[i];
	int j;

	for (j = 0; j < 4; t >>= 8, j++)
	*--fcp = (unsigned char) t
	}
#else
	for (i = 0; i < 8; i++)
		finalcount[i] = (unsigned char)((context->count[(i >= 4 ? 0 : 1)] >> ((3-(i & 3)) * 8) ) & 255);  /* Endian independent */
#endif

	c = 0200;
	Hashing::updateSHA1(context, &c, 1);
	while ((context->count[0] & 504) != 448) 
	{
		c = 0000;
		Hashing::updateSHA1(context, &c, 1);
	}

	Hashing::updateSHA1(context, finalcount, 8);  /* Should cause a SHA1Transform() */
	for (i = 0; i < 20; i++)
		digest[i] = (unsigned char)((context->state[i>>2] >> ((3-(i & 3)) * 8) ) & 255);

	/* Wipe variables */
	memset(context, '\0', sizeof(*context));
	memset(&finalcount, '\0', sizeof(finalcount));
}

unsigned char Hashing::CalculateCRC8(const void * const buffer, unsigned int bufferSize)
{
	unsigned char crc;

	//verificar parametros
	if ((buffer == nullptr) || (bufferSize == 0))
		return 0;

	//inicializar o cálculo
	crc = 0x0000;

	//preciso de andar com o buffer
	const unsigned char *bufferWalker = (const unsigned char *)buffer;

	//assim se constrói o CRC
	while(bufferSize--)
		crc = crctable8[crc ^ *bufferWalker++];

	//finalizar o calculo
	crc ^= 0x0000;
	return crc;
}

unsigned short Hashing::CalculateCRC16(const void * const buffer, unsigned int bufferSize)
{
	unsigned short crc;

	//verificar parametros
	if ((buffer == nullptr) || (bufferSize == 0))
		return 0;

	//inicializar o cálculo
	crc = 0xFFFF;

	//preciso de andar com o buffer
	const unsigned char *bufferWalker = (const unsigned char *)buffer;

	//assim se constrói o CRC
	while(bufferSize--)
		crc = ( crc << 8 ) ^ crctable16[ ( crc >> 8 ) ^ *bufferWalker++ ];

	//finalizar o calculo
	crc ^= 0x0000;
	return crc;
}

unsigned long Hashing::CalculateCRC32(const void * const buffer, unsigned int bufferSize)
{
	unsigned long crc;

	//verificar parametros
	if ((buffer == nullptr) || (bufferSize == 0))
		return 0;

	//inicializar o cálculo
	crc = 0xffffffffL;

	//preciso de andar com o buffer
	const unsigned char *bufferWalker = (const unsigned char *)buffer;

	//assim se constrói o CRC
	while(bufferSize--)
		crc = crctable32[ ( crc ^ ( *bufferWalker++ ) ) & 0xff ] ^ ( crc >> 8 );

	//finalizar o calculo
	crc ^= 0xffffffffL;
	return crc;
}

void Hashing::CalculateMD5(const void * const buffer, const unsigned int bufferSize, hData128 * const hash)
{
	Hashing::DataMD5 dataMD5;

	//verificar parametros
	if ((hash==nullptr) || (buffer==nullptr) || (bufferSize==0))
		return;

	//basta fazer isto
	Hashing::initMD5(&dataMD5);
	Hashing::updateMD5(&dataMD5, (unsigned char *)buffer, bufferSize);
	Hashing::finishMD5(&dataMD5, hash->ui8);
}

void Hashing::CalculateSHA1(const void * const buffer, const unsigned int bufferSize, hData160 * const hash)
{
	Hashing::DataSHA1 dataSHA1;

	//verificar parametros
	if ((hash==nullptr) || (buffer==nullptr) || (bufferSize==0))
		return;

	//basta fazer isto
	Hashing::initSHA1(&dataSHA1);
	Hashing::updateSHA1(&dataSHA1, (unsigned char *)buffer, bufferSize);
	Hashing::finishSHA1(hash->ui8, &dataSHA1);
}

unsigned long Hashing::CalculateMD5Short(const void * const buffer, const unsigned int bufferSize)
{
	Hashing::DataMD5 dataMD5;
	unsigned long digest[4];

	//verificar parametros
	if ((buffer==nullptr) || (bufferSize==0))
		return 0;

	//calculo o MD5
	Hashing::initMD5(&dataMD5);
	Hashing::updateMD5(&dataMD5, (unsigned char *)buffer, bufferSize);
	Hashing::finishMD5(&dataMD5, (unsigned char *)digest);

	//o MD5 mais pequeno é assim
	return (digest[0] ^ digest[1] ^ digest[2] ^ digest[3]);
}

unsigned int Hashing::SuperFastHash(const void * const buffer, unsigned int bufferSize)
{
	hUInt32 hash, tmp;
	int rem;

	//verifico isto
	if ((buffer == nullptr) || (bufferSize <= 0))
		return 0;

	//iniciar coisas
	hash = bufferSize;
	rem = bufferSize & 3;
	bufferSize >>= 2;

	//preciso de andar com o buffer
	const unsigned char *bufferWalker = (const unsigned char *)buffer;

	/* Main loop */
	for (;bufferSize > 0; bufferSize--)
	{
		hash  += SUPERFAST_GET16BITS (bufferWalker);
		tmp    = (SUPERFAST_GET16BITS (bufferWalker+2) << 11) ^ hash;
		hash   = (hash << 16) ^ tmp;
		bufferWalker  += 2*sizeof (hUInt16);
		hash  += hash >> 11;
	}

	/* Handle end cases */
	switch (rem)
	{
		case 3:
			hash += SUPERFAST_GET16BITS (bufferWalker);
			hash ^= hash << 16;
			hash ^= bufferWalker[sizeof (hUInt16)] << 18;
			hash += hash >> 11;
			break;
		case 2:
			hash += SUPERFAST_GET16BITS (bufferWalker);
			hash ^= hash << 11;
			hash += hash >> 17;
			break;
		case 1:
			hash += *bufferWalker;
			hash ^= hash << 10;
			hash += hash >> 1;
	}

	/* Force "avalanching" of final 127 bits */
	hash ^= hash << 3;
	hash += hash >> 5;
	hash ^= hash << 4;
	hash += hash >> 17;
	hash ^= hash << 25;
	hash += hash >> 6;
	return hash;
}

const unsigned char Hashing::crctable8[256] = {
	0x00, 0x07, 0x0E, 0x09, 0x1C, 0x1B, 0x12, 0x15, 0x38, 0x3F, 0x36, 0x31, 0x24, 0x23, 0x2A, 0x2D,
	0x70, 0x77, 0x7E, 0x79, 0x6C, 0x6B, 0x62, 0x65, 0x48, 0x4F, 0x46, 0x41, 0x54, 0x53, 0x5A, 0x5D,
	0xE0, 0xE7, 0xEE, 0xE9, 0xFC, 0xFB, 0xF2, 0xF5, 0xD8, 0xDF, 0xD6, 0xD1, 0xC4, 0xC3, 0xCA, 0xCD,
	0x90, 0x97, 0x9E, 0x99, 0x8C, 0x8B, 0x82, 0x85, 0xA8, 0xAF, 0xA6, 0xA1, 0xB4, 0xB3, 0xBA, 0xBD,
	0xC7, 0xC0, 0xC9, 0xCE, 0xDB, 0xDC, 0xD5, 0xD2, 0xFF, 0xF8, 0xF1, 0xF6, 0xE3, 0xE4, 0xED, 0xEA,
	0xB7, 0xB0, 0xB9, 0xBE, 0xAB, 0xAC, 0xA5, 0xA2, 0x8F, 0x88, 0x81, 0x86, 0x93, 0x94, 0x9D, 0x9A,
	0x27, 0x20, 0x29, 0x2E, 0x3B, 0x3C, 0x35, 0x32, 0x1F, 0x18, 0x11, 0x16, 0x03, 0x04, 0x0D, 0x0A,
	0x57, 0x50, 0x59, 0x5E, 0x4B, 0x4C, 0x45, 0x42, 0x6F, 0x68, 0x61, 0x66, 0x73, 0x74, 0x7D, 0x7A,
	0x89, 0x8E, 0x87, 0x80, 0x95, 0x92, 0x9B, 0x9C, 0xB1, 0xB6, 0xBF, 0xB8, 0xAD, 0xAA, 0xA3, 0xA4,
	0xF9, 0xFE, 0xF7, 0xF0, 0xE5, 0xE2, 0xEB, 0xEC, 0xC1, 0xC6, 0xCF, 0xC8, 0xDD, 0xDA, 0xD3, 0xD4,
	0x69, 0x6E, 0x67, 0x60, 0x75, 0x72, 0x7B, 0x7C, 0x51, 0x56, 0x5F, 0x58, 0x4D, 0x4A, 0x43, 0x44,
	0x19, 0x1E, 0x17, 0x10, 0x05, 0x02, 0x0B, 0x0C, 0x21, 0x26, 0x2F, 0x28, 0x3D, 0x3A, 0x33, 0x34,
	0x4E, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5C, 0x5B, 0x76, 0x71, 0x78, 0x7F, 0x6A, 0x6D, 0x64, 0x63,
	0x3E, 0x39, 0x30, 0x37, 0x22, 0x25, 0x2C, 0x2B, 0x06, 0x01, 0x08, 0x0F, 0x1A, 0x1D, 0x14, 0x13,
	0xAE, 0xA9, 0xA0, 0xA7, 0xB2, 0xB5, 0xBC, 0xBB, 0x96, 0x91, 0x98, 0x9F, 0x8A, 0x8D, 0x84, 0x83,
	0xDE, 0xD9, 0xD0, 0xD7, 0xC2, 0xC5, 0xCC, 0xCB, 0xE6, 0xE1, 0xE8, 0xEF, 0xFA, 0xFD, 0xF4, 0xF3
};

const unsigned short Hashing::crctable16[256] = {
	0x0000,	0x1021,	0x2042,	0x3063,	0x4084,	0x50a5,	0x60c6,	0x70e7, 0x8108,	0x9129,	0xa14a,	0xb16b,	0xc18c,	0xd1ad,	0xe1ce,	0xf1ef,
	0x1231,	0x0210,	0x3273,	0x2252,	0x52b5,	0x4294,	0x72f7,	0x62d6, 0x9339,	0x8318,	0xb37b,	0xa35a,	0xd3bd,	0xc39c,	0xf3ff,	0xe3de,
	0x2462,	0x3443,	0x0420,	0x1401,	0x64e6,	0x74c7,	0x44a4,	0x5485, 0xa56a,	0xb54b,	0x8528,	0x9509,	0xe5ee,	0xf5cf,	0xc5ac,	0xd58d,
	0x3653,	0x2672,	0x1611,	0x0630,	0x76d7,	0x66f6,	0x5695,	0x46b4, 0xb75b,	0xa77a,	0x9719,	0x8738,	0xf7df,	0xe7fe,	0xd79d,	0xc7bc,
	0x48c4,	0x58e5,	0x6886,	0x78a7,	0x0840,	0x1861,	0x2802,	0x3823, 0xc9cc,	0xd9ed,	0xe98e,	0xf9af,	0x8948,	0x9969,	0xa90a,	0xb92b,
	0x5af5,	0x4ad4,	0x7ab7,	0x6a96,	0x1a71,	0x0a50,	0x3a33,	0x2a12, 0xdbfd,	0xcbdc,	0xfbbf,	0xeb9e,	0x9b79,	0x8b58,	0xbb3b,	0xab1a,
	0x6ca6,	0x7c87,	0x4ce4,	0x5cc5,	0x2c22,	0x3c03,	0x0c60,	0x1c41, 0xedae,	0xfd8f,	0xcdec,	0xddcd,	0xad2a,	0xbd0b,	0x8d68,	0x9d49,
	0x7e97,	0x6eb6,	0x5ed5,	0x4ef4,	0x3e13,	0x2e32,	0x1e51,	0x0e70, 0xff9f,	0xefbe,	0xdfdd,	0xcffc,	0xbf1b,	0xaf3a,	0x9f59,	0x8f78,
	0x9188,	0x81a9,	0xb1ca,	0xa1eb,	0xd10c,	0xc12d,	0xf14e,	0xe16f, 0x1080,	0x00a1,	0x30c2,	0x20e3,	0x5004,	0x4025,	0x7046,	0x6067,
	0x83b9,	0x9398,	0xa3fb,	0xb3da,	0xc33d,	0xd31c,	0xe37f,	0xf35e, 0x02b1,	0x1290,	0x22f3,	0x32d2,	0x4235,	0x5214,	0x6277,	0x7256,
	0xb5ea,	0xa5cb,	0x95a8,	0x8589,	0xf56e,	0xe54f,	0xd52c,	0xc50d, 0x34e2,	0x24c3,	0x14a0,	0x0481,	0x7466,	0x6447,	0x5424,	0x4405,
	0xa7db,	0xb7fa,	0x8799,	0x97b8,	0xe75f,	0xf77e,	0xc71d,	0xd73c, 0x26d3,	0x36f2,	0x0691,	0x16b0,	0x6657,	0x7676,	0x4615,	0x5634,
	0xd94c,	0xc96d,	0xf90e,	0xe92f,	0x99c8,	0x89e9,	0xb98a,	0xa9ab, 0x5844,	0x4865,	0x7806,	0x6827,	0x18c0,	0x08e1,	0x3882,	0x28a3,
	0xcb7d,	0xdb5c,	0xeb3f,	0xfb1e,	0x8bf9,	0x9bd8,	0xabbb,	0xbb9a, 0x4a75,	0x5a54,	0x6a37,	0x7a16,	0x0af1,	0x1ad0,	0x2ab3,	0x3a92,
	0xfd2e,	0xed0f,	0xdd6c,	0xcd4d,	0xbdaa,	0xad8b,	0x9de8,	0x8dc9, 0x7c26,	0x6c07,	0x5c64,	0x4c45,	0x3ca2,	0x2c83,	0x1ce0,	0x0cc1,
	0xef1f,	0xff3e,	0xcf5d,	0xdf7c,	0xaf9b,	0xbfba,	0x8fd9,	0x9ff8, 0x6e17,	0x7e36,	0x4e55,	0x5e74,	0x2e93,	0x3eb2,	0x0ed1,	0x1ef0
};

const unsigned long Hashing::crctable32[256] = {
	0x00000000L, 0x77073096L, 0xee0e612cL, 0x990951baL, 0x076dc419L, 0x706af48fL, 0xe963a535L, 0x9e6495a3L,
	0x0edb8832L, 0x79dcb8a4L, 0xe0d5e91eL, 0x97d2d988L, 0x09b64c2bL, 0x7eb17cbdL, 0xe7b82d07L, 0x90bf1d91L,
	0x1db71064L, 0x6ab020f2L, 0xf3b97148L, 0x84be41deL, 0x1adad47dL, 0x6ddde4ebL, 0xf4d4b551L, 0x83d385c7L,
	0x136c9856L, 0x646ba8c0L, 0xfd62f97aL, 0x8a65c9ecL, 0x14015c4fL, 0x63066cd9L, 0xfa0f3d63L, 0x8d080df5L,
	0x3b6e20c8L, 0x4c69105eL, 0xd56041e4L, 0xa2677172L, 0x3c03e4d1L, 0x4b04d447L, 0xd20d85fdL, 0xa50ab56bL,
	0x35b5a8faL, 0x42b2986cL, 0xdbbbc9d6L, 0xacbcf940L, 0x32d86ce3L, 0x45df5c75L, 0xdcd60dcfL, 0xabd13d59L,
	0x26d930acL, 0x51de003aL, 0xc8d75180L, 0xbfd06116L, 0x21b4f4b5L, 0x56b3c423L, 0xcfba9599L, 0xb8bda50fL,
	0x2802b89eL, 0x5f058808L, 0xc60cd9b2L, 0xb10be924L, 0x2f6f7c87L, 0x58684c11L, 0xc1611dabL, 0xb6662d3dL,
	0x76dc4190L, 0x01db7106L, 0x98d220bcL, 0xefd5102aL, 0x71b18589L, 0x06b6b51fL, 0x9fbfe4a5L, 0xe8b8d433L,
	0x7807c9a2L, 0x0f00f934L, 0x9609a88eL, 0xe10e9818L, 0x7f6a0dbbL, 0x086d3d2dL, 0x91646c97L, 0xe6635c01L,
	0x6b6b51f4L, 0x1c6c6162L, 0x856530d8L, 0xf262004eL, 0x6c0695edL, 0x1b01a57bL, 0x8208f4c1L, 0xf50fc457L,
	0x65b0d9c6L, 0x12b7e950L, 0x8bbeb8eaL, 0xfcb9887cL, 0x62dd1ddfL, 0x15da2d49L, 0x8cd37cf3L, 0xfbd44c65L,
	0x4db26158L, 0x3ab551ceL, 0xa3bc0074L, 0xd4bb30e2L, 0x4adfa541L, 0x3dd895d7L, 0xa4d1c46dL, 0xd3d6f4fbL,
	0x4369e96aL, 0x346ed9fcL, 0xad678846L, 0xda60b8d0L, 0x44042d73L, 0x33031de5L, 0xaa0a4c5fL, 0xdd0d7cc9L,
	0x5005713cL, 0x270241aaL, 0xbe0b1010L, 0xc90c2086L, 0x5768b525L, 0x206f85b3L, 0xb966d409L, 0xce61e49fL,
	0x5edef90eL, 0x29d9c998L, 0xb0d09822L, 0xc7d7a8b4L, 0x59b33d17L, 0x2eb40d81L, 0xb7bd5c3bL, 0xc0ba6cadL,
	0xedb88320L, 0x9abfb3b6L, 0x03b6e20cL, 0x74b1d29aL, 0xead54739L, 0x9dd277afL, 0x04db2615L, 0x73dc1683L,
	0xe3630b12L, 0x94643b84L, 0x0d6d6a3eL, 0x7a6a5aa8L, 0xe40ecf0bL, 0x9309ff9dL, 0x0a00ae27L, 0x7d079eb1L,
	0xf00f9344L, 0x8708a3d2L, 0x1e01f268L, 0x6906c2feL, 0xf762575dL, 0x806567cbL, 0x196c3671L, 0x6e6b06e7L,
	0xfed41b76L, 0x89d32be0L, 0x10da7a5aL, 0x67dd4accL, 0xf9b9df6fL, 0x8ebeeff9L, 0x17b7be43L, 0x60b08ed5L,
	0xd6d6a3e8L, 0xa1d1937eL, 0x38d8c2c4L, 0x4fdff252L, 0xd1bb67f1L, 0xa6bc5767L, 0x3fb506ddL, 0x48b2364bL,
	0xd80d2bdaL, 0xaf0a1b4cL, 0x36034af6L, 0x41047a60L, 0xdf60efc3L, 0xa867df55L, 0x316e8eefL, 0x4669be79L,
	0xcb61b38cL, 0xbc66831aL, 0x256fd2a0L, 0x5268e236L, 0xcc0c7795L, 0xbb0b4703L, 0x220216b9L, 0x5505262fL,
	0xc5ba3bbeL, 0xb2bd0b28L, 0x2bb45a92L, 0x5cb36a04L, 0xc2d7ffa7L, 0xb5d0cf31L, 0x2cd99e8bL, 0x5bdeae1dL,
	0x9b64c2b0L, 0xec63f226L, 0x756aa39cL, 0x026d930aL, 0x9c0906a9L, 0xeb0e363fL, 0x72076785L, 0x05005713L,
	0x95bf4a82L, 0xe2b87a14L, 0x7bb12baeL, 0x0cb61b38L, 0x92d28e9bL, 0xe5d5be0dL, 0x7cdcefb7L, 0x0bdbdf21L,
	0x86d3d2d4L, 0xf1d4e242L, 0x68ddb3f8L, 0x1fda836eL, 0x81be16cdL, 0xf6b9265bL, 0x6fb077e1L, 0x18b74777L,
	0x88085ae6L, 0xff0f6a70L, 0x66063bcaL, 0x11010b5cL, 0x8f659effL, 0xf862ae69L, 0x616bffd3L, 0x166ccf45L,
	0xa00ae278L, 0xd70dd2eeL, 0x4e048354L, 0x3903b3c2L, 0xa7672661L, 0xd06016f7L, 0x4969474dL, 0x3e6e77dbL,
	0xaed16a4aL, 0xd9d65adcL, 0x40df0b66L, 0x37d83bf0L, 0xa9bcae53L, 0xdebb9ec5L, 0x47b2cf7fL, 0x30b5ffe9L,
	0xbdbdf21cL, 0xcabac28aL, 0x53b39330L, 0x24b4a3a6L, 0xbad03605L, 0xcdd70693L, 0x54de5729L, 0x23d967bfL,
	0xb3667a2eL, 0xc4614ab8L, 0x5d681b02L, 0x2a6f2b94L, 0xb40bbe37L, 0xc30c8ea1L, 0x5a05df1bL, 0x2d02ef8dL
};

}//namespace HorseRadish