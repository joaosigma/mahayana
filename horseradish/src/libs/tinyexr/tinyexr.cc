#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#endif

#define TINYEXR_USE_THREAD 0
#define TINYEXR_USE_OPENMP 0

#define TINYEXR_USE_MINIZ 0
#include "libs/zlib/zlib.h"

#define TINYEXR_IMPLEMENTATION
#include "tinyexr.h"
