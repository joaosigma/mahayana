#pragma once

#include "common/imageFactory.hpp"

#include "libs/stb/stb_image.h"
#include "libs/lodepng/lodepng.h"
#include "libs/tinyexr/tinyexr.h"

#include <memory>

namespace hr { namespace imaging
{
	namespace
	{
		int stbIORead(void *user, char *data, int size)
		{
			auto streamReader = static_cast<hr::streams::StreamReader*>(user);
			return streamReader->read(data, size);
		}

		void stbIOSkip(void *user, int n)
		{
			auto streamReader = static_cast<hr::streams::StreamReader*>(user);
			streamReader->skip(n);
		}

		int stbIOEof(void *user)
		{
			auto streamReader = static_cast<hr::streams::StreamReader*>(user);
			return !streamReader->canRead();
		}
	}

	Image<unsigned char, ImageFormatRGB> Factory::readPNG(const hr::streams::StreamReader &streamReader)
	{
		hr::streams::MemoryViewStream streamContent;
		streamReader.stream().cloneAllContent(streamContent);

		unsigned int outW = 0, outH = 0;
		unsigned char* outBuffer = nullptr;

		if (lodepng_decode24(&outBuffer, &outW, &outH, static_cast<const unsigned char *>(streamContent.data()), streamContent.length()) != 0)
			return Image<unsigned char, ImageFormatRGB>();

		if (outBuffer)
			return Image<unsigned char, ImageFormatRGB>(std::unique_ptr<unsigned char[]>(outBuffer), outW, outH);

		return Image<unsigned char, ImageFormatRGB>();
	}

	Image<unsigned char, ImageFormatRGBA> Factory::readPNGWithAlpha(const hr::streams::StreamReader &streamReader)
	{
		hr::streams::MemoryViewStream streamContent;
		streamReader.stream().cloneAllContent(streamContent);

		unsigned int outW = 0, outH = 0;
		unsigned char* outBuffer = nullptr;

		if (lodepng_decode32(&outBuffer, &outW, &outH, static_cast<const unsigned char *>(streamContent.data()), streamContent.length()) != 0)
			return Image<unsigned char, ImageFormatRGBA>();

		if (outBuffer)
			return Image<unsigned char, ImageFormatRGBA>(std::unique_ptr<unsigned char[]>(outBuffer), outW, outH);

		return Image<unsigned char, ImageFormatRGBA>();
	}

	bool Factory::savePNG(hr::streams::StreamWriter &streamWriter, const ImageView<unsigned char, ImageFormatRGB>& imgView)
	{
		if (imgView.empty())
			return false;

		unsigned char *bufferOut = nullptr;
		size_t bufferOutSize = 0;
		lodepng_encode24(&bufferOut, &bufferOutSize, imgView.data(), imgView.width(), imgView.height());

		if (!bufferOut || (bufferOutSize <= 0))
			return false;

		streamWriter.write(bufferOut, bufferOutSize);

		free(bufferOut);

		return true;
	}

	bool Factory::savePNG(hr::streams::StreamWriter &streamWriter, const ImageView<unsigned char, ImageFormatRGBA>& imgView)
	{
		if (imgView.empty())
			return false;

		unsigned char *bufferOut = nullptr;
		size_t bufferOutSize = 0;
		lodepng_encode32(&bufferOut, &bufferOutSize, imgView.data(), imgView.width(), imgView.height());

		if (!bufferOut || (bufferOutSize <= 0))
			return false;

		streamWriter.write(bufferOut, bufferOutSize);

		free(bufferOut);

		return true;
	}

	Image<unsigned char, ImageFormatRGBA> Factory::readTGA(hr::streams::StreamReader &streamReader)
	{
		stbi_io_callbacks ioCbS;
		ioCbS.read = stbIORead;
		ioCbS.skip = stbIOSkip;
		ioCbS.eof = stbIOEof;

		int imgWidth, imgHeight, imgNumComponents;

		auto imgData = stbi_load_from_callbacks(&ioCbS, &streamReader, &imgWidth, &imgHeight, &imgNumComponents, 4);
		if (!imgData)
			return Image<unsigned char, ImageFormatRGBA>();

		return Image<unsigned char, ImageFormatRGBA>(std::unique_ptr<unsigned char[]>(imgData), imgWidth, imgHeight);
	}

	Image<unsigned char, ImageFormatRGB> Factory::readJPG(hr::streams::StreamReader &streamReader)
	{
		stbi_io_callbacks ioCbS;
		ioCbS.read = stbIORead;
		ioCbS.skip = stbIOSkip;
		ioCbS.eof = stbIOEof;

		int imgWidth, imgHeight, imgNumComponents;

		auto imgData = stbi_load_from_callbacks(&ioCbS, &streamReader, &imgWidth, &imgHeight, &imgNumComponents, 3);
		if (!imgData)
			return Image<unsigned char, ImageFormatRGB>();

		return Image<unsigned char, ImageFormatRGB>(std::unique_ptr<unsigned char[]>(imgData), imgWidth, imgHeight);
	}

	Image<float, ImageFormatRGB> Factory::readHDRI(hr::streams::StreamReader &streamReader)
	{
		stbi_io_callbacks ioCbS;
		ioCbS.read = stbIORead;
		ioCbS.skip = stbIOSkip;
		ioCbS.eof = stbIOEof;

		int imgWidth, imgHeight, imgNumComponents;

		auto imgData = stbi_loadf_from_callbacks(&ioCbS, &streamReader, &imgWidth, &imgHeight, &imgNumComponents, 3);
		if (!imgData)
			return Image<float, ImageFormatRGB>();

		return Image<float, ImageFormatRGB>(std::unique_ptr<float[]>(imgData), imgWidth, imgHeight);
	}

	Image<float, ImageFormatRGBA> Factory::readEXR(const char* const fileName)
	{
		float* bufferOut = nullptr; // width * height * RGBA
		int width, height;
		const char* err;

		if (LoadEXR(&bufferOut, &width, &height, fileName, &err) != TINYEXR_SUCCESS)
		{
			if (bufferOut)
				free(bufferOut);
			return Image<float, ImageFormatRGBA>();
		}

		return Image<float, ImageFormatRGBA>(std::unique_ptr<float[]>(bufferOut), width, height);
	}
} }
