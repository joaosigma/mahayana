#pragma once

#include "common/imageFactory.hpp"

#include "libs/stb/stb_image.h"
#include "libs/lodepng/lodepng.h"

#include <memory>

static
int stbIORead(void *user, char *data, int size)
{
	auto streamReader = static_cast<HorseRadish::Streams::StreamReader*>(user);
	return streamReader->read(data, size);
}

static
void stbIOSkip(void *user, int n)
{
	auto streamReader = static_cast<HorseRadish::Streams::StreamReader*>(user);
	streamReader->skip(n);
}

static
int stbIOEof(void *user)
{
	auto streamReader = static_cast<HorseRadish::Streams::StreamReader*>(user);
	return !streamReader->canRead();
}

namespace HorseRadish { namespace Imaging {

Image<unsigned char, ImageFormatRGB> Factory::readPNG(HorseRadish::Streams::StreamReader &streamReader)
{
	auto streamContent = streamReader.stream().readEntireContent();

	size_t outW = 0, outH = 0;
	unsigned char* outBuffer = nullptr;

	if (lodepng_decode24(&outBuffer, &outW, &outH, static_cast<const unsigned char *>(streamContent->getData()), streamContent->length()) != 0)
		return Image<unsigned char, ImageFormatRGB>();

	if (outBuffer != nullptr)
		return Image<unsigned char, ImageFormatRGB>(std::unique_ptr<unsigned char[]>(outBuffer), outW, outH);

	return Image<unsigned char, ImageFormatRGB>();
}

Image<unsigned char, ImageFormatRGBA> Factory::readPNGWithAlpha(HorseRadish::Streams::StreamReader &streamReader)
{
	auto streamContent = streamReader.stream().readEntireContent();

	size_t outW = 0, outH = 0;
	unsigned char* outBuffer = nullptr;

	if (lodepng_decode32(&outBuffer, &outW, &outH, static_cast<const unsigned char *>(streamContent->getData()), streamContent->length()) != 0)
		return Image<unsigned char, ImageFormatRGBA>();

	if (outBuffer != nullptr)
		return Image<unsigned char, ImageFormatRGBA>(std::unique_ptr<unsigned char[]>(outBuffer), outW, outH);

	return Image<unsigned char, ImageFormatRGBA>();
}

bool Factory::savePNG(HorseRadish::Streams::StreamWriter &streamWriter, const ImageView<unsigned char, ImageFormatRGB>& imgView)
{
	if (imgView.empty())
		return false;

	unsigned char *bufferOut = nullptr;
	size_t bufferOutSize = 0;
	lodepng_encode24(&bufferOut, &bufferOutSize, imgView.data(), imgView.width(), imgView.height());

	if ((bufferOut == nullptr) || (bufferOutSize <= 0))
		return false;

	streamWriter.write(bufferOut, bufferOutSize);

	free(bufferOut);

	return true;
}

bool Factory::savePNG(HorseRadish::Streams::StreamWriter &streamWriter, const ImageView<unsigned char, ImageFormatRGBA>& imgView)
{
	if (imgView.empty())
		return false;

	unsigned char *bufferOut = nullptr;
	size_t bufferOutSize = 0;
	lodepng_encode32(&bufferOut, &bufferOutSize, imgView.data(), imgView.width(), imgView.height());

	if ((bufferOut == nullptr) || (bufferOutSize <= 0))
		return false;

	streamWriter.write(bufferOut, bufferOutSize);

	free(bufferOut);

	return true;
}

Image<unsigned char, ImageFormatRGBA> Factory::readTGA(HorseRadish::Streams::StreamReader &streamReader)
{
	#pragma pack(1)
	struct Tgaheader
	{
		__int8	descriptionlen;
		__int8	cmaptype;
		__int8	imagetype;
		__int16	cmapstart;
		__int16	cmapentries;
		__int8	cmapbits;
		__int16	xoffset;
		__int16	yoffset;
		__int16	width;
		__int16	height;
		__int8	bpp;
		__int8	attrib;
	};

	Tgaheader header;
	streamReader.read(&header, sizeof(header));

	if ((header.bpp != 8) && (header.bpp != 16) && (header.bpp != 24) && (header.bpp != 32))
		return Image<unsigned char, ImageFormatRGBA>();

	if (!(header.imagetype & 0x08) && (header.bpp == 24 || header.bpp == 32)) // plain format
	{
		Image<unsigned char, ImageFormatRGBA> newImage(header.width, header.height);

		streamReader.skip(header.descriptionlen + header.cmapentries * header.cmapbits / 8);

		if (header.bpp == 24)
		{
			auto imgWalker = newImage.data();

			for (int curPixel = newImage.getArea() - 1; curPixel >= 0; curPixel--, imgWalker += 4)
			{
				streamReader.read(imgWalker, 3);
				imgWalker[3] = 255;
			}
		}
		else
		{
			streamReader.read(newImage.data(), newImage.getArea() * 4);
		}

		if (header.attrib & (1 << 5))
			newImage.flip();

		return newImage;
	}

	Image<unsigned char, ImageFormatRGBA> newImage(header.width, header.height);
	
	std::unique_ptr<unsigned char[]> palette; //read palette
	{
		auto palSize = header.descriptionlen + header.cmapentries * header.cmapbits / 8;
		if (palSize > 0)
		{
			palette.reset(new unsigned char[palSize]);
			streamReader.read(palette.get(), palSize);
		}
	}

	size_t rawSize = newImage.getArea() * (header.bpp / 8);
	std::unique_ptr<unsigned char[]> rawData = std::unique_ptr<unsigned char[]>(new unsigned char[rawSize]);

	if (header.imagetype & 0x08) //raw data is compressed
	{
		unsigned char v[16];

		size_t dataChannels = header.bpp / 8;
		auto rawWalker = rawData.get();
		while (rawSize > 0)
		{
			size_t c = 0;
			streamReader.read(&c, 0);

			size_t count = (c & 0x7f) + 1;
			rawSize -= count*dataChannels;
			if (c & 0x80)
			{
				streamReader.read(v, dataChannels);

				while (count > 0)
				{
					memcpy(rawWalker, v, dataChannels);
					rawWalker += dataChannels;
					count--;
				}
			}
			else
			{
				count *= dataChannels;

				streamReader.read(rawWalker, count);
				rawWalker += count;
			}
		}
	}
	else
	{
		streamReader.read(rawData.get(), rawSize);
	}

	auto imgWalker = newImage.data();
	auto imgWidth = newImage.width();
	auto imgHeight = newImage.height();

	switch (header.bpp)
	{
	case 8:

		if (palette)
		{
			for (size_t y = 0; y < imgHeight; y++)
			{
				auto rawWalker = rawData.get() + imgWidth * (imgHeight - y - 1);
				for (size_t x = 0; x < imgWidth; x++)
				{
					size_t tempPixel = (*rawWalker) * 3;
					rawWalker++;

					imgWalker[0] = palette[tempPixel + 0];
					imgWalker[1] = palette[tempPixel + 1];
					imgWalker[2] = palette[tempPixel + 2];
					imgWalker[3] = 255;
					imgWalker += 4;
				}
			}
			break;
		}

		for (size_t y = 0; y < imgHeight; y++)
		{
			auto rawWalker = rawData.get() + imgWidth*(imgHeight - y - 1);
			for (size_t x = 0; x < imgWidth; x++)
			{
				imgWalker[0] = imgWalker[1] = imgWalker[2] = *rawWalker;
				imgWalker[3] = 255;
				rawWalker++;
				imgWalker += 4;
			}
		}
		break;

	case 16:
		for (size_t y = 0; y < imgHeight; y++)
		{
			for (size_t x = 0; x < imgWidth; x++)
			{
				__int16 temp = ((__int16 *)rawData.get())[(imgWidth * (imgHeight - y - 1) + x)];
				size_t pixelPos = 4 * (y * imgWidth + x);

				imgWalker[0] = (temp & 0x1F) << 3;
				imgWalker[1] = ((temp >> 5) & 0x1F) << 3;
				imgWalker[2] = ((temp >> 10) & 0x1F) << 3;
				imgWalker[3] = (temp >> 15) ? 255 : 0;
				imgWalker += 4;
			}
		}
		break;

	case 24:
		for (size_t y = 0; y < imgHeight; y++)
		{
			auto rawWalker = rawData.get() + (imgWidth*(imgHeight - y - 1) * 3);
			for (size_t x = 0; x < imgWidth; x++)
			{
				imgWalker[0] = rawWalker[0];
				imgWalker[1] = rawWalker[1];
				imgWalker[2] = rawWalker[2];
				imgWalker[3] = 255;
				rawWalker += 3;
				imgWalker += 4;
			}
		}
		break;

	case 32:
		for (size_t y = 0; y < imgHeight; y++)
		{
			memcpy(imgWalker, rawData.get() + (imgWidth*(imgHeight - y - 1) * 4), imgWidth * 4);
			imgWalker += imgWidth * 4;
		}
		break;
	}

	if (header.attrib & (1 << 5))
		newImage.flip();

	return newImage;
}

Image<unsigned char, ImageFormatRGB> Factory::readJPG(HorseRadish::Streams::StreamReader &streamReader)
{
	stbi_io_callbacks ioCbS;
	ioCbS.read = stbIORead;
	ioCbS.skip = stbIOSkip;
	ioCbS.eof = stbIOEof;

	int imgWidth, imgHeight, imgNumComponents;

	stbi_set_flip_vertically_on_load(1);
	auto imgData = stbi_load_from_callbacks(&ioCbS, &streamReader, &imgWidth, &imgHeight, &imgNumComponents, 3);
	if (imgData == nullptr)
		return Image<unsigned char, ImageFormatRGB>();

	return Image<unsigned char, ImageFormatRGB>(std::unique_ptr<unsigned char[]>(imgData), imgWidth, imgHeight);
}

Image<float, ImageFormatRGB> Factory::readHDRI(HorseRadish::Streams::StreamReader &streamReader)
{
	stbi_io_callbacks ioCbS;
	ioCbS.read = stbIORead;
	ioCbS.skip = stbIOSkip;
	ioCbS.eof = stbIOEof;

	int imgWidth, imgHeight, imgNumComponents;

	stbi_set_flip_vertically_on_load(1);
	auto imgData = stbi_loadf_from_callbacks(&ioCbS, &streamReader, &imgWidth, &imgHeight, &imgNumComponents, 3);
	if (imgData == nullptr)
		return Image<float, ImageFormatRGB>();

	return Image<float, ImageFormatRGB>(std::unique_ptr<float[]>(imgData), imgWidth, imgHeight);
}

} }

