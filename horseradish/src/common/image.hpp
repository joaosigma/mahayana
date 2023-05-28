#pragma once

#include "color.hpp"
#include "imageFormats.hpp"
#include "libs/stb/stb_image_resize.h"

#include <cassert>
#include <concepts>
#include <type_traits>

namespace hr::imaging
{
	template<typename TDataType, typename TDataFormat> class Image;
	template<typename TDataType, typename TDataFormat> class ImageView;
	template<typename TDataType, typename TDataFormat> class ImageBase;

	template<typename TDataType, typename TDataFormat>
	class ImageViewBase
	{
		static_assert(std::is_arithmetic_v<TDataType>, "Data type must be arithmetic (e.g.: float, uint8_t, etc.)");
		static_assert(std::is_base_of_v<ImageFormat<TDataFormat>, TDataFormat>, "Data format must inherit from type ImageFormat");

		friend class ImageView<TDataType, TDataFormat>;
		friend class ImageBase<TDataType, TDataFormat>;

	public:
		constexpr size_t pixelSize() const
		{
			return (TDataFormat::size() * sizeof(TDataType));
		}

	public:
		ImageViewBase(const ImageViewBase&) = delete;
		ImageViewBase& operator=(const ImageViewBase&) = delete;

		ImageViewBase(ImageViewBase&& imgView) noexcept
		{
			*this = std::move(imgView);
		}

		ImageViewBase& operator=(ImageViewBase&& imgView) noexcept
		{
			if (this != &imgView)
			{
				std::swap(mDataPtr, imgView.mDataPtr);
				std::swap(mWidth, imgView.mWidth);
				std::swap(mHeight, imgView.mHeight);
			}

			return *this;
		}

		constexpr explicit operator bool() const noexcept
		{
			return mDataPtr;
		}

		std::span<const TDataType> asSpan() const noexcept
		{
			return {mDataPtr, size()};
		}

		const TDataType* data() const noexcept
		{
			return mDataPtr;
		}

		size_t width() const noexcept
		{
			return mWidth;
		}

		size_t height() const noexcept
		{
			return mHeight;
		}

		bool empty() const noexcept
		{
			return (area() <= 0);
		}

		size_t area() const noexcept
		{
			return mWidth * mHeight;
		}

		size_t size() const noexcept
		{
			return (mWidth * mHeight * TDataFormat::size() * sizeof(TDataType));
		}

		size_t rowSize() const noexcept
		{
			return (mWidth * TDataFormat::size() * sizeof(TDataType));
		}

		size_t pixelOffset(const size_t x, const size_t y) const noexcept
		{
			return ((y * mWidth * TDataFormat::size()) + (x * TDataFormat::size()));
		}

		void getPixel(const size_t x, const size_t y, TDataType* const pixelValue, const TDataType defaultColorValue, const TDataType defaultAlphaValue) const noexcept
		{
			TDataFormat::template readRGBA<TDataType>(mDataPtr + pixelOffset(x, y), pixelValue, defaultColorValue, defaultAlphaValue);
		}

		Image<TDataType, TDataFormat> crop(size_t cropX, size_t cropY, size_t cropWidth, size_t cropHeight) const;
		Image<TDataType, TDataFormat> clone() const;
		Image<TDataType, TDataFormat> resize(size_t width, size_t height, bool assumeSRGB) const;

		template<typename TNewDataType, typename TNewDataFormat>
		Image<TNewDataType, TNewDataFormat> convert(const TDataType defaultColorValue, const TDataType defaultAlphaValue) const;

		void iterateMipmaps(bool assumeSRGB, std::invocable<size_t, const Image<TDataType, TDataFormat>&> auto&& func) const;

	protected:
		ImageViewBase(TDataType* const data, size_t width, size_t height) noexcept
			: mDataPtr{ data }, mWidth{ width }, mHeight{ height }
		{ }

	private:
		TDataType* mDataPtr{ nullptr };
		size_t mWidth{ 0 }, mHeight{ 0 };
	};

