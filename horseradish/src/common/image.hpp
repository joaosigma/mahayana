#pragma once

#include "color.hpp"
#include "imageFormats.hpp"
#include "vector.hpp"

#include <stb_image_resize2.h>

#include <array>
#include <cassert>
#include <concepts>
#include <cstdint>
#include <cstring>
#include <span>
#include <type_traits>
#include <utility>

namespace hr::imaging
{
    template<typename TDataType, typename TDataFormat>
        requires concepts::ImageFormat<TDataFormat, TDataType>
    class Image;
    template<typename TDataType, typename TDataFormat>
        requires concepts::ImageFormat<TDataFormat, TDataType>
    class ImageView;
    template<typename TDataType, typename TDataFormat>
        requires concepts::ImageFormat<TDataFormat, TDataType>
    class ImageBase;

    template<typename TDataType, typename TDataFormat>
        requires concepts::ImageFormat<TDataFormat, TDataType>
    class ImageViewBase
    {
        static_assert(std::is_arithmetic_v<TDataType>, "Data type must be arithmetic (e.g.: float, uint8_t, etc.)");

        friend class ImageView<TDataType, TDataFormat>;
        friend class ImageBase<TDataType, TDataFormat>;

    public:
        constexpr static size_t pixelSize()
        {
            return (TDataFormat::size * sizeof(TDataType));
        }

    private:
        const TDataType* mDataPtr{nullptr};
        size_t mWidth{0}, mHeight{0};

    protected:
        ImageViewBase(const TDataType* data, size_t width, size_t height) noexcept
          : mDataPtr{data}, mWidth{width}, mHeight{height}
        {}

    public:
        ImageViewBase() noexcept = delete;
        ~ImageViewBase() noexcept = default;

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

        std::span<const TDataType> data() const noexcept
        {
            return {mDataPtr, size()};
        }

        std::span<const TDataType> dataPixel(const size_t x, const size_t y) const noexcept
        {
            return {mDataPtr + pixelOffset(x, y), TDataFormat::size};
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
            return (mWidth * mHeight * TDataFormat::size * sizeof(TDataType));
        }

        size_t rowSize() const noexcept
        {
            return (mWidth * TDataFormat::size * sizeof(TDataType));
        }

        size_t pixelOffset(const size_t x, const size_t y) const noexcept
        {
            return ((y * mWidth * TDataFormat::size) + (x * TDataFormat::size));
        }

        void getPixel(const size_t x, const size_t y, std::span<TDataType, 4> pixelValue, const TDataType defaultColorValue, const TDataType defaultAlphaValue) const noexcept
        {
            TDataFormat::template readRGBA<TDataType>(dataPixel(x, y), pixelValue, defaultColorValue, defaultAlphaValue);
        }

        Image<TDataType, TDataFormat> crop(size_t cropX, size_t cropY, size_t cropWidth, size_t cropHeight) const;
        Image<TDataType, TDataFormat> clone() const;
        Image<TDataType, TDataFormat> resize(size_t width, size_t height, bool assumeSRGB) const;

        template<typename TNewDataType, concepts::ImageFormat<TNewDataType> TNewDataFormat>
        Image<TNewDataType, TNewDataFormat> convert(TDataType defaultColorValue, TDataType defaultAlphaValue) const;

        void iterateMipmaps(bool assumeSRGB, std::invocable<size_t, const Image<TDataType, TDataFormat>&> auto&& func) const;
    };

    template<typename TDataType, typename TDataFormat> // generic ImageView (any type supported)
        requires concepts::ImageFormat<TDataFormat, TDataType>
    class ImageView: public ImageViewBase<TDataType, TDataFormat>
    {
        using BaseType = ImageViewBase<TDataType, TDataFormat>;

        friend class ImageView<TDataType, TDataFormat>;
        friend class ImageBase<TDataType, TDataFormat>;

    public:
        ImageView() noexcept
          : BaseType(nullptr, 0, 0)
        {}

