#pragma once

#include "color.hpp"
#include "imageFormats.hpp"
#include "libs/stb/stb_image_resize.h"

#include <cassert>
#include <type_traits>

namespace hr::imaging
{
	//forward declaration of the two main types
	template<typename TDataType, typename TDataFormat> class Image;
	template<typename TDataType, typename TDataFormat> class ImageView;

	template<typename TDataType, typename TDataFormat>
	class ImageViewBase
	{
		static_assert(std::is_arithmetic_v<TDataType>, "Data type must be arithmetic (e.g.: float, uint8_t, etc.)");
		static_assert(std::is_base_of_v<ImageFormat<TDataFormat>, TDataFormat>, "Data format must inherit from type ImageFormat");

		template<typename, typename> friend class Image;
		template<typename, typename> friend class ImageBase;
		template<typename, typename> friend class ImageView;
		template<typename, typename> friend class ImageViewBase;

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

		void getPixel(const size_t x, const size_t y, TDataType* const pixelValue, const TDataType defaultColorValue, const TDataType defaultAlphaValue) const
		{
			assert(mDataPtr);
			assert(pixelValue);

			TDataFormat::readRGBA<TDataType>(mDataPtr + getPos(x, y), pixelValue, defaultColorValue, defaultAlphaValue);
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

		Image<TDataType, TDataFormat> clone() const
		{
			Image<TDataType, TDataFormat> newImg(mWidth, mHeight);

			std::memcpy(newImg.mDataPtr, mDataPtr, getSize());
			return newImg;
		}

		template<typename TNewDataType, typename TNewDataFormat>
		Image<TNewDataType, TNewDataFormat> convert(const TDataType defaultColorValue, const TDataType defaultAlphaValue) const
		{
			static_assert(std::is_base_of_v<ImageFormat<TNewDataFormat>, TNewDataFormat>, "Data format must inherit from type ImageFormat");

			//special clone case
			if constexpr (std::is_same_v<TDataType, TNewDataType>&& std::is_same_v<TDataFormat, TNewDataFormat>)
			{
				return clone();
			}
			else
			{
				Image<TNewDataType, TNewDataFormat> newImg(mWidth, mHeight);

				//for the same data type
				if constexpr (std::is_same_v<TDataType, TNewDataType>)
				{
					TDataType pixel[4];
					for (size_t y = 0; y < mHeight; ++y)
					{
						for (size_t x = 0; x < mWidth; ++x)
						{
							TDataFormat::readRGBA<TDataType>(mDataPtr + getPos(x, y), pixel, defaultColorValue, defaultAlphaValue);
							TNewDataFormat::writeRGBA<TNewDataType>(newImg.mDataPtr + newImg.getPos(x, y), pixel);
						}
					}
				}
				//for float <-> uint8_t conversion
				else if constexpr (
					(std::is_same_v<TDataType, uint8_t>&& std::is_same_v<TNewDataType, float>) ||
					(std::is_same_v<TDataType, float> && std::is_same_v<TNewDataType, uint8_t>))
				{
					TDataType pixelIn[4];
					TNewDataType pixelOut[4];
					for (size_t y = 0; y < mHeight; ++y)
					{
						for (size_t x = 0; x < mWidth; ++x)
						{
							TDataFormat::readRGBA<TDataType>(mDataPtr + getPos(x, y), pixelIn, defaultColorValue, defaultAlphaValue);
							hr::Color::convertColor(pixelOut, pixelIn, true);
							TNewDataFormat::writeRGBA<TNewDataType>(newImg.mDataPtr + newImg.getPos(x, y), pixelOut);
						}
					}
				}
				//all other convertions are (currently) not supported
				else
				{
					static_assert(std::false_type::value, "Can't convert between data types");
				}

				return newImg;
			}
		}

	private:
		ImageViewBase() = default;

		size_t getPos(const size_t x, const size_t y) const
		{
			return ((y * mWidth * TDataFormat::size()) + (x * TDataFormat::size()));
		}

	private:
		TDataType* mDataPtr = nullptr;
		size_t mWidth = 0, mHeight = 0;
	};

