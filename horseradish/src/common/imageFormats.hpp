#pragma once

namespace HorseRadish { namespace Imaging {

template <class TFormat>
class ImageFormat
{
	static size_t size()
	{
		TFormat::size();
	}

	static bool hasAlpha()
	{
		TFormat::hasAlpha();
	}

	template<typename T>
	static void readRGB(const T* const dataIn, T* const dataOut, const T defaultColorValue)
	{
		TFormat::readRGB<T>(dataIn, dataOut, defaultColorValue);
	}

	template<typename T>
	static void readRGBA(const T* const dataIn, T* const dataOut, const T defaultColorValue, const T defaultAlphaValue)
	{
		TFormat::readRGBA<T>(dataIn, dataOut, defaultColorValue, defaultAlphaValue);
	}

	template<typename T>
	static void writeRGB(T* const dataOut, const T* const pixelValue)
	{
		TFormat::writeRGBA<T>(dataOut, pixelValue);
	}

	template<typename T>
	static void writeRGBA(T* const dataOut, const T* const pixelValue)
	{
		TFormat::writeRGBA<T>(dataOut, pixelValue);
	}
};

struct ImageFormatR : ImageFormat<ImageFormatR>
{
	static size_t size()
	{
		return 1;
	}

	static bool hasAlpha()
	{
		return false;
	}

	template<typename T>
	static void readRGB(const T* const dataIn, T* const dataOut, const T defaultColorValue)
	{
		dataOut[0] = dataIn[0];
		dataOut[1] = dataOut[2] = defaultColorValue;
	}

	template<typename T>
	static void readRGBA(const T* const dataIn, T* const dataOut, const T defaultColorValue, const T defaultAlphaValue)
	{
		dataOut[0] = dataIn[0];
		dataOut[1] = dataOut[2] = defaultColorValue;
		dataOut[3] = defaultAlphaValue;
	}

	template<typename T>
	static void writeRGB(T* const dataOut, const T* const pixelValue)
	{
		dataOut[0] = pixelValue[0];
	}

	template<typename T>
	static void writeRGBA(T* const dataOut, const T* const pixelValue)
	{
		dataOut[0] = pixelValue[0];
	}
};

struct ImageFormatRA : ImageFormat<ImageFormatR>
{
	static size_t size()
	{
		return 2;
	}

	static bool hasAlpha()
	{
		return true;
	}

	template<typename T>
	static void readRGB(const T* const dataIn, T* const dataOut, const T defaultColorValue)
	{
		dataOut[0] = dataIn[0];
		dataOut[1] = dataOut[2] = defaultColorValue;
	}

	template<typename T>
	static void readRGBA(const T* const dataIn, T* const dataOut, const T defaultColorValue, const T defaultAlphaValue)
	{
		dataOut[0] = dataIn[0];
		dataOut[1] = dataOut[2] = defaultColorValue;
		dataOut[3] = dataIn[1];
	}

	template<typename T>
	static void writeRGB(T* const dataOut, const T* const pixelValue)
	{
		dataOut[0] = pixelValue[0];
	}

	template<typename T>
	static void writeRGBA(T* const dataOut, const T* const pixelValue)
	{
		dataOut[0] = pixelValue[0];
		dataOut[1] = pixelValue[3];
	}
};

struct ImageFormatRGB : ImageFormat<ImageFormatRGB>
{
	static size_t size()
	{
		return 3;
	}

	static bool hasAlpha()
	{
		return false;
	}

	template<typename T>
	static void readRGB(const T* const dataIn, T* const dataOut, const T defaultColorValue)
	{
		std::memcpy(dataOut, dataIn, sizeof(T) * 3);
	}

	template<typename T>
	static void readRGBA(const T* const dataIn, T* const dataOut, const T defaultColorValue, const T defaultAlphaValue)
	{
		std::memcpy(dataOut, dataIn, sizeof(T) * 3);
		dataOut[3] = defaultAlphaValue;
	}

	template<typename T>
	static void writeRGB(T* const dataOut, const T* const pixelValue)
	{
		std::memcpy(dataOut, pixelValue, sizeof(T) * 3);
	}

	template<typename T>
	static void writeRGBA(T* const dataOut, const T* const pixelValue)
	{
		std::memcpy(dataOut, pixelValue, sizeof(T) * 3);
	}
};

struct ImageFormatRGBA : ImageFormat<ImageFormatRGBA>
{
	static size_t size()
	{
		return 4;
	}

	static bool hasAlpha()
	{
		return true;
	}

	template<typename T>
	static void readRGB(const T* const dataIn, T* const dataOut, const T defaultColorValue)
	{
		std::memcpy(dataOut, dataIn, sizeof(T) * 3);
	}

	template<typename T>
	static void readRGBA(const T* const dataIn, T* const dataOut, const T defaultColorValue, const T defaultAlphaValue)
	{
		std::memcpy(dataOut, dataIn, sizeof(T) * 4);
	}

	template<typename T>
	static void writeRGB(T* const dataOut, const T* const pixelValue)
	{
		std::memcpy(dataOut, pixelValue, sizeof(T) * 3);
	}

	template<typename T>
	static void writeRGBA(T* const dataOut, const T* const pixelValue)
	{
		std::memcpy(dataOut, pixelValue, sizeof(T) * 4);
	}
};

} }

