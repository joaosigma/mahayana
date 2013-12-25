#pragma once
#ifndef __IMAGE_FACTORY_HDRI__
#define __IMAGE_FACTORY_HDRI__

#include "common\image.hpp"
#include "common\stream.hpp"

HorseRadish::Imaging::Image* readHDRI(HorseRadish::Streams::StreamReader * const streamReader);

#endif