	template<typename TDataType, typename TDataFormat> // generic ImageView (any type supported)
	class ImageView
		: public ImageViewBase<TDataType, TDataFormat>
	{
		using BaseType = ImageViewBase<TDataType, TDataFormat>;

		template<typename, typename> friend class Image;
		template<typename, typename> friend class ImageBase;
		template<typename, typename> friend class ImageView;
		template<typename, typename> friend class ImageViewBase;

	public:
		ImageView(TDataType* const data, size_t width, size_t height)
			: BaseType(data, width, height)
		{ }

		ImageView(ImageView&& imgView) = default;
		ImageView& operator=(ImageView&& imgView) = default;

	private:
		ImageView() = default;
	};

	template<typename TDataFormat> // uint8_t specialization
	class ImageView<uint8_t, TDataFormat>
		: public ImageViewBase<uint8_t, TDataFormat>
	{
		using BaseType = ImageViewBase<uint8_t, TDataFormat>;

		template<typename, typename> friend class Image;
		template<typename, typename> friend class ImageBase;
		template<typename, typename> friend class ImageView;
		template<typename, typename> friend class ImageViewBase;

	public:
		ImageView(uint8_t* const data, size_t width, size_t height)
			: BaseType(data, width, height)
		{ }

		ImageView(ImageView&& imgView) = default;
		ImageView& operator=(ImageView&& imgView) = default;

		void getPixel(const size_t x, const size_t y, hr::Color& pixelValue) const
		{
			assert(mDataPtr);

			uint8_t tmpPixel[4];

			TDataFormat::readRGBA<uint8_t>(mDataPtr + getPos(x, y), tmpPixel, 0, 255);
			pixelValue.Set(tmpPixel);
		}

		void getPixel(const size_t x, const size_t y, uint8_t* const pixelValue) const
		{
			assert(mDataPtr);
			assert(pixelValue);

			TDataFormat::readRGBA<uint8_t>(mDataPtr + getPos(x, y), pixelValue, 0, 255);
		}

		template<typename TNewDataType, typename TNewDataFormat>
		Image<TNewDataType, TNewDataFormat> convert(const uint8_t defaultColorValue, const uint8_t defaultAlphaValue) const
		{
			static_assert(std::is_base_of<ImageFormat<TNewDataFormat>, TNewDataFormat>::value, "Data format must inherit from type ImageFormat");

			// { uint8_t -> float, same format } special case
			if constexpr (std::is_same_v<float, TNewDataType>&& std::is_same_v<TDataFormat, TNewDataFormat>)
			{
				Image<float, TDataFormat> newImg(mWidth, mHeight);

				{
					size_t curPos = 0;
					auto walker = mDataPtr;
					auto count = getArea() * TDataFormat::size();
					auto walkerOut = newImg.mDataPtr;
					auto countBlock = (count / 4) * 4;

					for (; curPos < countBlock; curPos += 4, walker += 4, walkerOut += 4)
						hr::Color::convertColor(walkerOut, walker, true);

					for (; curPos < count; curPos++, walker++, walkerOut++)
						*walkerOut = hr::Color::convertColor(*walker);
				}

				return newImg;
			}
			// { same type, RGB -> RGBA } special case
			else if constexpr (std::is_same_v<uint8_t, TNewDataType>&& std::is_same_v<TDataFormat, ImageFormatRGB>&& std::is_same_v<TNewDataFormat, ImageFormatRGBA>)
			{
				Image<uint8_t, ImageFormatRGBA> newImg(mWidth, mHeight);

				{
					auto numPixels = getArea();
					auto walker = mDataPtr;
					auto walkerOut = newImg.mDataPtr;

					for (size_t pixel = 0; pixel < numPixels; ++pixel, walker += 3, walkerOut += 4)
					{
						walkerOut[0] = walker[0];
						walkerOut[1] = walker[1];
						walkerOut[2] = walker[2];
						walkerOut[3] = defaultAlphaValue;
					}
				}

				return newImg;
			}
			// { same type, RGBA -> RGB }
			else if constexpr (std::is_same_v<uint8_t, TNewDataType>&& std::is_same_v<TDataFormat, ImageFormatRGBA>&& std::is_same_v<TNewDataFormat, ImageFormatRGB>)
			{
				Image<uint8_t, ImageFormatRGB> newImg(mWidth, mHeight);

				{
					auto numPixels = getArea();
					auto walker = mDataPtr;
					auto walkerOut = newImg.mDataPtr;

					for (size_t pixel = 0; pixel < numPixels; ++pixel, walker += 4, walkerOut += 3)
					{
						walkerOut[0] = walker[0];
						walkerOut[1] = walker[1];
						walkerOut[2] = walker[2];
					}
				}

				return newImg;
			}
			else
			{
				//this takes care of all other cases
				return ImageViewBase<uint8_t, TDataFormat>::template convert<TNewDataType, TNewDataFormat>(defaultColorValue, defaultAlphaValue);
			}
		}

		Image<uint8_t, TDataFormat> resize(size_t width, size_t height, bool assumeSRGB) const
		{
			Image<uint8_t, TDataFormat> newImg(width, height);

			stbir_resize_uint8_generic(mDataPtr, BaseType::width(), BaseType::height(), 0,
				newImg.mDataPtr, width, height, 0,
				TDataFormat::size(), STBIR_ALPHA_CHANNEL_NONE,
				STBIR_FLAG_ALPHA_PREMULTIPLIED, STBIR_EDGE_CLAMP, STBIR_FILTER_DEFAULT, assumeSRGB ? STBIR_COLORSPACE_SRGB : STBIR_COLORSPACE_LINEAR, nullptr);

			return newImg;
		}

	private:
		ImageView() = default;
	};