	template<typename TDataType, typename TDataFormat> // generic ImageView (any type supported)
	class ImageView
		: public ImageViewBase<TDataType, TDataFormat>
	{
		using BaseType = ImageViewBase<TDataType, TDataFormat>;

		friend class ImageView<TDataType, TDataFormat>;
		friend class ImageBase<TDataType, TDataFormat>;

	public:
		ImageView() noexcept
			: BaseType(nullptr, 0, 0)
		{ }

		ImageView(TDataType* const data, size_t width, size_t height) noexcept
			: BaseType(data, width, height)
		{ }

		ImageView(ImageView&& imgView) = default;
		ImageView& operator=(ImageView&& imgView) = default;		
	};

	template<typename TDataFormat> // uint8_t specialization
	class ImageView<uint8_t, TDataFormat>
		: public ImageViewBase<uint8_t, TDataFormat>
	{
		using BaseType = ImageViewBase<uint8_t, TDataFormat>;

		friend class ImageView<uint8_t, TDataFormat>;
		friend class ImageBase<uint8_t, TDataFormat>;

	public:
		ImageView() noexcept
			: BaseType(nullptr, 0, 0)
		{ }

		ImageView(uint8_t* const data, size_t width, size_t height) noexcept
			: BaseType(data, width, height)
		{ }

		ImageView(ImageView&& imgView) = default;
		ImageView& operator=(ImageView&& imgView) = default;

		void getPixel(const size_t x, const size_t y, hr::Colorf& pixelValue) const noexcept
		{
			BaseType::getPixel(x, y, pixelValue.data(), 0, 255);
		}

		void getPixel(const size_t x, const size_t y, uint8_t* const pixelValue) const noexcept
		{
			BaseType::getPixel(x, y, pixelValue, 0, 255);
		}
	};

	template<typename TDataFormat> // float specialization
	class ImageView<float, TDataFormat>
		: public ImageViewBase<float, TDataFormat>
	{
		using BaseType = ImageViewBase<float, TDataFormat>;

		friend class ImageView<float, TDataFormat>;
		friend class ImageBase<float, TDataFormat>;

	public:
		ImageView() noexcept
			: BaseType(nullptr, 0, 0)
		{ }

		ImageView(float* const data, size_t width, size_t height) noexcept
			: BaseType(data, width, height)
		{ }

		ImageView(ImageView&& imgView) = default;
		ImageView& operator=(ImageView&& imgView) = default;

		void getPixel(const size_t x, const size_t y, hr::Colorf& pixelValue) const noexcept
		{
			BaseType::getPixel(x, y, pixelValue.data(), 0.0f, 1.0f);
		}

		void getPixel(const size_t x, const size_t y, float* const pixelValue) const noexcept
		{
			BaseType::getPixel(x, y, pixelValue, 0.0f, 1.0f);
		}
	};