        ImageView(const TDataType* data, size_t width, size_t height) noexcept
          : BaseType(data, width, height)
        {}

        ~ImageView() noexcept = default;

        ImageView(const ImageView&) = delete;
        ImageView& operator=(const ImageView&) = delete;

        ImageView(ImageView&& imgView) = default;
        ImageView& operator=(ImageView&& imgView) = default;
    };

    template<typename TDataFormat> // uint8_t specialization
    class ImageView<uint8_t, TDataFormat>: public ImageViewBase<uint8_t, TDataFormat>
    {
        using BaseType = ImageViewBase<uint8_t, TDataFormat>;

        friend class ImageView<uint8_t, TDataFormat>;
        friend class ImageBase<uint8_t, TDataFormat>;

    public:
        ImageView() noexcept
          : BaseType(nullptr, 0, 0)
        {}

        ImageView(const uint8_t* data, size_t width, size_t height) noexcept
          : BaseType(data, width, height)
        {}

        ~ImageView() noexcept = default;

        ImageView(const ImageView&) = delete;
        ImageView& operator=(const ImageView&) = delete;

        ImageView(ImageView&& imgView) = default;
        ImageView& operator=(ImageView&& imgView) = default;

        void getPixel(const size_t x, const size_t y, hr::Colorf& pixelValue) const noexcept
        {
            std::array<uint8_t, 4> pixel;
            BaseType::getPixel(x, y, pixel, 0, 255);

            pixelValue.set(pixel);
        }

        void getPixel(const size_t x, const size_t y, std::span<uint8_t, 4> pixelValue) const noexcept
        {
            BaseType::getPixel(x, y, pixelValue, 0, 255);
        }
    };

    template<typename TDataFormat> // float specialization
    class ImageView<float, TDataFormat>: public ImageViewBase<float, TDataFormat>
    {
        using BaseType = ImageViewBase<float, TDataFormat>;

        friend class ImageView<float, TDataFormat>;
        friend class ImageBase<float, TDataFormat>;

    public:
        ImageView() noexcept
          : BaseType(nullptr, 0, 0)
        {}

        ImageView(const float* data, size_t width, size_t height) noexcept
          : BaseType(data, width, height)
        {}

        ~ImageView() noexcept = default;

        ImageView(const ImageView&) = delete;
        ImageView& operator=(const ImageView&) = delete;

        ImageView(ImageView&& imgView) = default;
        ImageView& operator=(ImageView&& imgView) = default;

        void getPixel(const size_t x, const size_t y, hr::Colorf& pixelValue) const noexcept
        {
            BaseType::getPixel(x, y, pixelValue.data(), 0.0f, 1.0f);
        }

        void getPixel(const size_t x, const size_t y, std::span<float, 4> pixelValue) const noexcept
        {
            BaseType::getPixel(x, y, pixelValue, 0.0f, 1.0f);
        }
    };

    template<typename TDataType, typename TDataFormat>
        requires concepts::ImageFormat<TDataFormat, TDataType>
    class ImageBase: public ImageView<TDataType, TDataFormat>
    {
        using BaseType = ImageView<TDataType, TDataFormat>;

    public:
        ~ImageBase() noexcept = default;

        ImageBase(const ImageBase&) = delete;
        ImageBase& operator=(const ImageBase&) = delete;
        ImageBase(ImageBase&& img) = default;
        ImageBase& operator=(ImageBase&& img) = default;

        template<typename Self>
        constexpr auto data(this Self&& self) noexcept
        {
            assert(!self.mDataSource || self.mDataSource.get() == static_cast<BaseType&>(self).data().data());
            return std::span{self.mDataSource.get(), static_cast<BaseType&>(self).size()};
        }

        template<typename Self>
        constexpr auto dataPixel(this Self&& self, const size_t x, const size_t y) noexcept
        {
            assert(!self.mDataSource || self.mDataSource.get() == static_cast<BaseType&>(self).data().data());
            return std::span{self.mDataSource.get() + static_cast<BaseType&>(self).pixelOffset(x, y), TDataFormat::size};
        }

