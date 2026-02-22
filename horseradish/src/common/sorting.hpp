#pragma once

#include <cstddef>

namespace hr
{
    class Sorting
    {
    public:
        static void radixSort(int* baseArray, int* tempArray, size_t numElements);
        static void radixQueue(float* baseArray, float* tempArray, int* orderOut, int* orderTemp, size_t numElements);
    };
}