	template<typename TDataType, typename TDataFormat>
	class ImageBase
		: public ImageView<TDataType, TDataFormat>
	{
		using BaseType = ImageView<TDataType, TDataFormat>;

	public:
		ImageBase(const ImageBase&) = delete;
		ImageBase& operator=(const ImageBase&) = delete;
		ImageBase(ImageBase&& img) = default;
		ImageBase& operator=(ImageBase&& img) = default;

		std::span<TDataType> asSpan() noexcept
		{
			assert(!mDataSource || mDataSource.get() == BaseType::data());
			return {mDataSource.get(), BaseType::size()};
		}

		std::span<const TDataType> asSpan() const noexcept
		{
			assert(!mDataSource || mDataSource.get() == BaseType::data());
			return {mDataSource.get(), BaseType::size()};
		}

		TDataType* data() noexcept
		{
			assert(!mDataSource || mDataSource.get() == BaseType::data());
			return mDataSource.get();
		}

		void flip()
		{
			std::unique_ptr<TDataType[]> tempRow(new TDataType[BaseType::width() * TDataFormat::size()]);

			auto rowSize = BaseType::width() * TDataFormat::size();
			auto numRows = BaseType::height() / 2;

			auto bottomPtr = mDataSource.get();
			auto topPtr = bottomPtr + ((BaseType::height() - 1) * rowSize);

			for (size_t curY = 0; curY < numRows; curY++)
			{
				std::memcpy(tempRow.get(), topPtr, rowSize * sizeof(TDataType));
				std::memcpy(topPtr, bottomPtr, rowSize * sizeof(TDataType));
				std::memcpy(bottomPtr, tempRow.get(), rowSize * sizeof(TDataType));

				bottomPtr += rowSize;
				topPtr -= rowSize;
			}
		}

		bool setPixelRegion(const ImageView<TDataType, TDataFormat>& imgView, const size_t offsetX, const size_t offsetY, const bool flipSource = false)
		{
			if (((offsetX + imgView.width()) > BaseType::width()) || ((offsetY + imgView.height()) > BaseType::height()))
				return false;

			auto destRowSize = BaseType::width() * TDataFormat::size();
			auto srcRowSize = imgView.width() * TDataFormat::size();

			if (!flipSource)
			{
				for (size_t curY = 0; curY < imgView.height(); curY++)
				{
					auto destY = ((offsetY + curY) * destRowSize) + (offsetX * TDataFormat::size());
					auto sourceY = (curY * srcRowSize);
					std::memcpy(mDataSource.get() + destY, imgView.data() + sourceY, srcRowSize * sizeof(TDataType));
				}
			}
			else
			{
				for (size_t curY = 0; curY < imgView.height(); curY++)
				{
					auto destY = ((offsetY + curY) * destRowSize) + (offsetX * TDataFormat::size());
					auto sourceY = (imgView.height() - curY - 1) * srcRowSize;
					std::memcpy(mDataSource.get() + destY, imgView.data() + sourceY, srcRowSize * sizeof(TDataType));
				}
			}

			return true;
		}

	protected:
		ImageBase() noexcept
			: BaseType(nullptr, 0, 0)
		{ }

		ImageBase(size_t width, size_t height)
			: BaseType(new TDataType[width * height * TDataFormat::size()], width, height)
			, mDataSource(const_cast<TDataType*>(BaseType::data()))
		{
			assert(mDataSource.get() == BaseType::data());
		}

		ImageBase(std::unique_ptr<TDataType[]> data, size_t width, size_t height) noexcept
			: BaseType(data.get(), width, height)
			, mDataSource{ std::move(data) }
		{ 
			assert(mDataSource.get() == BaseType::data());
		}

	private:
		std::unique_ptr<TDataType[]> mDataSource;
	};

	template<typename TDataType, typename TDataFormat> // generic Image (any type supported)
	class Image
		: public ImageBase<TDataType, TDataFormat>
	{
		using BaseType = ImageBase<TDataType, TDataFormat>;

	public:
		Image() = default;

		explicit Image(size_t width, size_t height)
			: BaseType(width, height)
		{ }

		explicit Image(std::unique_ptr<TDataType[]> data, size_t width, size_t height)
			: BaseType(std::move(data), width, height)
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
			auto walkerPtr = BaseType::data();

			auto imgArea = BaseType::area();
			auto imgAreaBlock = (imgArea / 4) * 4;

			for (; curPos < imgAreaBlock; curPos += 4)
			{
				TDataFormat::template writeRGBA(walkerPtr + (0 * TDataFormat::size()), tmpPixel);
				TDataFormat::template writeRGBA(walkerPtr + (1 * TDataFormat::size()), tmpPixel);
				TDataFormat::template writeRGBA(walkerPtr + (2 * TDataFormat::size()), tmpPixel);
				TDataFormat::template writeRGBA(walkerPtr + (3 * TDataFormat::size()), tmpPixel);
				walkerPtr += (TDataFormat::size() * 4);
			}

			for (; curPos < imgArea; curPos++)
			{
				TDataFormat::template writeRGBA(walkerPtr, tmpPixel);
				walkerPtr += TDataFormat::size();
			}
		}

		void multiplyAlpha()
		{
			if (!TDataFormat::hasAlpha())
				return;

			auto walkerPtr = BaseType::data();
			auto imgArea = BaseType::area();

			hr::Colorf pixelValue;
			uint8_t tmpPixel[4];

			for (size_t curPos = 0; curPos < imgArea; curPos++)
			{
				TDataFormat::template readRGBA<uint8_t>(walkerPtr, tmpPixel, 0, 255);
				if (tmpPixel[3] != 255)
				{
					pixelValue.set(tmpPixel);
					pixelValue.weightRGB(pixelValue[3]);
					pixelValue.write(tmpPixel);

					TDataFormat::template writeRGB<uint8_t>(walkerPtr, tmpPixel);
				}

				walkerPtr += TDataFormat::size();
			}
		}

