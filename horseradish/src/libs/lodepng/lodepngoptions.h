#ifndef LODEPNG_OPTIONS_H
#define LODEPNG_OPTIONS_H

/*deflate & zlib. If disabled, you must specify alternative zlib functions in
the custom_zlib field of the compress and decompress settings*/
// #define LODEPNG_NO_COMPILE_ZLIB

/*png encoder and png decoder*/
// #define LODEPNG_NO_COMPILE_PNG

/*deflate&zlib decoder and png decoder*/
// #define LODEPNG_NO_COMPILE_DECODER

/*deflate&zlib encoder and png encoder*/
// #define LODEPNG_NO_COMPILE_ENCODER

/*the optional built in harddisk file loading and saving functions*/
#define LODEPNG_NO_COMPILE_DISK

/*support for chunks other than IHDR, IDAT, PLTE, tRNS, IEND: ancillary and unknown chunks*/
// #define LODEPNG_NO_COMPILE_ANCILLARY_CHUNKS

/*ability to convert error numerical codes to English text string*/
#define LODEPNG_NO_COMPILE_ERROR_TEXT

/*Compile the default allocators (C's free, malloc and realloc). If you disable this,
you can define the functions lodepng_free, lodepng_malloc and lodepng_realloc in your
source files with custom allocators.*/
// #define LODEPNG_NO_COMPILE_ALLOCATORS

/*compile the C++ version (you can disable the C++ wrapper here even when compiling for C++)*/
#ifdef __cplusplus
#define LODEPNG_NO_COMPILE_CPP
#endif

#endif /*LODEPNG_OPTIONS_H inclusion guard*/
