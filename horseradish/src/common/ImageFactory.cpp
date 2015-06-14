#pragma once

#include "common\ImageFactory.hpp"

#include "libs\stb\stb_image.h"
#include "libs\lodepng\lodepng.h"

static
int stbIORead(void *user, char *data, int size)
{
	auto streamReader = static_cast<HorseRadish::Streams::StreamReader*>(user);
	return streamReader->Read(data, size);
}

static
void stbIOSkip(void *user, int n)
{
	auto streamReader = static_cast<HorseRadish::Streams::StreamReader*>(user);
	streamReader->Seek(n);
}

static
int stbIOEof(void *user)
{
	auto streamReader = static_cast<HorseRadish::Streams::StreamReader*>(user);
	return !streamReader->CanRead();
}

namespace HorseRadish { namespace Imaging {

Image<unsigned char, ImageFormatRGB> Factory::readPNG(HorseRadish::Streams::StreamReader &streamReader)
{
	auto streamContent = streamReader.getStream().readEntireContent();

	unsigned int outW = 0, outH = 0;
	unsigned char* outBuffer = nullptr;

	if (lodepng_decode24(&outBuffer, &outW, &outH, static_cast<const unsigned char *>(streamContent->getData()), streamContent->GetLength()) != 0)
		return Image<unsigned char, ImageFormatRGB>();

	if (outBuffer != nullptr)
		return Image<unsigned char, ImageFormatRGB>(std::unique_ptr<unsigned char[]>(outBuffer), outW, outH);

	return Image<unsigned char, ImageFormatRGB>();
}

Image<unsigned char, ImageFormatRGBA> Factory::readPNGWithAlpha(HorseRadish::Streams::StreamReader &streamReader)
{
	auto streamContent = streamReader.getStream().readEntireContent();

	unsigned int outW = 0, outH = 0;
	unsigned char* outBuffer = nullptr;

	if (lodepng_decode32(&outBuffer, &outW, &outH, static_cast<const unsigned char *>(streamContent->getData()), streamContent->GetLength()) != 0)
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

	streamWriter.Write(bufferOut, bufferOutSize);

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

	streamWriter.Write(bufferOut, bufferOutSize);

	free(bufferOut);

	return true;
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

