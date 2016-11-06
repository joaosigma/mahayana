#pragma once

#include "color.hpp"
#include "imageFormats.hpp"
#include "libs/stb/stb_image_resize.h"

#include <cassert>
#include <type_traits>

namespace HorseRadish { namespace Imaging
{
	template<typename TDataType, typename TDataFormat>
	class Image;

	template<typename TDataType, typename TDataFormat, typename TDerived>
	class ImageViewBase
	{
		static_assert(std::is_arithmetic<TDataType>::value, "Data type must be arithmetic (e.g.: float, unsigned char, etc.)");
		static_assert(std::is_base_of<ImageFormat<TDataFormat>, TDataFormat>::value, "Data format must inherit from type ImageFormat");

		template<typename T1, typename T2>
		friend class ImageView;
		template<typename T1, typename T2>
		friend class ImageBase;
		template<typename T1, typename T2>
		friend class Image;

	public:
		ImageViewBase(TDataType* const data, size_t width, size_t height)
			: mDataPtr(data), mWidth(width), mHeight(height)
		{ }

		ImageViewBase(const ImageViewBase&) = delete;
		ImageViewBase& operator=(const ImageViewBase&) = delete;

		ImageViewBase(ImageViewBase&& imgView)
		{ 
			*this = std::move(imgView);
		}

		ImageViewBase& operator=(ImageViewBase&& imgView)
		{
			if (this != &imgView)
			{
				std::swap(mDataPtr, imgView.mDataPtr);
				std::swap(mWidth, imgView.mWidth);
				std::swap(mHeight, imgView.mHeight);
			}

			return *this;
		}

		const TDataType* data() const
		{
			assert(mDataPtr);
			return mDataPtr;
		}

		size_t width() const
		{
			return mWidth;
		}

		size_t height() const
		{
			return mHeight;
		}

		bool empty() const
		{
			return (getArea() == 0);
		}

		size_t getArea() const
		{
			return mWidth * mHeight;
		}

		size_t getSize() const
		{
			return (mWidth * mHeight * TDataFormat::size() * sizeof(TDataType));
		}

		size_t getRowSize() const
		{
			return (mWidth * TDataFormat::size() * sizeof(TDataType));
		}

		size_t getPixelSize() const
		{
			return (TDataFormat::size() * sizeof(TDataType));
		}

		Image<TDataType, TDataFormat> crop(size_t cropX, size_t cropY, size_t cropWidth, size_t cropHeight) const
		{
			if (empty() || ((cropWidth * cropHeight) <= 0) || ((cropX + cropWidth) > mWidth) || ((cropY + cropHeight) > mHeight))
				return Image<TDataType, TDataFormat>();

			Image<TDataType, TDataFormat> newImg(cropWidth, cropHeight);

			auto srcRowSize = width() * TDataFormat::size();
			auto destRowSize = cropWidth * TDataFormat::size();

			for (size_t curY = 0; curY < cropHeight; curY++)
			{
				auto sourcePos = ((curY + cropY) * srcRowSize) + (cropX * TDataFormat::size());
				auto destPos = curY * destRowSize;
				
				memcpy(newImg.mDataPtr + destPos, mDataPtr + sourcePos, destRowSize * sizeof(TDataType));
			}

			return newImg;
		}

	private:
		ImageViewBase()
		{ }

		size_t getPos(const size_t x, const size_t y) const
		{
			return ((y * mWidth * TDataFormat::size()) + (x * TDataFormat::size()));
		}

	private:
		TDataType* mDataPtr = nullptr;
		size_t mWidth = 0, mHeight = 0;
	};

	template<typename TDataType, typename TDataFormat> // generic ImageView (any type supported)
	class ImageView : public ImageViewBase<TDataType, TDataFormat, ImageView<TDataType, TDataFormat>>
	{
		typedef ImageViewBase<TDataType, TDataFormat, ImageView<TDataType, TDataFormat>> BaseType;

		template<typename T1, typename T2>
		friend class ImageBase;
		template<typename T1, typename T2>
		friend class Image;

	public:
		ImageView(TDataType* const data, size_t width, size_t height)
			: BaseType(data, width, height)
		{ }

		ImageView(ImageView&& imgView) = default;
		ImageView& operator=(ImageView&& imgView) = default;

		Image<TDataType, TDataFormat> clone() const
		{
			Image<TDataType, TDataFormat> newImg(mWidth, mHeight);

			std::memcpy(newImg.mDataPtr, mDataPtr, getSize());
			return newImg;
		}

