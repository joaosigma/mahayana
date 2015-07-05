#pragma once

#include "Image.hpp"
#include "Stream.hpp"

namespace HorseRadish { namespace Imaging {

class Factory
{
public:

	static Image<unsigned char, ImageFormatRGB> readPNG(HorseRadish::Streams::StreamReader &streamReader);
	static Image<unsigned char, ImageFormatRGBA> readPNGWithAlpha(HorseRadish::Streams::StreamReader &streamReader);

	static bool savePNG(HorseRadish::Streams::StreamWriter &streamWriter, const ImageView<unsigned char, ImageFormatRGB>& imgView);
	static bool savePNG(HorseRadish::Streams::StreamWriter &streamWriter, const ImageView<unsigned char, ImageFormatRGBA>& imgView);

	static Image<unsigned char, ImageFormatRGBA> readTGA(HorseRadish::Streams::StreamReader &streamReader);
	
	static Image<unsigned char, ImageFormatRGB> readJPG(HorseRadish::Streams::StreamReader &streamReader);

	static Image<float, ImageFormatRGB> readHDRI(HorseRadish::Streams::StreamReader &streamReader);
};

} }

