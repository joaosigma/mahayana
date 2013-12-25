#pragma once
#ifndef __HIMAGE_FACTORY__
#define __HIMAGE_FACTORY__

#include "Image.hpp"
#include "Stream.hpp"

namespace HorseRadish
{
namespace Imaging
{

class Factory {
public:

	static HorseRadish::Imaging::Image* CreateImageUByte(const int width, const int height, const HorseRadish::Imaging::Image::ImageFormat format, hUInt8 ** const imageBuffer);
	static HorseRadish::Imaging::Image* CreateImageFloat(const int width, const int height, const HorseRadish::Imaging::Image::ImageFormat format, hFloat ** const imageBuffer);

	static HorseRadish::Imaging::Image* Read(HorseRadish::Streams::StreamReader * const streamReader);

	static HorseRadish::Imaging::Image* ReadTGA(HorseRadish::Streams::StreamReader * const streamReader);
	static bool SaveTGA(HorseRadish::Streams::StreamWriter * const streamWriter, const HorseRadish::Imaging::Image * const imageToSave);

	static HorseRadish::Imaging::Image* ReadPNG(HorseRadish::Streams::StreamReader * const streamReader);
	static bool SavePNG(HorseRadish::Streams::StreamWriter * const streamWriter, const HorseRadish::Imaging::Image * const imageToSave);

	static HorseRadish::Imaging::Image* ReadJPEG(HorseRadish::Streams::StreamReader * const streamReader);
	static bool SaveJPEG(HorseRadish::Streams::StreamWriter * const streamWriter, const HorseRadish::Imaging::Image * const imageToSave, int qualityPercent);

	static HorseRadish::Imaging::Image* ReadHDRI(HorseRadish::Streams::StreamReader * const streamReader);

	static bool SaveBMP(HorseRadish::Streams::StreamWriter * const streamWriter, const HorseRadish::Imaging::Image * const imageToSave);
};

}//namespace Imaging
}//namespace HorseRadish

#endif