        void flip()
        {
            std::unique_ptr<TDataType[]> tempRow(new TDataType[BaseType::width() * TDataFormat::size]);

            auto rowSize = BaseType::width() * TDataFormat::size;
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

            auto destRowSize = BaseType::width() * TDataFormat::size;
            auto srcRowSize = imgView.width() * TDataFormat::size;

            if (!flipSource)
            {
                for (size_t curY = 0; curY < imgView.height(); curY++)
                {
                    auto destY = ((offsetY + curY) * destRowSize) + (offsetX * TDataFormat::size);
                    auto sourceY = (curY * srcRowSize);
                    std::memcpy(mDataSource.get() + destY, imgView.data().data() + sourceY, srcRowSize * sizeof(TDataType));
                }
            }
            else
            {
                for (size_t curY = 0; curY < imgView.height(); curY++)
                {
                    auto destY = ((offsetY + curY) * destRowSize) + (offsetX * TDataFormat::size);
                    auto sourceY = (imgView.height() - curY - 1) * srcRowSize;
                    std::memcpy(mDataSource.get() + destY, imgView.data().data() + sourceY, srcRowSize * sizeof(TDataType));
                }
            }

            return true;
        }

    protected:
        ImageBase() noexcept
          : BaseType(nullptr, 0, 0)
        {}

        ImageBase(std::unique_ptr<TDataType[]> data, size_t width, size_t height) noexcept
          : BaseType(data.get(), width, height), mDataSource{std::move(data)}
        {
            assert(mDataSource.get() == BaseType::data().data());
        }

    private:
        std::unique_ptr<TDataType[]> mDataSource;
    };

    template<typename TDataType, typename TDataFormat> // generic Image (any type supported)
        requires concepts::ImageFormat<TDataFormat, TDataType>
    class Image: public ImageBase<TDataType, TDataFormat>
    {
        using BaseType = ImageBase<TDataType, TDataFormat>;

    public:
        static Image create(size_t width, size_t height)
        {
            auto data = std::make_unique<TDataType[]>(width * height * TDataFormat::size);
            return Image{std::move(data), width, height};
        }

        static Image create(std::unique_ptr<TDataType[]> data, size_t width, size_t height) noexcept
        {
            return Image{std::move(data), width, height};
        }

    private:
        Image(std::unique_ptr<TDataType[]> data, size_t width, size_t height) noexcept
          : BaseType(std::move(data), width, height)
        {}

    public:
        Image() = default;
        ~Image() noexcept = default;

        Image(const Image&) = delete;
        Image& operator=(const Image&) = delete;
        Image(Image&& img) = default;
        Image& operator=(Image&& img) = default;
    };

    template<typename TDataFormat> // uint8_t specialization
    class Image<uint8_t, TDataFormat>: public ImageBase<uint8_t, TDataFormat>
    {
        using BaseType = ImageBase<uint8_t, TDataFormat>;

    public:
        static Image create(size_t width, size_t height)
        {
            auto data = std::make_unique<uint8_t[]>(width * height * TDataFormat::size);
            return Image{std::move(data), width, height};
        }

        static Image create(std::unique_ptr<uint8_t[]> data, size_t width, size_t height) noexcept
        {
            return Image{std::move(data), width, height};
        }

    private:
        Image(std::unique_ptr<uint8_t[]> data, size_t width, size_t height) noexcept
          : BaseType(std::move(data), width, height)
        {}

    public:
        Image() = default;
        ~Image() noexcept = default;

        Image(const Image&) = delete;
        Image& operator=(const Image&) = delete;
        Image(Image&& img) = default;
        Image& operator=(Image&& img) = default;