		void getPixel(const size_t x, const size_t y, TDataType* const pixelValue, const TDataType defaultColorValue, const TDataType defaultAlphaValue) const
		{
			assert(mDataPtr);
			assert(pixelValue);

			TDataFormat::readRGBA<TDataType>(mDataPtr + getPos(x, y), pixelValue, defaultColorValue, defaultAlphaValue);
		}

	private:
		ImageView()
		{ }
	};

	template<typename TDataFormat> // unsigned char specialization
	class ImageView<unsigned char, TDataFormat> : public ImageViewBase<unsigned char, TDataFormat, ImageView<unsigned char, TDataFormat>>
	{
		typedef ImageViewBase<unsigned char, TDataFormat, ImageView<unsigned char, TDataFormat>> BaseType;

		template<typename T1, typename T2>
		friend class ImageBase;
		template<typename T1, typename T2>
		friend class Image;

		void convertTo(float* dataOut, const size_t pos, const size_t count) const
		{
			assert(mDataPtr);

			size_t curPos = 0;
			auto walker = mDataPtr + pos;
			auto countBlock = (count / 4) * 4;

			for (; curPos < countBlock; curPos += 4, walker += 4, dataOut += 4)
				HorseRadish::Color::ConvertColor(dataOut, walker, true);

			for (; curPos < count; curPos++, walker++, dataOut++)
				*dataOut = HorseRadish::Color::ConvertColor(*walker);
		}

	public:
		ImageView(unsigned char* const data, size_t width, size_t height)
			: BaseType(data, width, height)
		{ }

		ImageView(ImageView&& imgView) = default;
		ImageView& operator=(ImageView&& imgView) = default;

		void getPixel(const size_t x, const size_t y, HorseRadish::Color &pixelValue) const
		{
			assert(mDataPtr);

			unsigned char tmpPixel[4];

			TDataFormat::readRGBA<unsigned char>(mDataPtr + getPos(x, y), tmpPixel, 0, 255);
			pixelValue.Set(tmpPixel);
		}

		void getPixel(const size_t x, const size_t y, unsigned char* const pixelValue) const
		{
			assert(mDataPtr);
			assert(pixelValue);

			TDataFormat::readRGBA<unsigned char>(mDataPtr + getPos(x, y), pixelValue, 0, 255);
		}

		template<typename TNewDataType = unsigned char, typename std::enable_if<std::is_same<TNewDataType, unsigned char>::value>::type* = nullptr>
		Image<unsigned char, TDataFormat> clone() const
		{
			Image<unsigned char, TDataFormat> newImg(mWidth, mHeight);

			std::memcpy(newImg.mDataPtr, mDataPtr, getSize());
			return newImg;
		}

		template<typename TNewDataType = unsigned char, typename std::enable_if<!std::is_same<TNewDataType, unsigned char>::value>::type* = nullptr>
		Image<TNewDataType, TDataFormat> clone() const
		{
			Image<TNewDataType, TDataFormat> newImg(mWidth, mHeight);

			convertTo(newImg.mDataPtr, 0, getArea() * TDataFormat::size());
			return newImg;
		}

		Image<unsigned char, TDataFormat> resize(size_t width, size_t height) const
		{
			Image<unsigned char, TDataFormat> newImg(width, height);

			stbir_resize_uint8_generic(mDataPtr, BaseType::width(), BaseType::height(), 0,
				newImg.mDataPtr, width, height, 0,
				TDataFormat::size(), STBIR_ALPHA_CHANNEL_NONE,
				STBIR_FLAG_ALPHA_PREMULTIPLIED, STBIR_EDGE_CLAMP, STBIR_FILTER_DEFAULT, STBIR_COLORSPACE_LINEAR, nullptr);

			return newImg;
		}

	private:
		ImageView()
		{ }
	};

	template<typename TDataFormat> // float specialization
	class ImageView<float, TDataFormat> : public ImageViewBase<float, TDataFormat, ImageView<float, TDataFormat>>
	{
		typedef ImageViewBase<float, TDataFormat, ImageView<float, TDataFormat>> BaseType;

		template<typename T1, typename T2>
		friend class ImageBase;
		template<typename T1, typename T2>
		friend class Image;

		void convertTo(unsigned char* dataOut, const size_t pos, const size_t count) const
		{
			assert(mDataPtr);

			size_t curPos = 0;
			auto walker = mDataPtr + pos;
			auto countBlock = (count / 4) * 4;

			for (; curPos < countBlock; curPos += 4, walker += 4, dataOut += 4)
				HorseRadish::Color::ConvertColor(dataOut, walker, true);

			for (; curPos < count; curPos++, walker++, dataOut++)
				*dataOut = HorseRadish::Color::ConvertColor(*walker);
		}