		void removeGamma()
		{
			if (!TDataFormat::hasAlpha())
			{
				auto walkerPtr = BaseType::data();
				auto walkerEnd = walkerPtr + (BaseType::area() * TDataFormat::size());

				for (; walkerPtr < walkerEnd; walkerPtr++)
					*walkerPtr = hr::Colorf::gammaCorrect(*walkerPtr);
			}
			else
			{
				auto walkerPtr = BaseType::data();
				auto imgArea = BaseType::area();

				uint8_t tmpPixel[4];
				for (size_t curPos = 0; curPos < imgArea; curPos++)
				{
					TDataFormat::template readRGB<uint8_t>(walkerPtr, tmpPixel, 0);
					tmpPixel[0] = hr::Colorf::gammaCorrect(tmpPixel[0]);
					tmpPixel[1] = hr::Colorf::gammaCorrect(tmpPixel[1]);
					tmpPixel[2] = hr::Colorf::gammaCorrect(tmpPixel[2]);
					TDataFormat::template writeRGB<uint8_t>(walkerPtr, tmpPixel);

					walkerPtr += TDataFormat::size();
				}
			}
		}

		void setPixel(const size_t x, const size_t y, const hr::Colorf& pixelValue)
		{
			uint8_t tmpPixel[4];
			hr::Colorf::convertColor(tmpPixel, pixelValue.data(), true);

			TDataFormat::template writeRGBA(BaseType::data() + BaseType::pixelOffset(x, y), tmpPixel);
		}

		void setPixel(const size_t x, const size_t y, const uint8_t* const pixelValue)
		{
			TDataFormat::template writeRGBA(BaseType::data() + BaseType::pixelOffset(x, y), pixelValue);
		}

		void renormalizeNormals(bool expandPixels)
		{
			if (BaseType::empty() || TDataFormat::size() < 3)
				return;

			auto walkerPtr = BaseType::data();
			auto imgArea = BaseType::area();

			if (expandPixels)
			{
				for (size_t curPos = 0; curPos < imgArea; curPos++, walkerPtr += TDataFormat::size())
				{
					Vector3f vec;

					hr::Colorf::convertColor(vec.data(), walkerPtr, false);
					vec.mad(2.0f, -1.0f).normalize().mad(0.5f, 0.5f);
					hr::Colorf::convertColor(walkerPtr, vec.data(), false);
				}
			}
			else
			{
				for (size_t curPos = 0; curPos < imgArea; curPos++, walkerPtr += TDataFormat::size())
				{
					Vector3f vec;

					hr::Colorf::convertColor(vec.data(), walkerPtr, false);
					vec.normalize();
					hr::Colorf::convertColor(walkerPtr, vec.data(), false);
				}
			}
		}

