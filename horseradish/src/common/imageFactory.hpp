#pragma once

#include "image.hpp"
#include "stream.hpp"

namespace hr { namespace imaging
{
	class Factory
	{
	public:

		static Image<unsigned char, ImageFormatRGB> readPNG(const hr::streams::StreamReader &streamReader);
		static Image<unsigned char, ImageFormatRGBA> readPNGWithAlpha(const hr::streams::StreamReader &streamReader);

		static bool savePNG(hr::streams::StreamWriter &streamWriter, const ImageView<unsigned char, ImageFormatRGB>& imgView);
		static bool savePNG(hr::streams::StreamWriter &streamWriter, const ImageView<unsigned char, ImageFormatRGBA>& imgView);

		static Image<unsigned char, ImageFormatRGBA> readTGA(hr::streams::StreamReader &streamReader);
		
		static Image<unsigned char, ImageFormatRGB> readJPG(hr::streams::StreamReader &streamReader);

		static Image<float, ImageFormatRGB> readHDRI(hr::streams::StreamReader &streamReader);
	};
} }
