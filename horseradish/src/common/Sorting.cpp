#include "Sorting.hpp"
#include "Types.hpp"

#include <windows.h>
#include <malloc.h>
#include <limits.h>

namespace HorseRadish
{

void Sorting::radixByte0(const int &numero, const int * const HRESTRICT source, int * const HRESTRICT dest)
{
	int count[256],index[256],i;

	memset(count,0,sizeof(count));
	for(i=0; i<numero; i++ )
		count[((source[i])>>(0))&0xff]++;
	index[0]=0;
	for(i=0; i<255; i++)
		index[i+1]=index[i]+count[i];
	for(i=0; i<numero; i++)
		dest[index[((source[i])>>(0))&0xff]++]=source[i];
}
void Sorting::radixByte1(const int &numero, const int * const HRESTRICT source, int * const HRESTRICT dest)
{
	int count[256],index[256],i;

	memset(count, 0, sizeof (count));
	for(i=0; i<numero; i++ )
		count[((source[i])>>(8))&0xff]++;
	index[0]=0;
	for(i=0; i<255; i++)
		index[i+1]=index[i]+count[i];
	for(i=0; i<numero; i++)
		dest[index[((source[i])>>(8))&0xff]++]=source[i];
}
void Sorting::radixByte2(const int &numero, const int * const HRESTRICT source, int * const HRESTRICT dest)
{
	int count[256],index[256],i;

	memset(count, 0, sizeof (count));
	for(i=0; i<numero; i++ )
		count[((source[i])>>(16))&0xff]++;
	index[0]=0;
	for(i=0; i<255; i++)
		index[i+1]=index[i]+count[i];
	for(i=0; i<numero; i++)
		dest[index[((source[i])>>(16))&0xff]++]=source[i];
}
void Sorting::radixByte3(const int &numero, const int * const HRESTRICT source, int * const HRESTRICT dest)
{
	int count[256],index[256],i;

	memset(count, 0, sizeof (count));
	for(i=0; i<numero; i++ )
		count[((source[i])>>(24))&0xff]++;
	index[0]=0;
	for(i=0; i<255; i++)
		index[i+1]=index[i]+count[i];
	for(i=0; i<numero; i++)
		dest[index[((source[i])>>(24))&0xff]++]=source[i];
}

void Sorting::RadixSort(int *baseArray, int *tempArray, const unsigned int numElements)
{
	if (baseArray==nullptr || tempArray==nullptr || numElements<=1)
		return;

	radixByte0(numElements, baseArray, tempArray);
	radixByte1(numElements, tempArray, baseArray);
	radixByte2(numElements, baseArray, tempArray);
	radixByte3(numElements, tempArray, baseArray);
}

void Sorting::RadixQueue(float *baseArray, float *tempArray, int *orderOut, int *orderTemp, const unsigned int numElements)
{
	int *tmpi;
	float *tmp;
	unsigned char *c;
	unsigned int i,counter[256],offset[256];

	if (baseArray==nullptr || tempArray==nullptr || numElements<=1)
		return;

	for(int p=0; p<4; ++p)
	{
		memset(counter,0,sizeof(int)*256);
		
		for(i=0; i<numElements; i++)
		{
			c=((unsigned char*)&baseArray[i])+p;
			counter[*c]++;
		}

		offset[0]=0;
		for(i=1; i<256; i++)
			offset[i]=offset[i-1]+counter[i-1];

		for(i=0; i<numElements; i++)
		{
			c=((unsigned char*)&baseArray[i])+p;
			tempArray[ offset[*c] ]=baseArray[i];
			orderTemp[offset[*c]]=orderOut[i];
			++offset[*c];
		}

		tmp=baseArray;
		baseArray=tempArray;
		tempArray=tmp;

		tmpi=orderOut;
		orderOut=orderTemp;
		orderTemp=tmpi;
	}
}

}//namespace HorseRadish