        void clear(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a)
        {
            std::array<uint8_t, 4> tmpPixel{r, g, b, a};
            std::span<const uint8_t, 4> tmpPixelSpan{tmpPixel};

            size_t curPos = 0;
            auto walkerPtr = BaseType::data();

            auto imgArea = BaseType::area();
            auto imgAreaBlock = (imgArea / 4) * 4;

            for (; curPos < imgAreaBlock; curPos += 4)
            {
                TDataFormat::template writeRGBA(walkerPtr.subspan(0 * TDataFormat::size), tmpPixelSpan);
                TDataFormat::template writeRGBA(walkerPtr.subspan(1 * TDataFormat::size), tmpPixelSpan);
                TDataFormat::template writeRGBA(walkerPtr.subspan(2 * TDataFormat::size), tmpPixelSpan);
                TDataFormat::template writeRGBA(walkerPtr.subspan(3 * TDataFormat::size), tmpPixelSpan);
                walkerPtr = walkerPtr.subspan(TDataFormat::size * 4);
            }

            for (; curPos < imgArea; curPos++)
            {
                TDataFormat::template writeRGBA(walkerPtr, tmpPixelSpan);
                walkerPtr = walkerPtr.subspan(TDataFormat::size);
            }
        }

        void multiplyAlpha()
        {
            if (!TDataFormat::hasAlpha())
                return;

            auto walkerPtr = BaseType::data();
            auto imgArea = BaseType::area();

            hr::Colorf pixelValue;
            std::array<uint8_t, 4> tmpPixel;

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

                walkerPtr += TDataFormat::size;
            }
        }

        void removeGamma()
        {
            if constexpr (!TDataFormat::hasAlpha())
            {
                auto walkerPtr = BaseType::data().data();
                auto walkerEnd = walkerPtr + (BaseType::area() * TDataFormat::size);

                for (; walkerPtr < walkerEnd; walkerPtr++)
                    *walkerPtr = hr::Colorf::convertColor(hr::Colorf::convertSRGB2Linear(*walkerPtr));
            }
            else
            {
                auto walkerPtr = BaseType::data().data();
                auto imgArea = BaseType::area();

                uint8_t tmpPixel[4];
                for (size_t curPos = 0; curPos < imgArea; curPos++)
                {
                    TDataFormat::template readRGB<uint8_t>(walkerPtr, tmpPixel, 0);
                    tmpPixel[0] = hr::Colorf::convertColor(hr::Colorf::convertSRGB2Linear(tmpPixel[0]));
                    tmpPixel[1] = hr::Colorf::convertColor(hr::Colorf::convertSRGB2Linear(tmpPixel[1]));
                    tmpPixel[2] = hr::Colorf::convertColor(hr::Colorf::convertSRGB2Linear(tmpPixel[2]));
                    TDataFormat::template writeRGB<uint8_t>(walkerPtr, tmpPixel);

                    walkerPtr += TDataFormat::size;
                }
            }
        }

        void setPixel(const size_t x, const size_t y, const hr::Colorf& pixelValue)
        {
            uint8_t tmpPixel[4];
            hr::Colorf::convertColor(tmpPixel, pixelValue.data(), true);

            TDataFormat::template writeRGBA(BaseType::data().data() + BaseType::pixelOffset(x, y), tmpPixel);
        }

        void setPixel(const size_t x, const size_t y, const uint8_t* const pixelValue)
        {
            TDataFormat::template writeRGBA(BaseType::data().data() + BaseType::pixelOffset(x, y), pixelValue);
        }

        void renormalizeNormals(bool expandPixels)
        {
            if (BaseType::empty() || TDataFormat::size < 3)
                return;

            auto walker = BaseType::data();

            if (expandPixels)
            {
                for (; !walker.empty(); walker = walker.subspan(TDataFormat::size))
                {
                    Vector3f vec;

                    hr::Colorf::convertColor(vec.data(), walker.first<3>());
                    vec.mad(2.0f, -1.0f).normalize().mad(0.5f, 0.5f);
                    hr::Colorf::convertColor(walker.first<3>(), vec.data());
                }
            }
            else
            {
                for (; !walker.empty(); walker = walker.subspan(TDataFormat::size))
                {
                    Vector3f vec;

                    hr::Colorf::convertColor(vec.data(), walker.first<3>());
                    vec.normalize();
                    hr::Colorf::convertColor(walker.first<3>(), vec.data());
                }
            }
        }

