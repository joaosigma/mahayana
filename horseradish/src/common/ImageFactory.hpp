#pragma once

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

			static HorseRadish::Imaging::Image* Read(HorseRadish::Streams::StreamReader &streamReader);

			static HorseRadish::Imaging::Image* ReadTGA(HorseRadish::Streams::StreamReader &streamReader);
			static bool SaveTGA(HorseRadish::Streams::StreamWriter &streamWriter, const HorseRadish::Imaging::Image * const imageToSave);

			static HorseRadish::Imaging::Image* ReadPNG(HorseRadish::Streams::StreamReader &streamReader);
			static bool SavePNG(HorseRadish::Streams::StreamWriter &streamWriter, const HorseRadish::Imaging::Image * const imageToSave);

			static HorseRadish::Imaging::Image* ReadJPEG(HorseRadish::Streams::StreamReader &streamReader);
			static bool SaveJPEG(HorseRadish::Streams::StreamWriter &streamWriter, const HorseRadish::Imaging::Image * const imageToSave, int qualityPercent);

			static HorseRadish::Imaging::Image* ReadHDRI(HorseRadish::Streams::StreamReader &streamReader);

			static bool SaveBMP(HorseRadish::Streams::StreamWriter &streamWriter, const HorseRadish::Imaging::Image * const imageToSave);
		};

	} //Imaging
} //HorseRadish