	public:
		ImageView(float* const data, size_t width, size_t height)
			: BaseType(data, width, height)
		{ }

		ImageView(ImageView&& imgView) = default;
		ImageView& operator=(ImageView&& imgView) = default;

		void getPixel(const size_t x, const size_t y, HorseRadish::Color &pixelValue) const
		{
			assert(mDataPtr);

			float tmpPixel[4];

			TDataFormat::readRGBA<float>(mDataPtr + getPos(x, y), tmpPixel, 0.0f, 1.0f);
			pixelValue.Set(tmpPixel);
		}

		void getPixel(const size_t x, const size_t y, float* const pixelValue) const
		{
			assert(mDataPtr);
			assert(pixelValue);

			TDataFormat::readRGBA<float>(mDataPtr + getPos(x, y), pixelValue, 0.0f, 1.0f);
		}

		template<typename TNewDataType = float, typename std::enable_if<std::is_same<TNewDataType, float>::value>::type* = nullptr>
		Image<float, TDataFormat> clone() const
		{
			Image<float, TDataFormat> newImg(mWidth, mHeight);

			std::memcpy(newImg.mDataPtr, mDataPtr, getSize());
			return newImg;
		}

		template<typename TNewDataType = float, typename std::enable_if<!std::is_same<TNewDataType, float>::value>::type* = nullptr>
		Image<TNewDataType, TDataFormat> clone() const
		{
			Image<TNewDataType, TDataFormat> newImg(mWidth, mHeight);

			convertTo(newImg.mDataPtr, 0, getArea() * TDataFormat::size());
			return newImg;
		}

		Image<float, TDataFormat> resize(size_t width, size_t height) const
		{
			Image<float, TDataFormat> newImg(width, height);

			stbir_resize_float_generic(mDataPtr, BaseType::width(), BaseType::height(), 0,
				newImg.mDataPtr, width, height, 0,
				TDataFormat::size(), STBIR_ALPHA_CHANNEL_NONE,
				STBIR_FLAG_ALPHA_PREMULTIPLIED, STBIR_EDGE_CLAMP, STBIR_FILTER_DEFAULT, STBIR_COLORSPACE_LINEAR, nullptr);

			return newImg;
		}

	private:
		ImageView()
		{ }
	};

	template<typename TDataType, typename TDataFormat>
	class ImageBase : public ImageView<TDataType, TDataFormat>
	{
		template<typename T1, typename T2>
		friend class Image;

	public:
		ImageBase(const ImageBase&) = delete;
		ImageBase& operator=(const ImageBase&) = delete;
		ImageBase(ImageBase&& img) = default;
		ImageBase& operator=(ImageBase&& img) = default;

		TDataType* data()
		{
			assert(mDataPtr);
			return mDataPtr;
		}

		void flip()
		{
			std::unique_ptr<TDataType[]> tempRow(new TDataType[mWidth * TDataFormat::size()]);

			auto rowSize = width() * TDataFormat::size();
			auto numRows = height() / 2;

			auto bottomPtr = mDataPtr;
			auto topPtr = mDataPtr + ((height() - 1) * rowSize);

			for (size_t curY = 0; curY < numRows; curY++)
			{
				memcpy(tempRow.get(), topPtr, rowSize * sizeof(TDataType));
				memcpy(topPtr, bottomPtr, rowSize * sizeof(TDataType));
				memcpy(bottomPtr, tempRow.get(), rowSize * sizeof(TDataType));

				bottomPtr += rowSize;
				topPtr -= rowSize;
			}
		}

		bool setPixelRegion(const ImageView<TDataType, TDataFormat>& imgView, const size_t offsetX, const size_t offsetY, const bool flipSource = false)
		{
			if (((offsetX + imgView.width()) > mWidth) || ((offsetY + imgView.height()) > mHeight))
				return false;

			auto destRowSize = width() * TDataFormat::size();
			auto srcRowSize = imgView.width() * TDataFormat::size();

			if (!flipSource)
			{
				for (size_t curY = 0; curY < imgView.height(); curY++)
				{
					auto destY = ((offsetY + curY) * destRowSize) + (offsetX * TDataFormat::size());
					auto sourceY = (curY * srcRowSize);
					memcpy(mDataPtr + destY, imgView.mDataPtr + sourceY, srcRowSize * sizeof(TDataType));
				}
			}
			else
			{
				for (size_t curY = 0; curY < imgView.height(); curY++)
				{
					auto destY = ((offsetY + curY) * destRowSize) + (offsetX * TDataFormat::size());
					auto sourceY = (imgView.height() - curY - 1) * srcRowSize;
					memcpy(mDataPtr + destY, imgView.mDataPtr + sourceY, srcRowSize * sizeof(TDataType));
				}
			}

			return true;
		}