		void transform(std::function<bool(hr::Colorf&)> cb)
		{
			if (BaseType::empty() || !cb)
				return;

			auto walkerPtr = BaseType::data();
			auto imgArea = BaseType::area();

			uint8_t tmpPixel[4];
			hr::Colorf pixelValue;

			for (size_t curPos = 0; curPos < imgArea; curPos++)
			{
				TDataFormat::template readRGBA<uint8_t>(walkerPtr, tmpPixel, 0, 255);

				pixelValue.set(tmpPixel);
				if (cb(pixelValue))
				{
					pixelValue.write(tmpPixel);
					TDataFormat::template writeRGBA<uint8_t>(walkerPtr, tmpPixel);
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
			auto walkerPtr = BaseType::data();

			auto imgArea = BaseType::area();
			auto imgAreaBlock = (imgArea / 4) * 4;

			for (; curPos < imgAreaBlock; curPos += 4)
			{
				TDataFormat::template writeRGBA(walkerPtr + (0 * TDataFormat::size()), tmpPixel);
				TDataFormat::template writeRGBA(walkerPtr + (1 * TDataFormat::size()), tmpPixel);
				TDataFormat::template writeRGBA(walkerPtr + (2 * TDataFormat::size()), tmpPixel);
				TDataFormat::template writeRGBA(walkerPtr + (3 * TDataFormat::size()), tmpPixel);
				walkerPtr += (TDataFormat::size() * 4);
			}

			for (; curPos < imgArea; curPos++)
			{
				TDataFormat::template writeRGBA(walkerPtr, tmpPixel);
				walkerPtr += TDataFormat::size();
			}
		}

		void setPixel(const size_t x, const size_t y, const hr::Colorf& pixelValue)
		{
			TDataFormat::template writeRGBA(BaseType::data() + BaseType::pixelOffset(x, y), pixelValue.data());
		}

		void setPixel(const size_t x, const size_t y, const float* const pixelValue)
		{
			TDataFormat::template writeRGBA(BaseType::data() + BaseType::pixelOffset(x, y), pixelValue);
		}

		void transform(const std::function<bool(hr::Colorf&)>& cb)
		{
			if (BaseType::empty() || !cb)
				return;

			auto walkerPtr = BaseType::data();
			auto imgArea = BaseType::area();

			float tmpPixel[4];
			hr::Colorf pixelValue;

			for (size_t curPos = 0; curPos < imgArea; curPos++, walkerPtr += TDataFormat::size())
			{
				TDataFormat::template readRGBA<float>(walkerPtr, tmpPixel, 0.0f, 1.0f);

				pixelValue.set(tmpPixel);
				if (cb(pixelValue))
				{
					pixelValue.write(tmpPixel);
					TDataFormat::template writeRGBA<float>(walkerPtr, tmpPixel);
				}
			}
		}

		void renormalizeNormals(bool expandPixels)
		{
			if (BaseType::empty() || TDataFormat::size() < 3)
				return;

			auto walkerPtr = BaseType::data();
			auto imgArea = BaseType::area();

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

	template<typename TDataType, typename TDataFormat>
	Image<TDataType, TDataFormat> ImageViewBase<TDataType, TDataFormat>::crop(size_t cropX, size_t cropY, size_t cropWidth, size_t cropHeight) const
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

			std::memcpy(newImg.mDataPtr + destPos, mDataPtr + sourcePos, destRowSize * sizeof(TDataType));
		}

		return newImg;
	}

	template<typename TDataType, typename TDataFormat>
	Image<TDataType, TDataFormat> ImageViewBase<TDataType, TDataFormat>::clone() const
	{
		Image<TDataType, TDataFormat> newImg(mWidth, mHeight);

		std::memcpy(newImg.data(), mDataPtr, size());
		return newImg;
	}

	template<typename TDataType, typename TDataFormat>
	Image<TDataType, TDataFormat> ImageViewBase<TDataType, TDataFormat>::resize(size_t width, size_t height, bool assumeSRGB) const
	{
		if constexpr (std::is_same_v<uint8_t, TDataType>)
		{
			Image<uint8_t, TDataFormat> newImg(width, height);

			stbir_resize_uint8_generic(mDataPtr, mWidth, mHeight, 0,
				newImg.data(), width, height, 0,
				TDataFormat::size(), STBIR_ALPHA_CHANNEL_NONE,
				STBIR_FLAG_ALPHA_PREMULTIPLIED, STBIR_EDGE_CLAMP, STBIR_FILTER_DEFAULT, assumeSRGB ? STBIR_COLORSPACE_SRGB : STBIR_COLORSPACE_LINEAR, nullptr);

			return newImg;
		}
		else if constexpr (std::is_same_v<float, TDataType>)
		{
			assert(!assumeSRGB); //float is always linear

			Image<float, TDataFormat> newImg(width, height);

			stbir_resize_float_generic(mDataPtr, mWidth, mHeight, 0,
				newImg.data(), width, height, 0,
				TDataFormat::size(), STBIR_ALPHA_CHANNEL_NONE,
				STBIR_FLAG_ALPHA_PREMULTIPLIED, STBIR_EDGE_CLAMP, STBIR_FILTER_DEFAULT, STBIR_COLORSPACE_LINEAR, nullptr);

			return newImg;
		}

		static_assert(std::is_same_v<uint8_t, TDataType> || std::is_same_v<float, TDataType>, "Unsupported data type for resize");
	}

	template<typename TDataType, typename TDataFormat>
	template<typename TNewDataType, typename TNewDataFormat>
	Image<TNewDataType, TNewDataFormat> ImageViewBase<TDataType, TDataFormat>::convert(const TDataType defaultColorValue, const TDataType defaultAlphaValue) const
	{
		static_assert(std::is_arithmetic_v<TDataType>, "Data type must be arithmetic (e.g.: float, uint8_t, etc.)");
		static_assert(std::is_base_of_v<ImageFormat<TDataFormat>, TDataFormat>, "Data format must inherit from type ImageFormat");

		//special clone case
		if constexpr (std::is_same_v<TDataType, TNewDataType> && std::is_same_v<TDataFormat, TNewDataFormat>)
		{
			return clone();
		}
		//the data type is the same (format changes)
		else if constexpr (std::is_same_v<TDataType, TNewDataType>)
		{
			Image<TNewDataType, TNewDataFormat> newImg(mWidth, mHeight);

			//RGB -> RGBA
			if constexpr (std::is_same_v<TDataFormat, ImageFormatRGB> && std::is_same_v<TNewDataFormat, ImageFormatRGBA>)
			{
				auto numPixels = area();
				auto walker = mDataPtr;
				auto walkerOut = newImg.data();

				for (size_t pixel = 0; pixel < numPixels; ++pixel, walker += 3, walkerOut += 4)
				{
					walkerOut[0] = walker[0];
					walkerOut[1] = walker[1];
					walkerOut[2] = walker[2];
					walkerOut[3] = defaultAlphaValue;
				}
			}
			//RGBA->RGB
			else if constexpr (std::is_same_v<TDataFormat, ImageFormatRGBA> && std::is_same_v<TNewDataFormat, ImageFormatRGB>)
			{
				auto numPixels = area();
				auto walker = mDataPtr;
				auto walkerOut = newImg.data();

				for (size_t pixel = 0; pixel < numPixels; ++pixel, walker += 4, walkerOut += 3)
				{
					walkerOut[0] = walker[0];
					walkerOut[1] = walker[1];
					walkerOut[2] = walker[2];
				}
			}
			else
			{
				TDataType pixel[4];
				for (size_t y = 0; y < mHeight; ++y)
				{
					for (size_t x = 0; x < mWidth; ++x)
					{
						TDataFormat::template readRGBA<TDataType>(mDataPtr + pixelOffset(x, y), pixel, defaultColorValue, defaultAlphaValue);
						TNewDataFormat::template writeRGBA<TNewDataType>(newImg.data() + newImg.pixelOffset(x, y), pixel);
					}
				}
			}

			return newImg;
		}
		//the data format is the same (type changes)
		else if constexpr (std::is_same_v<TDataFormat, TNewDataFormat>)
		{
			Image<TNewDataType, TNewDataFormat> newImg(mWidth, mHeight);

			//uint8_t <-> float
			if constexpr ((std::is_same_v<uint8_t, TDataType> || std::is_same_v<float, TDataType>) && (std::is_same_v<uint8_t, TNewDataType> || std::is_same_v<float, TNewDataType>))
			{
				size_t curPos = 0;
				auto walker = mDataPtr;
				auto count = area() * TDataFormat::size();
				auto walkerOut = newImg.data();
				auto countBlock = (count / 4) * 4;

				for (; curPos < countBlock; curPos += 4, walker += 4, walkerOut += 4)
					hr::Colorf::convertColor(walkerOut, walker, true);

				for (; curPos < count; curPos++, walker++, walkerOut++)
					*walkerOut = hr::Colorf::convertColor(*walker);
			}

			static_assert((std::is_same_v<uint8_t, TDataType> || std::is_same_v<float, TDataType>) && (std::is_same_v<uint8_t, TNewDataType> || std::is_same_v<float, TNewDataType>), "Can only convert between uint8_t and float");

			return newImg;
		}

		return {};
	}

	template<typename TDataType, typename TDataFormat>
	void ImageViewBase<TDataType, TDataFormat>::iterateMipmaps(bool assumeSRGB, std::invocable<size_t, const Image<TDataType, TDataFormat>&> auto&& func) const
	{
		if (area() <= 1)
			return;

		auto imageScaled = clone();

		size_t mipLevel{1};
		while (imageScaled.area() > 1)
		{
			imageScaled = imageScaled.resize(std::max<size_t>(1, imageScaled.width() >> 1), std::max<size_t>(1, imageScaled.height() >> 1), assumeSRGB);
			func(mipLevel++, imageScaled);
		}
	}
}
