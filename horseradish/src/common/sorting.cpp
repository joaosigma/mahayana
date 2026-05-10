module;

#include "types.hpp"

module core:sorting.impl;

import :types;
import :sorting;

namespace hr
{
    namespace
    {
        void radixByte0(size_t howMuch, const int* const HRESTRICT source, int* const HRESTRICT dest)
        {
            size_t count[256], index[256], i;

            std::memset(count, 0, sizeof(count));
            for (i = 0; i < howMuch; i++)
                count[((source[i]) >> (0)) & 0xff]++;
            index[0] = 0;
            for (i = 0; i < 255; i++)
                index[i + 1] = index[i] + count[i];
            for (i = 0; i < howMuch; i++)
                dest[index[((source[i]) >> (0)) & 0xff]++] = source[i];
        }

        void radixByte1(size_t howMuch, const int* const HRESTRICT source, int* const HRESTRICT dest)
        {
            size_t count[256], index[256], i;

            std::memset(count, 0, sizeof(count));
            for (i = 0; i < howMuch; i++)
                count[((source[i]) >> (8)) & 0xff]++;
            index[0] = 0;
            for (i = 0; i < 255; i++)
                index[i + 1] = index[i] + count[i];
            for (i = 0; i < howMuch; i++)
                dest[index[((source[i]) >> (8)) & 0xff]++] = source[i];
        }

        void radixByte2(size_t howMuch, const int* const HRESTRICT source, int* const HRESTRICT dest)
        {
            size_t count[256], index[256], i;

            std::memset(count, 0, sizeof(count));
            for (i = 0; i < howMuch; i++)
                count[((source[i]) >> (16)) & 0xff]++;
            index[0] = 0;
            for (i = 0; i < 255; i++)
                index[i + 1] = index[i] + count[i];
            for (i = 0; i < howMuch; i++)
                dest[index[((source[i]) >> (16)) & 0xff]++] = source[i];
        }

        void radixByte3(size_t howMuch, const int* const HRESTRICT source, int* const HRESTRICT dest)
        {
            size_t count[256], index[256], i;

            std::memset(count, 0, sizeof(count));
            for (i = 0; i < howMuch; i++)
                count[((source[i]) >> (24)) & 0xff]++;
            index[0] = 0;
            for (i = 0; i < 255; i++)
                index[i + 1] = index[i] + count[i];
            for (i = 0; i < howMuch; i++)
                dest[index[((source[i]) >> (24)) & 0xff]++] = source[i];
        }
    }

    void Sorting::radixSort(int* baseArray, int* tempArray, size_t numElements)
    {
        if (!baseArray || !tempArray || numElements <= 1)
            return;

        radixByte0(numElements, baseArray, tempArray);
        radixByte1(numElements, tempArray, baseArray);
        radixByte2(numElements, baseArray, tempArray);
        radixByte3(numElements, tempArray, baseArray);
    }

    void Sorting::radixQueue(float* baseArray, float* tempArray, int* orderOut, int* orderTemp, size_t numElements)
    {
        unsigned char* c;
        size_t i, counter[256], offset[256];

        if (!baseArray || !tempArray || numElements <= 1)
            return;

        for (size_t p = 0; p < 4; ++p)
        {
            std::memset(counter, 0, sizeof(int) * 256);

            for (i = 0; i < numElements; i++)
            {
                c = ((unsigned char*)&baseArray[i]) + p;
                counter[*c]++;
            }

            offset[0] = 0;
            for (i = 1; i < 256; i++)
                offset[i] = offset[i - 1] + counter[i - 1];

            for (i = 0; i < numElements; i++)
            {
                c = ((unsigned char*)&baseArray[i]) + p;
                tempArray[offset[*c]] = baseArray[i];
                orderTemp[offset[*c]] = orderOut[i];
                ++offset[*c];
            }

            std::swap(baseArray, tempArray);
            std::swap(orderOut, orderTemp);
        }
    }
}