	private:
		ImageBase()
			: ImageView(nullptr, 0, 0)
		{ }

		ImageBase(size_t width, size_t height)
			: mDataSource(new TDataType[width * height * TDataFormat::size()]), ImageView(nullptr, width, height)
		{
			mDataPtr = mDataSource.get();
		}

		ImageBase(std::unique_ptr<TDataType[]> data, size_t width, size_t height)
			: mDataSource(std::move(data)), ImageView(nullptr, width, height)
		{
			mDataPtr = mDataSource.get();
		}

	private:
		std::unique_ptr<TDataType[]> mDataSource;
	};

	template<typename TDataType, typename TDataFormat> // generic Image (any type supported)
	class Image : public ImageBase<TDataType, TDataFormat>
	{
		typedef ImageBase<TDataType, TDataFormat> BaseType;

		template<typename T1, typename T2>
		friend class ImageView;

	public:
		Image()
		{ }

		Image(size_t width, size_t height)
			: BaseType(width, height)
		{ }

		Image(std::unique_ptr<TDataType[]> data, size_t width, size_t height)
			: BaseType(data, width, height)
		{ }

		Image(const Image&) = delete;
		Image& operator=(const Image&) = delete;
		Image(Image&& img) = default;
		Image& operator=(Image&& img) = default;
	};

	template<typename TDataFormat> // unsigned char specialization
	class Image<unsigned char, TDataFormat> : public ImageBase<unsigned char, TDataFormat>
	{
		typedef ImageBase<unsigned char, TDataFormat> BaseType;

		template<typename T1, typename T2>
		friend class ImageView;

	public:
		Image()
		{ }

		Image(size_t width, size_t height)
			: BaseType(width, height)
		{ }

		Image(std::unique_ptr<unsigned char[]> data, size_t width, size_t height)
			: BaseType(std::move(data), width, height)
		{ }

		Image(const Image&) = delete;
		Image& operator=(const Image&) = delete;
		Image(Image&& img) = default;
		Image& operator=(Image&& img) = default;

		void clear(const unsigned char r, const unsigned char g, const unsigned char b, const unsigned char a)
		{
			unsigned char tmpPixel[] = { r, g, b, a };

			size_t curPos = 0;
			auto walkerPtr = mDataPtr;

			auto imgArea = getArea();
			auto imgAreaBlock = (imgArea / 4) * 4;

			for (; curPos < imgAreaBlock; curPos += 4)
			{
				TDataFormat::writeRGBA(walkerPtr + (0 * TDataFormat::size()), tmpPixel);
				TDataFormat::writeRGBA(walkerPtr + (1 * TDataFormat::size()), tmpPixel);
				TDataFormat::writeRGBA(walkerPtr + (2 * TDataFormat::size()), tmpPixel);
				TDataFormat::writeRGBA(walkerPtr + (3 * TDataFormat::size()), tmpPixel);
				walkerPtr += (TDataFormat::size() * 4);
			}

			for (; curPos < imgArea; curPos++)
			{
				TDataFormat::writeRGBA(walkerPtr, tmpPixel);
				walkerPtr += TDataFormat::size();
			}
		}

		void multiplyAlpha()
		{
			if (!TDataFormat::hasAlpha())
				return;

			auto walkerPtr = mDataPtr;
			auto imgArea = getArea();

			HorseRadish::Color pixelValue;
			unsigned char tmpPixel[4];

			for (size_t curPos = 0; curPos < imgArea; curPos++)
			{
				TDataFormat::readRGBA<unsigned char>(walkerPtr, tmpPixel, 0, 255);
				if (tmpPixel[3] != 255)
				{
					pixelValue.Set(tmpPixel);
					pixelValue.WeightRGB(pixelValue.a);
					pixelValue.Write(tmpPixel);

					TDataFormat::writeRGB<unsigned char>(walkerPtr, tmpPixel);
				}

				walkerPtr += TDataFormat::size();
			}
		}