	template<typename TDataFormat> // float specialization
	class ImageView<float, TDataFormat>
		: public ImageViewBase<float, TDataFormat>
	{
		using BaseType = ImageViewBase<float, TDataFormat>;

		template<typename, typename> friend class Image;
		template<typename, typename> friend class ImageBase;
		template<typename, typename> friend class ImageView;
		template<typename, typename> friend class ImageViewBase;

	public:
		ImageView(float* const data, size_t width, size_t height)
			: BaseType(data, width, height)
		{ }

		ImageView(ImageView&& imgView) = default;
		ImageView& operator=(ImageView&& imgView) = default;

		void getPixel(const size_t x, const size_t y, hr::Color& pixelValue) const
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

		template<typename TNewDataType, typename TNewDataFormat>
		Image<TNewDataType, TNewDataFormat> convert(const float defaultColorValue, const float defaultAlphaValue) const
		{
			static_assert(std::is_base_of<ImageFormat<TNewDataFormat>, TNewDataFormat>::value, "Data format must inherit from type ImageFormat");

			// { float -> uint8_t, same format } special case
			if constexpr (std::is_same_v<uint8_t, TNewDataType>&& std::is_same_v<TDataFormat, TNewDataFormat>)
			{
				Image<uint8_t, TDataFormat> newImg(mWidth, mHeight);

				{
					size_t curPos = 0;
					auto walker = mDataPtr;
					auto count = getArea() * TDataFormat::size();
					auto walkerOut = newImg.mDataPtr;
					auto countBlock = (count / 4) * 4;

					for (; curPos < countBlock; curPos += 4, walker += 4, walkerOut += 4)
						hr::Color::convertColor(walkerOut, walker, true);

					for (; curPos < count; curPos++, walker++, walkerOut++)
						*walkerOut = hr::Color::convertColor(*walker);
				}

				return newImg;
			}
			else
			{
				//this takes care of all other cases
				return ImageViewBase<float, TDataFormat>::template convert<TNewDataType, TNewDataFormat>(defaultColorValue, defaultAlphaValue);
			}
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
		ImageView() = default;
	};

	template<typename TDataType, typename TDataFormat>
	class ImageBase
		: public ImageView<TDataType, TDataFormat>
	{
		template<typename, typename> friend class Image;
		template<typename, typename> friend class ImageBase;
		template<typename, typename> friend class ImageView;
		template<typename, typename> friend class ImageViewBase;

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
			: mDataSource(new TDataType[width * height * TDataFormat::size()])
			, ImageView(nullptr, width, height)
		{
			mDataPtr = mDataSource.get();
		}

		ImageBase(std::unique_ptr<TDataType[]> data, size_t width, size_t height)
			: mDataSource(std::move(data))
			, ImageView(nullptr, width, height)
		{
			mDataPtr = mDataSource.get();
		}

	private:
		std::unique_ptr<TDataType[]> mDataSource;
	};

	template<typename TDataType, typename TDataFormat> // generic Image (any type supported)
	class Image
		: public ImageBase<TDataType, TDataFormat>
	{
		using BaseType = ImageBase<TDataType, TDataFormat>;

		template<typename, typename> friend class Image;
		template<typename, typename> friend class ImageBase;
		template<typename, typename> friend class ImageView;
		template<typename, typename> friend class ImageViewBase;

	public:
		Image() = default;

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

	template<typename TDataFormat> // uint8_t specialization
	class Image<uint8_t, TDataFormat>
		: public ImageBase<uint8_t, TDataFormat>
	{
		using BaseType = ImageBase<uint8_t, TDataFormat>;

		template<typename, typename> friend class Image;
		template<typename, typename> friend class ImageBase;
		template<typename, typename> friend class ImageView;
		template<typename, typename> friend class ImageViewBase;

	public:
		Image() = default;

		Image(size_t width, size_t height)
			: BaseType(width, height)
		{ }

		Image(std::unique_ptr<uint8_t[]> data, size_t width, size_t height)
			: BaseType(std::move(data), width, height)
		{ }

		Image(const Image&) = delete;
		Image& operator=(const Image&) = delete;
		Image(Image&& img) = default;
		Image& operator=(Image&& img) = default;

		void clear(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a)
		{
			uint8_t tmpPixel[] = { r, g, b, a };

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

			hr::Color pixelValue;
			uint8_t tmpPixel[4];

			for (size_t curPos = 0; curPos < imgArea; curPos++)
			{
				TDataFormat::readRGBA<uint8_t>(walkerPtr, tmpPixel, 0, 255);
				if (tmpPixel[3] != 255)
				{
					pixelValue.Set(tmpPixel);
					pixelValue.WeightRGB(pixelValue.a);
					pixelValue.Write(tmpPixel);

					TDataFormat::writeRGB<uint8_t>(walkerPtr, tmpPixel);
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

				uint8_t tmpPixel[4];
				for (size_t curPos = 0; curPos < imgArea; curPos++)
				{
					TDataFormat::readRGB<uint8_t>(walkerPtr, tmpPixel, 0);
					tmpPixel[0] = Color::gammaCorrect(tmpPixel[0]);
					tmpPixel[1] = Color::gammaCorrect(tmpPixel[1]);
					tmpPixel[2] = Color::gammaCorrect(tmpPixel[2]);
					TDataFormat::writeRGB<uint8_t>(walkerPtr, tmpPixel);

					walkerPtr += TDataFormat::size();
				}
			}
		}

		void setPixel(const size_t x, const size_t y, hr::Color& pixelValue)
		{
			uint8_t tmpPixel[4];
			hr::Color::convertColor(hr::Color(r, g, b, a), tmpPixel, true);

			TDataFormat::writeRGBA(mDataPtr + getPos(x, y), tmpPixel);
		}

		void setPixel(const size_t x, const size_t y, const uint8_t* const pixelValue)
		{
			TDataFormat::writeRGBA(mDataPtr + getPos(x, y), pixelValue);
		}

		void renormalizeNormals(bool expandPixels)
		{
			if (empty() || TDataFormat::size() < 3)
				return;

			auto walkerPtr = mDataPtr;
			auto imgArea = getArea();

			if (expandPixels)
			{
				for (size_t curPos = 0; curPos < imgArea; curPos++, walkerPtr += TDataFormat::size())
				{
					Vector3f vec;

					hr::Color::convertColor(vec.data(), walkerPtr, false);
					vec.mad(2.0f, -1.0f).normalize().mad(0.5f, 0.5f);
					hr::Color::convertColor(walkerPtr, vec.data(), false);
				}
			}
			else
			{
				for (size_t curPos = 0; curPos < imgArea; curPos++, walkerPtr += TDataFormat::size())
				{
					Vector3f vec;

					hr::Color::convertColor(vec.data(), walkerPtr, false);
					vec.normalize();
					hr::Color::convertColor(walkerPtr, vec.data(), false);
				}
			}
		}

		void transform(std::function<bool(hr::Color&)> cb)
		{
			if (empty() || !cb)
				return;

			auto walkerPtr = mDataPtr;
			auto imgArea = getArea();

			uint8_t tmpPixel[4];
			hr::Color pixelValue;

			for (size_t curPos = 0; curPos < imgArea; curPos++)
			{
				TDataFormat::readRGBA<uint8_t>(walkerPtr, tmpPixel, 0, 255);

				pixelValue.Set(tmpPixel);
				if (cb(pixelValue))
				{
					pixelValue.Write(tmpPixel);
					TDataFormat::writeRGBA<uint8_t>(walkerPtr, tmpPixel);
				}

				walkerPtr += TDataFormat::size();
			}
		}
	};

	template<typename TDataFormat> // float specialization
	class Image<float, TDataFormat>
		: public ImageBase<float, TDataFormat>
	{
		using BaseType = ImageBase<float, TDataFormat>;

		template<typename, typename> friend class Image;
		template<typename, typename> friend class ImageBase;
		template<typename, typename> friend class ImageView;
		template<typename, typename> friend class ImageViewBase;

	public:
		Image() = default;

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

		void setPixel(const size_t x, const size_t y, hr::Color& pixelValue)
		{
			TDataFormat::writeRGBA(mDataPtr + getPos(x, y), pixelValue);
		}

		void setPixel(const size_t x, const size_t y, const float* const pixelValue)
		{
			TDataFormat::writeRGBA(mDataPtr + getPos(x, y), pixelValue);
		}

		void transform(const std::function<bool(hr::Color&)>& cb)
		{
			if (empty() || !cb)
				return;

			auto walkerPtr = mDataPtr;
			auto imgArea = getArea();

			float tmpPixel[4];
			hr::Color pixelValue;

			for (size_t curPos = 0; curPos < imgArea; curPos++, walkerPtr += TDataFormat::size())
			{
				TDataFormat::readRGBA<float>(walkerPtr, tmpPixel, 0.0f, 1.0f);

				pixelValue.set(tmpPixel);
				if (cb(pixelValue))
				{
					pixelValue.write(tmpPixel);
					TDataFormat::writeRGBA<float>(walkerPtr, tmpPixel);
				}
			}
		}

		void renormalizeNormals(bool expandPixels)
		{
			if (empty() || TDataFormat::size() < 3)
				return;

			auto walkerPtr = mDataPtr;
			auto imgArea = getArea();

			if (expandPixels)
			{
				for (size_t curPos = 0; curPos < imgArea; curPos++, walkerPtr += TDataFormat::size())
				{
					Vector3f vec(walkerPtr);
					vec.mad(2.0f, -1.0f).normalize().mad(0.5f, 0.5f).write(walkerPtr);
				}
			}
			else
			{
				for (size_t curPos = 0; curPos < imgArea; curPos++, walkerPtr += TDataFormat::size())
				{
					Vector3f vec(walkerPtr);
					vec.normalize().write(walkerPtr);
				}
			}
		}
	};
}
