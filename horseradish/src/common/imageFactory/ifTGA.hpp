#pragma once

#include "common\image.hpp"
#include "common\stream.hpp"

HorseRadish::Imaging::Image* readTGA(HorseRadish::Streams::StreamReader &streamReader);
bool isTGA(HorseRadish::Streams::StreamReader &streamReader);
bool saveTGA(HorseRadish::Streams::StreamWriter &streamWriter, const HorseRadish::Imaging::Image * const imageToSave);