		void removeGamma()
		{
			if (!TDataFormat::hasAlpha())
			{
				auto walkerPtr = mDataPtr;
				auto walkerEnd = walkerPtr + (getArea() * TDataFormat::size());

				for (; walkerPtr < walkerEnd; walkerPtr++)
					*walkerPtr = Color::gammaCorrect(*walkerPtr);
			}
			else
			{
				auto walkerPtr = mDataPtr;
				auto imgArea = getArea();

				unsigned char tmpPixel[4];
				for (size_t curPos = 0; curPos < imgArea; curPos++)
				{
					TDataFormat::readRGB<unsigned char>(walkerPtr, tmpPixel, 0);
					tmpPixel[0] = Color::gammaCorrect(tmpPixel[0]);
					tmpPixel[1] = Color::gammaCorrect(tmpPixel[1]);
					tmpPixel[2] = Color::gammaCorrect(tmpPixel[2]);
					TDataFormat::writeRGB<unsigned char>(walkerPtr, tmpPixel);

					walkerPtr += TDataFormat::size();
				}
			}
		}

		void setPixel(const size_t x, const size_t y, HorseRadish::Color &pixelValue)
		{
			unsigned char tmpPixel[4];
			HorseRadish::Color::ConvertColor(HorseRadish::Color(r, g, b, a), tmpPixel, true);

			TDataFormat::writeRGBA(mDataPtr + getPos(x, y), tmpPixel);
		}

		void setPixel(const size_t x, const size_t y, const unsigned char* const pixelValue)
		{
			TDataFormat::writeRGBA(mDataPtr + getPos(x, y), pixelValue);
		}

		void transform(std::function<bool(HorseRadish::Color&)> cb)
		{
			if (empty() || !cb)
				return;

			auto walkerPtr = mDataPtr;
			auto imgArea = getArea();

			unsigned char tmpPixel[4];
			HorseRadish::Color pixelValue;

			for (size_t curPos = 0; curPos < imgArea; curPos++)
			{
				TDataFormat::readRGBA<unsigned char>(walkerPtr, tmpPixel, 0, 255);

				pixelValue.Set(tmpPixel);
				if (cb(pixelValue))
				{
					pixelValue.Write(tmpPixel);
					TDataFormat::writeRGBA<unsigned char>(walkerPtr, tmpPixel);
				}
				
				walkerPtr += TDataFormat::size();
			}
		}
	};

	template<typename TDataFormat> // float specialization
	class Image<float, TDataFormat> : public ImageBase<float, TDataFormat>
	{
		typedef ImageBase<float, TDataFormat> BaseType;

		template<typename T1, typename T2>
		friend class ImageView;

	public:
		Image()
		{ }

		Image(size_t width, size_t height)
			: BaseType(width, height)
		{ }

		Image(std::unique_ptr<float[]> data, size_t width, size_t height)
			: BaseType(std::move(data), width, height)
		{ }

		Image(const Image&) = delete;
		Image& operator=(const Image&) = delete;
		Image(Image&& img) = default;
		Image& operator=(Image&& img) = default;

		void clear(const float r, const float g, const float b, const float a)
		{
			float tmpPixel[] = { r, g, b, a };

			size_t curPos = 0;
			auto walkerPtr = mDataPtr;

			auto imgArea = getArea();
			auto imgAreaBlock = (imgArea / 4) * 4;

			for (; curPos < imgAreaBlock; curPos += 4)
			{
				TDataFormat::writeRGBA(walkerPtr + (0 * TDataFormat::size()), tmpPixel);
				TDataFormat::writeRGBA(walkerPtr + (1 * TDataFormat::size()), tmpPixel);
				TDataFormat::writeRGBA(walkerPtr + (2 * TDataFormat::size()), tmpPixel);
				TDataFormat::writeRGBA(walkerPtr + (3 * TDataFormat::size()), tmpPixel);
				walkerPtr += (TDataFormat::size() * 4);
			}

			for (; curPos < imgArea; curPos++)
			{
				TDataFormat::writeRGBA(walkerPtr, tmpPixel);
				walkerPtr += TDataFormat::size();
			}
		}

		void setPixel(const size_t x, const size_t y, HorseRadish::Color &pixelValue)
		{
			TDataFormat::writeRGBA(mDataPtr + getPos(x, y), pixelValue);
		}

		void setPixel(const size_t x, const size_t y, const float* const pixelValue)
		{
			TDataFormat::writeRGBA(mDataPtr + getPos(x, y), pixelValue);
		}
	};
} }