        template<typename TCallback>
        void transform(TCallback&& cb)
        {
            static_assert(std::is_invocable_r_v<bool, TCallback, hr::Colorf&>);

            if (BaseType::empty() || !cb)
                return;

            auto walkerPtr = BaseType::data().data();
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

                walkerPtr += TDataFormat::size;
            }
        }
    };

    template<typename TDataFormat> // float specialization
    class Image<float, TDataFormat>: public ImageBase<float, TDataFormat>
    {
        using BaseType = ImageBase<float, TDataFormat>;

    public:
        static Image create(size_t width, size_t height)
        {
            auto data = std::make_unique<float[]>(width * height * TDataFormat::size);
            return Image{std::move(data), width, height};
        }

        static Image create(std::unique_ptr<float[]> data, size_t width, size_t height) noexcept
        {
            return Image{std::move(data), width, height};
        }

    private:
        Image(std::unique_ptr<float[]> data, size_t width, size_t height) noexcept
          : BaseType(std::move(data), width, height)
        {}

    public:
        Image() = default;
        ~Image() noexcept = default;

        Image(const Image&) = delete;
        Image& operator=(const Image&) = delete;
        Image(Image&& img) = default;
        Image& operator=(Image&& img) = default;

        void clear(const float r, const float g, const float b, const float a)
        {
            float tmpPixel[] = {r, g, b, a};

            size_t curPos = 0;
            auto walkerPtr = BaseType::data();

            auto imgArea = BaseType::area();
            auto imgAreaBlock = (imgArea / 4) * 4;

            for (; curPos < imgAreaBlock; curPos += 4)
            {
                TDataFormat::template writeRGBA<float>(walkerPtr + (0 * TDataFormat::size), tmpPixel);
                TDataFormat::template writeRGBA<float>(walkerPtr + (1 * TDataFormat::size), tmpPixel);
                TDataFormat::template writeRGBA<float>(walkerPtr + (2 * TDataFormat::size), tmpPixel);
                TDataFormat::template writeRGBA<float>(walkerPtr + (3 * TDataFormat::size), tmpPixel);
                walkerPtr += (TDataFormat::size * 4);
            }

            for (; curPos < imgArea; curPos++)
            {
                TDataFormat::template writeRGBA<float>(walkerPtr, tmpPixel);
                walkerPtr += TDataFormat::size;
            }
        }

        void setPixel(const size_t x, const size_t y, const hr::Colorf& pixelValue)
        {
            TDataFormat::template writeRGBA<float>(BaseType::dataPixel(x, y), pixelValue.data());
        }

        void setPixel(const size_t x, const size_t y, const float* const pixelValue)
        {
            TDataFormat::template writeRGBA<float>(BaseType::dataPixel(x, y), pixelValue);
        }

        void transform(const std::function<bool(hr::Colorf&)>& cb)
        {
            if (BaseType::empty() || !cb)
                return;

            auto walkerPtr = BaseType::data();
            auto imgArea = BaseType::area();

            float tmpPixel[4];
            hr::Colorf pixelValue;

            for (size_t curPos = 0; curPos < imgArea; curPos++)
            {
                TDataFormat::template readRGBA<float>(walkerPtr, tmpPixel, 0.0f, 1.0f);

                pixelValue.set(tmpPixel);
                if (cb(pixelValue))
                {
                    pixelValue.write(tmpPixel);
                    TDataFormat::template writeRGBA<float>(walkerPtr, tmpPixel);
                }

                walkerPtr = walkerPtr.subspan(TDataFormat::size);
            }
        }

        void renormalizeNormals(bool expandPixels)
        {
            if (BaseType::empty() || TDataFormat::size < 3)
                return;

            auto walker = BaseType::data();

            if (expandPixels)
            {
                for (; !walker.empty(); walker = walker.subspan(TDataFormat::size))
                {
                    Vector3f vec(walker.first<3>());
                    vec.mad(2.0f, -1.0f).normalize().mad(0.5f, 0.5f).write(walker.data());
                }
            }
            else
            {
                for (; !walker.empty(); walker = walker.subspan(TDataFormat::size))
                {
                    Vector3f vec(walker.first<3>());
                    vec.normalize().write(walker.data());
                }
            }
        }
    };

    template<typename TDataType, typename TDataFormat>
        requires concepts::ImageFormat<TDataFormat, TDataType>
    auto ImageViewBase<TDataType, TDataFormat>::crop(size_t cropX, size_t cropY, size_t cropWidth, size_t cropHeight) const -> Image<TDataType, TDataFormat>
    {
        if (empty() || ((cropWidth * cropHeight) <= 0) || ((cropX + cropWidth) > mWidth) || ((cropY + cropHeight) > mHeight))
            return Image<TDataType, TDataFormat>();

        Image<TDataType, TDataFormat> newImg(cropWidth, cropHeight);

        auto srcRowSize = width() * TDataFormat::size;
        auto destRowSize = cropWidth * TDataFormat::size;

        for (size_t curY = 0; curY < cropHeight; curY++)
        {
            auto sourcePos = ((curY + cropY) * srcRowSize) + (cropX * TDataFormat::size);
            auto destPos = curY * destRowSize;

            std::memcpy(newImg.mDataPtr + destPos, mDataPtr + sourcePos, destRowSize * sizeof(TDataType));
        }

        return newImg;
    }

    template<typename TDataType, typename TDataFormat>
        requires concepts::ImageFormat<TDataFormat, TDataType>
    auto ImageViewBase<TDataType, TDataFormat>::clone() const -> Image<TDataType, TDataFormat>
    {
        auto newImg = Image<TDataType, TDataFormat>::create(mWidth, mHeight);

        std::memcpy(newImg.data().data(), mDataPtr, size());
        return newImg;
    }

    template<typename TDataType, typename TDataFormat>
        requires concepts::ImageFormat<TDataFormat, TDataType>
    auto ImageViewBase<TDataType, TDataFormat>::resize(size_t width, size_t height, bool assumeSRGB) const -> Image<TDataType, TDataFormat>
    {
        static_assert(std::is_same_v<uint8_t, TDataType> || std::is_same_v<float, TDataType>, "Unsupported data type for resize");
        static_assert(std::is_same_v<ImageFormatR, TDataFormat> || std::is_same_v<ImageFormatRA, TDataFormat> || std::is_same_v<ImageFormatRGB, TDataFormat> ||
                        std::is_same_v<ImageFormatRGBA, TDataFormat>,
                      "Unsupported data type for resize");

        stbir_pixel_layout pixelLayout;
        if constexpr (std ::is_same_v<ImageFormatR, TDataFormat>)
            pixelLayout = STBIR_1CHANNEL;
        else if constexpr (std ::is_same_v<ImageFormatRA, TDataFormat>)
            pixelLayout = STBIR_RA;
        else if constexpr (std ::is_same_v<ImageFormatRGB, TDataFormat>)
            pixelLayout = STBIR_RGB;
        else if constexpr (std ::is_same_v<ImageFormatRGBA, TDataFormat>)
            pixelLayout = STBIR_RGBA;

        if constexpr (std::is_same_v<uint8_t, TDataType>)
        {
            auto newImg = Image<uint8_t, TDataFormat>::create(width, height);

            if (assumeSRGB)
                stbir_resize_uint8_srgb(mDataPtr, mWidth, mHeight, 0, newImg.data().data(), width, height, 0, pixelLayout);
            else
                stbir_resize_uint8_linear(mDataPtr, mWidth, mHeight, 0, newImg.data().data(), width, height, 0, pixelLayout);

            return newImg;
        }
        else if constexpr (std::is_same_v<float, TDataType>)
        {
            assert(!assumeSRGB); // float is always linear

            auto newImg = Image<float, TDataFormat>::create(width, height);

            stbir_resize_float_linear(mDataPtr, mWidth, mHeight, 0, newImg.data().data(), width, height, 0, pixelLayout);

            return newImg;
        }
    }

    template<typename TDataType, typename TDataFormat>
        requires concepts::ImageFormat<TDataFormat, TDataType>
    template<typename TNewDataType, concepts::ImageFormat<TNewDataType> TNewDataFormat>
    auto ImageViewBase<TDataType, TDataFormat>::convert(TDataType defaultColorValue, TDataType defaultAlphaValue) const -> Image<TNewDataType, TNewDataFormat>
    {
        static_assert(std::is_arithmetic_v<TDataType>, "Data type must be arithmetic (e.g.: float, uint8_t, etc.)");

        // special clone case
        if constexpr (std::is_same_v<TDataType, TNewDataType> && std::is_same_v<TDataFormat, TNewDataFormat>)
        {
            return clone();
        }
        // the data type is the same (format changes)
        else if constexpr (std::is_same_v<TDataType, TNewDataType>)
        {
            auto newImg = Image<TNewDataType, TNewDataFormat>::create(mWidth, mHeight);

            // RGB -> RGBA
            if constexpr (std::is_same_v<TDataFormat, ImageFormatRGB> && std::is_same_v<TNewDataFormat, ImageFormatRGBA>)
            {
                auto numPixels = area();
                auto walker = mDataPtr;
                auto walkerOut = newImg.data().data();

                for (size_t pixel = 0; pixel < numPixels; ++pixel, walker += 3, walkerOut += 4)
                {
                    walkerOut[0] = walker[0];
                    walkerOut[1] = walker[1];
                    walkerOut[2] = walker[2];
                    walkerOut[3] = defaultAlphaValue;
                }
            }
            // RGBA->RGB
            else if constexpr (std::is_same_v<TDataFormat, ImageFormatRGBA> && std::is_same_v<TNewDataFormat, ImageFormatRGB>)
            {
                auto numPixels = area();
                auto walker = mDataPtr;
                auto walkerOut = newImg.data().data();

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
                        TDataFormat::template readRGBA<TDataType>(dataPixel(x, y), pixel, defaultColorValue, defaultAlphaValue);
                        TNewDataFormat::template writeRGBA<TNewDataType>(newImg.dataPixel(x, y), pixel);
                    }
                }
            }

            return newImg;
        }
        // the data format is the same (type changes)
        else if constexpr (std::is_same_v<TDataFormat, TNewDataFormat>)
        {
            auto newImg = Image<TNewDataType, TNewDataFormat>::create(mWidth, mHeight);

            // uint8_t <-> float
            if constexpr ((std::is_same_v<uint8_t, TDataType> || std::is_same_v<float, TDataType>) &&
                          (std::is_same_v<uint8_t, TNewDataType> || std::is_same_v<float, TNewDataType>))
            {
                auto walker = std::span(mDataPtr, area() * TDataFormat::size);
                auto walkerOut = newImg.data();
                assert(walker.size() == walkerOut.size());

                for (; walker.size() >= 4; walker = walker.subspan(4), walkerOut = walkerOut.subspan(4))
                    hr::Colorf::convertColor(walkerOut.first<4>(), walker.first<4>());

                for (; !walker.empty(); walker = walker.subspan(1), walkerOut = walkerOut.subspan(1))
                    walkerOut[0] = hr::Colorf::convertColor(walker[0]);
            }

            static_assert((std::is_same_v<uint8_t, TDataType> || std::is_same_v<float, TDataType>) &&
                            (std::is_same_v<uint8_t, TNewDataType> || std::is_same_v<float, TNewDataType>),
                          "Can only convert between uint8_t and float");

            return newImg;
        }

        return {};
    }

    template<typename TDataType, typename TDataFormat>
        requires concepts::ImageFormat<TDataFormat, TDataType>
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
