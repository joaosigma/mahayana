#pragma once
#ifndef __IMAGE_FACTORY_TARGA__
#define __IMAGE_FACTORY_TARGA__

#include "common\image.hpp"
#include "common\stream.hpp"

HorseRadish::Imaging::Image* readTGA(HorseRadish::Streams::StreamReader * const streamReader);
bool isTGA(HorseRadish::Streams::StreamReader * const streamReader);
bool saveTGA(HorseRadish::Streams::StreamWriter * const streamWriter, const HorseRadish::Imaging::Image * const imageToSave);

#endif