#include "Math.hpp"

//coisas auxiliares
union halfType{
	unsigned short bits;
	struct{
		unsigned long m : 10;
		unsigned long e : 5;
		unsigned long s : 1;
	}ieee;
};
union ieee_single {
	float f;
	struct{
		unsigned long m : 23;
		unsigned long e : 8;
		unsigned long s : 1;
		}ieee;
};

namespace HorseRadish
{

//§§§§§§§§§§§§§§§§§§§§
//§§§§§§ Math		§§
//§§§§§§§§§§§§§§§§§§§§
float Math::htof(const unsigned short &val)
{
	halfType h;
	ieee_single sng;

	h.bits = val;
	sng.ieee.s = h.ieee.s;

	// zero
	if ( (h.ieee.e==0) && (h.ieee.m==0) )
	{
		sng.ieee.m=0;
		sng.ieee.e=0;
		return sng.f;
	}

	// denorm -- denorm half will fit in non-denorm single
	if ( (h.ieee.e==0) && (h.ieee.m!=0) )
	{
		const float half_denorm = (1.0f/16384.0f); // 2^-14
		float mantissa = ((float)(h.ieee.m)) / 1024.0f;
		float sgn = (h.ieee.s)? -1.0f :1.0f;
		sng.f = sgn*mantissa*half_denorm;
		return sng.f;
	}

	// infinity
	if ( (h.ieee.e==31) && (h.ieee.m==0) )
	{
		sng.ieee.e = 0xff;
		sng.ieee.m = 0;
		return sng.f;
	}

	// NaN
	if ( (h.ieee.e==31) && (h.ieee.m!=0) )
	{
		sng.ieee.e = 0xff;
		sng.ieee.m = 1;
		return sng.f;
	}
	
	sng.ieee.e = h.ieee.e+112;
	sng.ieee.m = (h.ieee.m << 13);
	return sng.f;
}

unsigned short Math::ftoh(const float &val)
{
	ieee_single f;
	f.f = val;
	halfType h;

	h.ieee.s = f.ieee.s;

	// handle special cases

	const float half_denorm = (1.0f/16384.0f);

	// zero
	if ( (f.ieee.e==0) && (f.ieee.m==0) )
	{
		h.ieee.m = 0;
		h.ieee.e = 0;
		return h.bits;
	}

	// denorm -- denorm float maps to 0 half
	if ( (f.ieee.e==0) && (f.ieee.m!=0) )
	{
		h.ieee.m = 0;
		h.ieee.e = 0;
		return h.bits;
	}

	// infinity
	if ( (f.ieee.e==0xff) && (f.ieee.m==0) )
	{
		h.ieee.m = 0;
		h.ieee.e = 31;
		return h.bits;
	}

	// NaN
	if ( (f.ieee.e==0xff) && (f.ieee.m!=0) )
	{
		h.ieee.m = 1;
		h.ieee.e = 31;
		return h.bits;
	}

	int new_exp = f.ieee.e-127;
	if (new_exp<-24)
	{ 
		// this maps to 0
		h.ieee.m = 0;
		h.ieee.e = 0;
	}

	// this maps to a denorm
	if (new_exp<-14)
	{
		h.ieee.e = 0;
		unsigned int exp_val = (unsigned int) (-14 - new_exp); // 2^-exp_val
		switch (exp_val)
		{
			case 0: h.ieee.m = 0; break;
			case 1: h.ieee.m = 512 + (f.ieee.m>>14); break;
			case 2: h.ieee.m = 256 + (f.ieee.m>>15); break;
			case 3: h.ieee.m = 128 + (f.ieee.m>>16); break;
			case 4: h.ieee.m = 64 + (f.ieee.m>>17); break;
			case 5: h.ieee.m = 32 + (f.ieee.m>>18); break;
			case 6: h.ieee.m = 16 + (f.ieee.m>>19); break;
			case 7: h.ieee.m = 8 + (f.ieee.m>>20); break;
			case 8: h.ieee.m = 4 + (f.ieee.m>>21); break;
			case 9: h.ieee.m = 2 + (f.ieee.m>>22); break;
			case 10: h.ieee.m = 1; break;
		}
		return h.bits;
	}

	// map this value to infinity
	if (new_exp>15)
	{
		h.ieee.m = 0;
		h.ieee.e = 31;
		return h.bits;
	}
	
	h.ieee.e = new_exp+15;
	h.ieee.m = (f.ieee.m >> 13);
	return h.bits;
}

void Math::fMAD(float *buf, const unsigned int num, const float mulVal, const float addVal)
{
	int resto,leftOver;
	float *walker;
	__m128 do1,do2,do3,do4,mulReg,addReg;

	//preparo algumas coisas
	walker=buf;
	resto=((unsigned int)buf)%16;
	mulReg = _mm_load_ps1(&mulVal);
	addReg = _mm_load_ps1(&addVal);

	//verifico se posso alinhar algumas coisas
	if (resto==4 || resto==8 || resto==12 || resto==0)
	{
		//faço só o resto que falta para ficar com o ponteiro alinhado
		if (resto!=0)
		{
			resto=(16-resto)/4;
			while(resto>0)
			{
				*walker=(*walker)*mulVal+addVal;
				resto--;
				walker++;
			}
		}

		//basta agora copiar tudo
		leftOver=num;
		for(; leftOver>=16; leftOver-=16,walker+=16)
		{
			//leio
			do1 = _mm_load_ps(walker+0);
			do2 = _mm_load_ps(walker+4);
			do3 = _mm_load_ps(walker+8);
			do4 = _mm_load_ps(walker+12);

			//calculo tudo
			do1=_mm_add_ps(_mm_mul_ps(do1,mulReg), addReg);
			do2=_mm_add_ps(_mm_mul_ps(do2,mulReg), addReg);
			do3=_mm_add_ps(_mm_mul_ps(do3,mulReg), addReg);
			do4=_mm_add_ps(_mm_mul_ps(do4,mulReg), addReg);

			//gravo
			_mm_store_ps(walker+0,do1);
			_mm_store_ps(walker+4,do2);
			_mm_store_ps(walker+8,do3);
			_mm_store_ps(walker+12,do4);
		}
		
		for(; leftOver>0; leftOver--,walker++)
			*walker = (*walker)*mulVal+addVal;

		return;
	}

	//chegando aqui não pode haver alinhamento
	leftOver=num;
	for(; leftOver>=16; leftOver-=16,walker+=16)
	{
		//leio
		do1 = _mm_loadu_ps(walker+0);
		do2 = _mm_loadu_ps(walker+4);
		do3 = _mm_loadu_ps(walker+8);
		do4 = _mm_loadu_ps(walker+12);

		//calculo tudo
		do1=_mm_add_ps(_mm_mul_ps(do1,mulReg), addReg);
		do2=_mm_add_ps(_mm_mul_ps(do2,mulReg), addReg);
		do3=_mm_add_ps(_mm_mul_ps(do3,mulReg), addReg);
		do4=_mm_add_ps(_mm_mul_ps(do4,mulReg), addReg);

		//gravo
		_mm_storeu_ps(walker+0,do1);
		_mm_storeu_ps(walker+4,do2);
		_mm_storeu_ps(walker+8,do3);
		_mm_storeu_ps(walker+12,do4);
	}

	for(; leftOver>0; leftOver--,walker++)
		*walker = (*walker)*mulVal+addVal;
}

float Math::WaveEvalLinear(const float * const items, const int numItems, const float t)
{
	float normalized,start;
	int readStart;

	normalized=t-floorf(t);
	start=normalized * (numItems-1);

	readStart=ftoi(floorf(start));
	normalized=start-floorf(start);
	return items[readStart]*(1.0f-normalized) + items[readStart+1]*normalized;
}

float Math::WaveEvalSnap(const float * const items, const int numItems, const float t)
{
	float start;
	int readStart;

	start=(t-floorf(t)) * (numItems-1);
	readStart=ftoi(floorf(start));
	return items[readStart];
}

void Math::EvalCatmullRom(const float * const p1, const float * const p2, const float * const p3, const float * const p4, const float t, float *output)
{
	float tSqr,tSqrSqr,newT;

	//temos de ver o t (tem de estar entre [0,1])
	newT=fClamp(t,0.0f,1.0f);

	//auxiliares
	tSqr=newT*newT*0.5f;
	tSqrSqr=newT*tSqr;
	newT*=0.5f;

	//inicio a zero
	output[0]=output[1]=output[2]=0.0f;

	// matrix row 1
	output[0]+=p1[0]*(-tSqrSqr);
	output[1]+=p1[1]*(-tSqrSqr);
	output[2]+=p1[2]*(-tSqrSqr);

	output[0]+=p2[0]*tSqrSqr*3.0f;
	output[1]+=p2[1]*tSqrSqr*3.0f;
	output[2]+=p2[2]*tSqrSqr*3.0f;

	output[0]+=p3[0]*tSqrSqr*(-3.0f);
	output[1]+=p3[1]*tSqrSqr*(-3.0f);
	output[2]+=p3[2]*tSqrSqr*(-3.0f);

	output[0]+=p4[0]*tSqrSqr;
	output[1]+=p4[1]*tSqrSqr;
	output[2]+=p4[2]*tSqrSqr;

	// matrix row 2
	output[0]+=p1[0]*tSqr*2.0f;
	output[1]+=p1[1]*tSqr*2.0f;
	output[2]+=p1[2]*tSqr*2.0f;

	output[0]+=p2[0]*tSqr*(-5.0f);
	output[1]+=p2[1]*tSqr*(-5.0f);
	output[2]+=p2[2]*tSqr*(-5.0f);

	output[0]+=p3[0]*tSqr*4.0f;
	output[1]+=p3[1]*tSqr*4.0f;
	output[2]+=p3[2]*tSqr*4.0f;

	output[0]+=p4[0]*(-tSqr);
	output[1]+=p4[1]*(-tSqr);
	output[2]+=p4[2]*(-tSqr);

	// matrix row 3
	output[0]+=p1[0]*(-newT);
	output[1]+=p1[1]*(-newT);
	output[2]+=p1[2]*(-newT);

	output[0]+=p3[0]*newT;
	output[1]+=p3[1]*newT;
	output[2]+=p3[2]*newT;

	// matrix row 4
	output[0]+=p2[0];
	output[1]+=p2[1];
	output[2]+=p2[2];
}

void Math::EvalHermite(const float * const p1, const float * const p2, const float * const p3, const float * const p4, const float t, float *output)
{
	float tSqr,tCube,newT;
	float d1[3],d2[3];

	//temos de ver o t (tem de estar entre [0,1]
	newT=fClamp(t,0.0f,1.0f);

	//auxiliares
	tSqr=newT*newT;
	tCube=newT*tSqr;

	//direcoes
	d1[0]=p2[0]-p1[0];
	d1[1]=p2[1]-p1[1];
	d1[2]=p2[2]-p1[2];

	d2[0]=p4[0]-p3[0];
	d2[1]=p4[1]-p3[1];
	d2[2]=p4[2]-p3[2];

	output[0]=p2[0]*(2.0f*tCube-3.0f*tSqr+1.0f);
	output[1]=p2[1]*(2.0f*tCube-3.0f*tSqr+1.0f);
	output[2]=p2[2]*(2.0f*tCube-3.0f*tSqr+1.0f);

	output[0]+=p3[0]*(-2.0f*tCube+3.0f*tSqr);
	output[1]+=p3[1]*(-2.0f*tCube+3.0f*tSqr);
	output[2]+=p3[2]*(-2.0f*tCube+3.0f*tSqr);

	output[0]+=d1[0]*(tCube-2.0f*tSqr+t);
	output[1]+=d1[1]*(tCube-2.0f*tSqr+t);
	output[2]+=d1[2]*(tCube-2.0f*tSqr+t);

	output[0]+=d2[0]*(tCube-tSqr);
	output[1]+=d2[1]*(tCube-tSqr);
	output[2]+=d2[2]*(tCube-tSqr);
}

//as constantes
const float	Math::PI				= 3.14159265358979323846f;
const float	Math::TWO_PI			= 6.28318530717958647692f;
const float	Math::HALF_PI			= 1.57079632679489661923f;
const float	Math::ONEFOURTH_PI		= 0.78539816339744830961f;
const float Math::E					= 2.71828182845904523536f;
const float Math::SQRT_TWO			= 1.41421356237309504880f;
const float Math::SQRT_THREE		= 1.73205080756887729352f;
const float	Math::SQRT_1OVER2		= 0.70710678118654752440f;
const float	Math::SQRT_1OVER3		= 0.57735026918962576450f;
const float	Math::DEG2RAD			= 0.017453292519943295769f;
const float	Math::RAD2DEG			= 57.29577951308232087679f;
const float	Math::INFINITY			= 1e30f;
const float Math::EPSILON			= 1.192092896e-07f;

const float Math::WaveTableSin[]={
0.000000f, 0.024541f, 0.049068f, 0.073565f, 0.098017f, 0.122411f, 0.146730f, 0.170962f,
0.195090f, 0.219101f, 0.242980f, 0.266713f, 0.290285f, 0.313682f, 0.336890f, 0.359895f, 0.382683f, 0.405241f, 0.427555f,
0.449611f, 0.471397f, 0.492898f, 0.514103f, 0.534998f, 0.555570f, 0.575808f, 0.595699f, 0.615232f, 0.634393f, 0.653173f,
0.671559f, 0.689541f, 0.707107f, 0.724247f, 0.740951f, 0.757209f, 0.773010f, 0.788346f, 0.803208f, 0.817585f, 0.831470f,
0.844854f, 0.857729f, 0.870087f, 0.881921f, 0.893224f, 0.903989f, 0.914210f, 0.923880f, 0.932993f, 0.941544f, 0.949528f,
0.956940f, 0.963776f, 0.970031f, 0.975702f, 0.980785f, 0.985278f, 0.989177f, 0.992480f, 0.995185f, 0.997290f, 0.998795f,
0.999699f, 1.000000f, 0.999699f, 0.998795f, 0.997290f, 0.995185f, 0.992480f, 0.989177f, 0.985278f, 0.980785f, 0.975702f,
0.970031f, 0.963776f, 0.956940f, 0.949528f, 0.941544f, 0.932993f, 0.923880f, 0.914210f, 0.903989f, 0.893224f, 0.881921f,
0.870087f, 0.857729f, 0.844854f, 0.831470f, 0.817585f, 0.803208f, 0.788346f, 0.773010f, 0.757209f, 0.740951f, 0.724247f,
0.707107f, 0.689541f, 0.671559f, 0.653173f, 0.634393f, 0.615232f, 0.595699f, 0.575808f, 0.555570f, 0.534998f, 0.514103f,
0.492898f, 0.471397f, 0.449611f, 0.427555f, 0.405241f, 0.382683f, 0.359895f, 0.336890f, 0.313682f, 0.290285f, 0.266713f,
0.242980f, 0.219101f, 0.195090f, 0.170962f, 0.146730f, 0.122411f, 0.098017f, 0.073565f, 0.049068f, 0.024541f, 0.000000f,
-0.024541f, -0.049068f, -0.073565f, -0.098017f, -0.122411f, -0.146730f, -0.170962f, -0.195090f, -0.219101f, -0.242980f,
-0.266713f, -0.290285f, -0.313682f, -0.336890f, -0.359895f, -0.382683f, -0.405241f, -0.427555f, -0.449611f, -0.471397f,
-0.492898f, -0.514103f, -0.534998f, -0.555570f, -0.575808f, -0.595699f, -0.615232f, -0.634393f, -0.653173f, -0.671559f,
-0.689541f, -0.707107f, -0.724247f, -0.740951f, -0.757209f, -0.773010f, -0.788346f, -0.803208f, -0.817585f, -0.831470f,
-0.844854f, -0.857729f, -0.870087f, -0.881921f, -0.893224f, -0.903989f, -0.914210f, -0.923880f, -0.932993f, -0.941544f,
-0.949528f, -0.956940f, -0.963776f, -0.970031f, -0.975702f, -0.980785f, -0.985278f, -0.989177f, -0.992480f, -0.995185f,
-0.997290f, -0.998795f, -0.999699f, -1.000000f, -0.999699f, -0.998795f, -0.997290f, -0.995185f, -0.992480f, -0.989177f,
-0.985278f, -0.980785f, -0.975702f, -0.970031f, -0.963776f, -0.956940f, -0.949528f, -0.941544f, -0.932993f, -0.923880f,
-0.914210f, -0.903989f, -0.893224f, -0.881921f, -0.870087f, -0.857729f, -0.844854f, -0.831470f, -0.817585f, -0.803208f,
-0.788346f, -0.773010f, -0.757209f, -0.740951f, -0.724247f, -0.707107f, -0.689541f, -0.671559f, -0.653173f, -0.634393f,
-0.615232f, -0.595699f, -0.575808f, -0.555570f, -0.534998f, -0.514103f, -0.492898f, -0.471397f, -0.449611f, -0.427555f,
-0.405241f, -0.382683f, -0.359895f, -0.336890f, -0.313682f, -0.290285f, -0.266713f, -0.242980f, -0.219101f, -0.195090f,
-0.170962f, -0.146730f, -0.122411f, -0.098017f, -0.073565f, -0.049068f, -0.024541f };
const float Math::WaveTableCos[]={
1.000000f, 0.999699f, 0.998795f, 0.997290f, 0.995185f, 0.992480f, 0.989177f, 0.985278f,
0.980785f, 0.975702f, 0.970031f, 0.963776f, 0.956940f, 0.949528f, 0.941544f, 0.932993f, 0.923880f, 0.914210f, 0.903989f,
0.893224f, 0.881921f, 0.870087f, 0.857729f, 0.844854f, 0.831470f, 0.817585f, 0.803208f, 0.788346f, 0.773010f, 0.757209f,
0.740951f, 0.724247f, 0.707107f, 0.689541f, 0.671559f, 0.653173f, 0.634393f, 0.615232f, 0.595699f, 0.575808f, 0.555570f,
0.534998f, 0.514103f, 0.492898f, 0.471397f, 0.449611f, 0.427555f, 0.405241f, 0.382683f, 0.359895f, 0.336890f, 0.313682f,
0.290285f, 0.266713f, 0.242980f, 0.219101f, 0.195090f, 0.170962f, 0.146730f, 0.122411f, 0.098017f, 0.073565f, 0.049068f,
0.024541f, 0.000000f, -0.024541f, -0.049068f, -0.073565f, -0.098017f, -0.122411f, -0.146730f, -0.170962f, -0.195090f,
-0.219101f, -0.242980f, -0.266713f, -0.290285f, -0.313682f, -0.336890f, -0.359895f, -0.382683f, -0.405241f, -0.427555f,
-0.449611f, -0.471397f, -0.492898f, -0.514103f, -0.534998f, -0.555570f, -0.575808f, -0.595699f, -0.615232f, -0.634393f,
-0.653173f, -0.671559f, -0.689541f, -0.707107f, -0.724247f, -0.740951f, -0.757209f, -0.773010f, -0.788346f, -0.803208f,
-0.817585f, -0.831470f, -0.844854f, -0.857729f, -0.870087f, -0.881921f, -0.893224f, -0.903989f, -0.914210f, -0.923880f,
-0.932993f, -0.941544f, -0.949528f, -0.956940f, -0.963776f, -0.970031f, -0.975702f, -0.980785f, -0.985278f, -0.989177f,
-0.992480f, -0.995185f, -0.997290f, -0.998795f, -0.999699f, -1.000000f, -0.999699f, -0.998795f, -0.997290f, -0.995185f,
-0.992480f, -0.989177f, -0.985278f, -0.980785f, -0.975702f, -0.970031f, -0.963776f, -0.956940f, -0.949528f, -0.941544f,
-0.932993f, -0.923880f, -0.914210f, -0.903989f, -0.893224f, -0.881921f, -0.870087f, -0.857729f, -0.844854f, -0.831470f,
-0.817585f, -0.803208f, -0.788346f, -0.773010f, -0.757209f, -0.740951f, -0.724247f, -0.707107f, -0.689541f, -0.671559f,
-0.653173f, -0.634393f, -0.615232f, -0.595699f, -0.575808f, -0.555570f, -0.534998f, -0.514103f, -0.492898f, -0.471397f,
-0.449611f, -0.427555f, -0.405241f, -0.382683f, -0.359895f, -0.336890f, -0.313682f, -0.290285f, -0.266713f, -0.242980f,
-0.219101f, -0.195090f, -0.170962f, -0.146730f, -0.122411f, -0.098017f, -0.073565f, -0.049068f, -0.024541f, 0.000000f,
0.024541f, 0.049068f, 0.073565f, 0.098017f, 0.122411f, 0.146730f, 0.170962f, 0.195090f, 0.219101f, 0.242980f, 0.266713f,
0.290285f, 0.313682f, 0.336890f, 0.359895f, 0.382683f, 0.405241f, 0.427555f, 0.449611f, 0.471397f, 0.492898f, 0.514103f,
0.534998f, 0.555570f, 0.575808f, 0.595699f, 0.615232f, 0.634393f, 0.653173f, 0.671559f, 0.689541f, 0.707107f, 0.724247f,
0.740951f, 0.757209f, 0.773010f, 0.788346f, 0.803208f, 0.817585f, 0.831470f, 0.844854f, 0.857729f, 0.870087f, 0.881921f,
0.893224f, 0.903989f, 0.914210f, 0.923880f, 0.932993f, 0.941544f, 0.949528f, 0.956940f, 0.963776f, 0.970031f, 0.975702f,
0.980785f, 0.985278f, 0.989177f, 0.992480f, 0.995185f, 0.997290f, 0.998795f, 0.999699f };
const float Math::WaveTableBinary[]={ 0.0f, 1.0f };
const float Math::WaveTableScale[]={ 1.0f, 2.0f };
const float Math::WaveTablePulse[]={ 0.0f, 1.0f, 0.0f };
const float Math::WaveTableConvex[]={
0.0000000f, 0.0249307f, 0.0498458f, 0.0747300f, 0.0995678f, 0.1243436f, 0.1490421f,
0.1736480f, 0.1981460f, 0.2225208f, 0.2467572f, 0.2708403f, 0.2947549f, 0.3184864f, 0.3420199f, 0.3653407f, 0.3884345f,
0.4112868f, 0.4338834f, 0.4562103f, 0.4782536f, 0.4999996f, 0.5214348f, 0.5425459f, 0.5633196f, 0.5837433f, 0.6038040f,
0.6234894f, 0.6427872f, 0.6616854f, 0.6801723f, 0.6982363f, 0.7158664f, 0.7330514f, 0.7497807f, 0.7660440f, 0.7818310f,
0.7971320f, 0.8119375f, 0.8262383f, 0.8400255f, 0.8532904f, 0.8660250f, 0.8782212f, 0.8898714f, 0.9009684f, 0.9115055f,
0.9214758f, 0.9308734f, 0.9396923f, 0.9479270f, 0.9555725f, 0.9626240f, 0.9690770f, 0.9749277f, 0.9801723f, 0.9848076f,
0.9888306f, 0.9922391f, 0.9950306f, 0.9972037f, 0.9987569f, 0.9996892f, 1.0000000f };
const float Math::WaveTableConcave[]={
0.0000000f, 0.0003108f, 0.0012431f, 0.0027962f, 0.0049692f, 0.0077608f, 0.0111692f,
0.0151922f, 0.0198275f, 0.0250720f, 0.0309227f, 0.0373757f, 0.0444271f, 0.0520726f, 0.0603073f, 0.0691261f, 0.0785237f,
0.0884940f, 0.0990310f, 0.1101280f, 0.1217782f, 0.1339744f, 0.1467089f, 0.1599738f, 0.1737609f, 0.1880617f, 0.2028672f,
0.2181682f, 0.2339552f, 0.2502184f, 0.2669477f, 0.2841327f, 0.3017627f, 0.3198268f, 0.3383136f, 0.3572118f, 0.3765096f,
0.3961950f, 0.4162557f, 0.4366793f, 0.4574530f, 0.4785641f, 0.4999992f, 0.5217452f, 0.5437885f, 0.5661154f, 0.5887120f,
0.6115643f, 0.6346580f, 0.6579789f, 0.6815124f, 0.7052438f, 0.7291585f, 0.7532415f, 0.7774780f, 0.8018527f, 0.8263507f,
0.8509566f, 0.8756551f, 0.9004309f, 0.9252687f, 0.9501528f, 0.9750680f, 1.0000000f };
const float Math::WaveTableSOS[]={ 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,};

//§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§ Math::SIMD		§§
//§§§§§§§§§§§§§§§§§§§§§§§§

//as constantes
const __m128 Math::SIMD::fOne = _mm_set_ps1(1.0f);
const __m128 Math::SIMD::fHalfOne = _mm_set_ps1(0.5f);
const __m128 Math::SIMD::fPi = _mm_set_ps1(Math::PI);
const __m128 Math::SIMD::fDeg2Rad = _mm_set_ps1(Math::DEG2RAD);
const __m128 Math::SIMD::fRad2Deg = _mm_set_ps1(Math::RAD2DEG);
const __m128 Math::SIMD::fUByteMax = _mm_set_ps1(255.0f);
const __m128 Math::SIMD::fUByteMaxInv = _mm_set_ps1(0.003921568627450980392f);

//as funções em baixo (tiradas daqui: http://gruntthepeon.free.fr/ssemath/) precisam destas constantes todas

#define _PS_CONST(Name, Val) static const __declspec(align(16)) float _ps_##Name[4]  = { Val, Val, Val, Val }
#define _PI32_CONST(Name, Val) static const __declspec(align(16)) int _pi32_##Name[4] = { Val, Val, Val, Val }
#define _PS_CONST_TYPE(Name, Type, Val) static const __declspec(align(16)) Type _ps_##Name[4] = { Val, Val, Val, Val }

/* the smallest non denormalized float number */
_PS_CONST_TYPE(sign_mask, int, 0x80000000);
_PS_CONST_TYPE(inv_sign_mask, int, ~0x80000000);
_PS_CONST_TYPE(min_norm_pos, int, 0x00800000);
_PS_CONST_TYPE(mant_mask, int, 0x7f800000);
_PS_CONST_TYPE(inv_mant_mask, int, ~0x7f800000);

_PI32_CONST(1, 1);
_PI32_CONST(inv1, ~1);
_PI32_CONST(2, 2);
_PI32_CONST(4, 4);
_PI32_CONST(0x7f, 0x7f);

_PS_CONST(minus_cephes_DP1, -0.78515625);
_PS_CONST(minus_cephes_DP2, -2.4187564849853515625e-4);
_PS_CONST(minus_cephes_DP3, -3.77489497744594108e-8);
_PS_CONST(sincof_p0, -1.9515295891E-4);
_PS_CONST(sincof_p1,  8.3321608736E-3);
_PS_CONST(sincof_p2, -1.6666654611E-1);
_PS_CONST(coscof_p0,  2.443315711809948E-005);
_PS_CONST(coscof_p1, -1.388731625493765E-003);
_PS_CONST(coscof_p2,  4.166664568298827E-002);
_PS_CONST(cephes_FOPI, 1.27323954473516); // 4 / M_PI

_PS_CONST(cephes_SQRTHF, 0.707106781186547524);
_PS_CONST(cephes_log_p0, 7.0376836292E-2);
_PS_CONST(cephes_log_p1, - 1.1514610310E-1);
_PS_CONST(cephes_log_p2, 1.1676998740E-1);
_PS_CONST(cephes_log_p3, - 1.2420140846E-1);
_PS_CONST(cephes_log_p4, + 1.4249322787E-1);
_PS_CONST(cephes_log_p5, - 1.6668057665E-1);
_PS_CONST(cephes_log_p6, + 2.0000714765E-1);
_PS_CONST(cephes_log_p7, - 2.4999993993E-1);
_PS_CONST(cephes_log_p8, + 3.3333331174E-1);
_PS_CONST(cephes_log_q1, -2.12194440e-4);
_PS_CONST(cephes_log_q2, 0.693359375);

_PS_CONST(exp_hi,	88.3762626647949f);
_PS_CONST(exp_lo,	-88.3762626647949f);

_PS_CONST(cephes_LOG2EF, 1.44269504088896341);
_PS_CONST(cephes_exp_C1, 0.693359375);
_PS_CONST(cephes_exp_C2, -2.12194440e-4);

_PS_CONST(cephes_exp_p0, 1.9875691500E-4);
_PS_CONST(cephes_exp_p1, 1.3981999507E-3);
_PS_CONST(cephes_exp_p2, 8.3334519073E-3);
_PS_CONST(cephes_exp_p3, 4.1665795894E-2);
_PS_CONST(cephes_exp_p4, 1.6666665459E-1);
_PS_CONST(cephes_exp_p5, 5.0000001201E-1);

void Math::SIMD::sinCosG(__m128 graus, __m128 * const s, __m128 * const c)
{
	//basta chamar isto
	Math::SIMD::sinCosR(_mm_mul_ps(graus, Math::SIMD::fDeg2Rad), s, c);
}

void Math::SIMD::sinCosR(__m128 radians, __m128 * const s, __m128 * const c)
{
	//since sin_ps and cos_ps are almost identical, sincos_ps could replace both of them..	it is almost as fast, and gives you a free cosine with your sine
	__m128 xmm1, xmm2, xmm3 = _mm_setzero_ps(), sign_bit_sin, y;
	__m128i emm0, emm2, emm4;

	sign_bit_sin = radians;
	//take the absolute value
	radians = _mm_and_ps(radians, *(__m128*)_ps_inv_sign_mask);
	//extract the sign bit (upper one)
	sign_bit_sin = _mm_and_ps(sign_bit_sin, *(__m128*)_ps_sign_mask);
  
	//scale by 4/Pi
	y = _mm_mul_ps(radians, *(__m128*)_ps_cephes_FOPI);
    
	//store the integer part of y in emm2
	emm2 = _mm_cvttps_epi32(y);

	//j=(j+1) & (~1) (see the cephes sources)
	emm2 = _mm_add_epi32(emm2, *(__m128i*)_pi32_1);
	emm2 = _mm_and_si128(emm2, *(__m128i*)_pi32_inv1);
	y = _mm_cvtepi32_ps(emm2);

	emm4 = emm2;

	//get the swap sign flag for the sine
	emm0 = _mm_and_si128(emm2, *(__m128i*)_pi32_4);
	emm0 = _mm_slli_epi32(emm0, 29);
	__m128 swap_sign_bit_sin = _mm_castsi128_ps(emm0);

	//get the polynom selection mask for the sine
	emm2 = _mm_and_si128(emm2, *(__m128i*)_pi32_2);
	emm2 = _mm_cmpeq_epi32(emm2, _mm_setzero_si128());
	__m128 poly_mask = _mm_castsi128_ps(emm2);

	//The magic pass: "Extended precision modular arithmetic" 	x = ((x - y * DP1) - y * DP2) - y * DP3;
	xmm1 = *(__m128*)_ps_minus_cephes_DP1;
	xmm2 = *(__m128*)_ps_minus_cephes_DP2;
	xmm3 = *(__m128*)_ps_minus_cephes_DP3;
	xmm1 = _mm_mul_ps(y, xmm1);
	xmm2 = _mm_mul_ps(y, xmm2);
	xmm3 = _mm_mul_ps(y, xmm3);
	radians = _mm_add_ps(radians, xmm1);
	radians = _mm_add_ps(radians, xmm2);
	radians = _mm_add_ps(radians, xmm3);

	emm4 = _mm_sub_epi32(emm4, *(__m128i*)_pi32_2);
	emm4 = _mm_andnot_si128(emm4, *(__m128i*)_pi32_4);
	emm4 = _mm_slli_epi32(emm4, 29);
	__m128 sign_bit_cos = _mm_castsi128_ps(emm4);

	sign_bit_sin = _mm_xor_ps(sign_bit_sin, swap_sign_bit_sin);

  
	//Evaluate the first polynom  (0 <= x <= Pi/4)
	__m128 z = _mm_mul_ps(radians, radians);
	y = *(__m128*)_ps_coscof_p0;

	y = _mm_mul_ps(y, z);
	y = _mm_add_ps(y, *(__m128*)_ps_coscof_p1);
	y = _mm_mul_ps(y, z);
	y = _mm_add_ps(y, *(__m128*)_ps_coscof_p2);
	y = _mm_mul_ps(y, z);
	y = _mm_mul_ps(y, z);
	__m128 tmp = _mm_mul_ps(z, Math::SIMD::fHalfOne);
	y = _mm_sub_ps(y, tmp);
	y = _mm_add_ps(y, Math::SIMD::fOne);
  
	//Evaluate the second polynom  (Pi/4 <= x <= 0)

	__m128 y2 = *(__m128*)_ps_sincof_p0;
	y2 = _mm_mul_ps(y2, z);
	y2 = _mm_add_ps(y2, *(__m128*)_ps_sincof_p1);
	y2 = _mm_mul_ps(y2, z);
	y2 = _mm_add_ps(y2, *(__m128*)_ps_sincof_p2);
	y2 = _mm_mul_ps(y2, z);
	y2 = _mm_mul_ps(y2, radians);
	y2 = _mm_add_ps(y2, radians);

	//select the correct result from the two polynoms
	xmm3 = poly_mask;
	__m128 ysin2 = _mm_and_ps(xmm3, y2);
	__m128 ysin1 = _mm_andnot_ps(xmm3, y);
	y2 = _mm_sub_ps(y2,ysin2);
	y = _mm_sub_ps(y, ysin1);

	xmm1 = _mm_add_ps(ysin1,ysin2);
	xmm2 = _mm_add_ps(y,y2);
 
	//update the sign
	*s = _mm_xor_ps(xmm1, sign_bit_sin);
	*c = _mm_xor_ps(xmm2, sign_bit_cos);
}

__m128 Math::SIMD::log(__m128 x)
{
	__m128i emm0;
	__m128 one = Math::SIMD::fOne;

	__m128 invalid_mask = _mm_cmple_ps(x, _mm_setzero_ps());

	x = _mm_max_ps(x, *(__m128*)_ps_min_norm_pos);  //cut off denormalized stuff

	
	emm0 = _mm_srli_epi32(_mm_castps_si128(x), 23);
	
	//keep only the fractional part
	x = _mm_and_ps(x, *(__m128*)_ps_inv_mant_mask);
	x = _mm_or_ps(x, Math::SIMD::fHalfOne);

	
	emm0 = _mm_sub_epi32(emm0, *(__m128i*)_pi32_0x7f);
	__m128 e = _mm_cvtepi32_ps(emm0);

	e = _mm_add_ps(e, one);

	/*part2: 
	if( x < SQRTHF ) {
	e -= 1;
	x = x + x - 1.0;
	} else { x = x - 1.0; }*/
	__m128 mask = _mm_cmplt_ps(x, *(__m128*)_ps_cephes_SQRTHF);
	__m128 tmp = _mm_and_ps(x, mask);
	x = _mm_sub_ps(x, one);
	e = _mm_sub_ps(e, _mm_and_ps(one, mask));
	x = _mm_add_ps(x, tmp);


	__m128 z = _mm_mul_ps(x,x);

	__m128 y = *(__m128*)_ps_cephes_log_p0;
	y = _mm_mul_ps(y, x);
	y = _mm_add_ps(y, *(__m128*)_ps_cephes_log_p1);
	y = _mm_mul_ps(y, x);
	y = _mm_add_ps(y, *(__m128*)_ps_cephes_log_p2);
	y = _mm_mul_ps(y, x);
	y = _mm_add_ps(y, *(__m128*)_ps_cephes_log_p3);
	y = _mm_mul_ps(y, x);
	y = _mm_add_ps(y, *(__m128*)_ps_cephes_log_p4);
	y = _mm_mul_ps(y, x);
	y = _mm_add_ps(y, *(__m128*)_ps_cephes_log_p5);
	y = _mm_mul_ps(y, x);
	y = _mm_add_ps(y, *(__m128*)_ps_cephes_log_p6);
	y = _mm_mul_ps(y, x);
	y = _mm_add_ps(y, *(__m128*)_ps_cephes_log_p7);
	y = _mm_mul_ps(y, x);
	y = _mm_add_ps(y, *(__m128*)_ps_cephes_log_p8);
	y = _mm_mul_ps(y, x);

	y = _mm_mul_ps(y, z);
  

	tmp = _mm_mul_ps(e, *(__m128*)_ps_cephes_log_q1);
	y = _mm_add_ps(y, tmp);


	tmp = _mm_mul_ps(z, Math::SIMD::fHalfOne);
	y = _mm_sub_ps(y, tmp);

	tmp = _mm_mul_ps(e, *(__m128*)_ps_cephes_log_q2);
	x = _mm_add_ps(x, y);
	x = _mm_add_ps(x, tmp);
	x = _mm_or_ps(x, invalid_mask); // negative arg will be NAN
	return x;
}

__m128 exp(__m128 x)
{
	__m128 tmp = _mm_setzero_ps(), fx;
	__m128i emm0;
	__m128 one = Math::SIMD::fOne;

	x = _mm_min_ps(x, *(__m128*)_ps_exp_hi);
	x = _mm_max_ps(x, *(__m128*)_ps_exp_lo);

	//express exp(x) as exp(g + n*log(2))
	fx = _mm_mul_ps(x, *(__m128*)_ps_cephes_LOG2EF);
	fx = _mm_add_ps(fx, Math::SIMD::fHalfOne);

	//how to perform a floorf with SSE: just below
	emm0 = _mm_cvttps_epi32(fx);
	tmp  = _mm_cvtepi32_ps(emm0);
	//if greater, substract 1
	__m128 mask = _mm_cmpgt_ps(tmp, fx);    
	mask = _mm_and_ps(mask, one);
	fx = _mm_sub_ps(tmp, mask);

	tmp = _mm_mul_ps(fx, *(__m128*)_ps_cephes_exp_C1);
	__m128 z = _mm_mul_ps(fx, *(__m128*)_ps_cephes_exp_C2);
	x = _mm_sub_ps(x, tmp);
	x = _mm_sub_ps(x, z);

	z = _mm_mul_ps(x,x);
  
	__m128 y = *(__m128*)_ps_cephes_exp_p0;
	y = _mm_mul_ps(y, x);
	y = _mm_add_ps(y, *(__m128*)_ps_cephes_exp_p1);
	y = _mm_mul_ps(y, x);
	y = _mm_add_ps(y, *(__m128*)_ps_cephes_exp_p2);
	y = _mm_mul_ps(y, x);
	y = _mm_add_ps(y, *(__m128*)_ps_cephes_exp_p3);
	y = _mm_mul_ps(y, x);
	y = _mm_add_ps(y, *(__m128*)_ps_cephes_exp_p4);
	y = _mm_mul_ps(y, x);
	y = _mm_add_ps(y, *(__m128*)_ps_cephes_exp_p5);
	y = _mm_mul_ps(y, z);
	y = _mm_add_ps(y, x);
	y = _mm_add_ps(y, one);

	emm0 = _mm_cvttps_epi32(fx);
	emm0 = _mm_add_epi32(emm0, *(__m128i*)_pi32_0x7f);
	emm0 = _mm_slli_epi32(emm0, 23);
	__m128 pow2n = _mm_castsi128_ps(emm0);
	y = _mm_mul_ps(y, pow2n);
	return y;
}

}//namespace HorseRadish