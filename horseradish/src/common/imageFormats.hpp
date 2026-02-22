#pragma once

namespace hr::imaging
{
    template<class TFormat>
    class ImageFormat
    {
        static constexpr size_t size()
        {
            return TFormat::template size();
        }

        static constexpr bool hasAlpha()
        {
            return TFormat::template hasAlpha();
        }

        template<typename T>
        static constexpr void readRGB(const T* const dataIn, T* const dataOut, const T defaultColorValue)
        {
            TFormat::template readRGB<T>(dataIn, dataOut, defaultColorValue);
        }

        template<typename T>
        static constexpr void readRGBA(const T* const dataIn, T* const dataOut, const T defaultColorValue, const T defaultAlphaValue)
        {
            TFormat::template readRGBA<T>(dataIn, dataOut, defaultColorValue, defaultAlphaValue);
        }

        template<typename T>
        static constexpr void writeRGB(T* const dataOut, const T* const pixelValue)
        {
            TFormat::template writeRGBA<T>(dataOut, pixelValue);
        }

        template<typename T>
        static constexpr void writeRGBA(T* const dataOut, const T* const pixelValue)
        {
            TFormat::template writeRGBA<T>(dataOut, pixelValue);
        }
    };

    struct ImageFormatR: ImageFormat<ImageFormatR>
    {
        static constexpr size_t size()
        {
            return 1;
        }

        static constexpr bool hasAlpha()
        {
            return false;
        }

        template<typename T>
        static constexpr void readRGB(const T* const dataIn, T* const dataOut, const T defaultColorValue)
        {
            dataOut[0] = dataIn[0];
            dataOut[1] = dataOut[2] = defaultColorValue;
        }

        template<typename T>
        static constexpr void readRGBA(const T* const dataIn, T* const dataOut, const T defaultColorValue, const T defaultAlphaValue)
        {
            dataOut[0] = dataIn[0];
            dataOut[1] = dataOut[2] = defaultColorValue;
            dataOut[3] = defaultAlphaValue;
        }

        template<typename T>
        static constexpr void writeRGB(T* const dataOut, const T* const pixelValue)
        {
            dataOut[0] = pixelValue[0];
        }

        template<typename T>
        static constexpr void writeRGBA(T* const dataOut, const T* const pixelValue)
        {
            dataOut[0] = pixelValue[0];
        }
    };

    struct ImageFormatRA: ImageFormat<ImageFormatRA>
    {
        static constexpr size_t size()
        {
            return 2;
        }

        static constexpr bool hasAlpha()
        {
            return true;
        }

        template<typename T>
        static constexpr void readRGB(const T* const dataIn, T* const dataOut, const T defaultColorValue)
        {
            dataOut[0] = dataIn[0];
            dataOut[1] = dataOut[2] = defaultColorValue;
        }

        template<typename T>
        static constexpr void readRGBA(const T* const dataIn, T* const dataOut, const T defaultColorValue, const T)
        {
            dataOut[0] = dataIn[0];
            dataOut[1] = dataOut[2] = defaultColorValue;
            dataOut[3] = dataIn[1];
        }

        template<typename T>
        static constexpr void writeRGB(T* const dataOut, const T* const pixelValue)
        {
            dataOut[0] = pixelValue[0];
        }

        template<typename T>
        static constexpr void writeRGBA(T* const dataOut, const T* const pixelValue)
        {
            dataOut[0] = pixelValue[0];
            dataOut[1] = pixelValue[3];
        }
    };

    struct ImageFormatRGB: ImageFormat<ImageFormatRGB>
    {
        static constexpr size_t size()
        {
            return 3;
        }

        static constexpr bool hasAlpha()
        {
            return false;
        }

        template<typename T>
        static constexpr void readRGB(const T* const dataIn, T* const dataOut, const T)
        {
            std::memcpy(dataOut, dataIn, sizeof(T) * 3);
        }

        template<typename T>
        static constexpr void readRGBA(const T* const dataIn, T* const dataOut, const T, const T defaultAlphaValue)
        {
            std::memcpy(dataOut, dataIn, sizeof(T) * 3);
            dataOut[3] = defaultAlphaValue;
        }

        template<typename T>
        static constexpr void writeRGB(T* const dataOut, const T* const pixelValue)
        {
            std::memcpy(dataOut, pixelValue, sizeof(T) * 3);
        }

        template<typename T>
        static constexpr void writeRGBA(T* const dataOut, const T* const pixelValue)
        {
            std::memcpy(dataOut, pixelValue, sizeof(T) * 3);
        }
    };

    struct ImageFormatRGBA: ImageFormat<ImageFormatRGBA>
    {
        static constexpr size_t size()
        {
            return 4;
        }

        static constexpr bool hasAlpha()
        {
            return true;
        }

        template<typename T>
        static constexpr void readRGB(const T* const dataIn, T* const dataOut, const T)
        {
            std::memcpy(dataOut, dataIn, sizeof(T) * 3);
        }

        template<typename T>
        static constexpr void readRGBA(const T* const dataIn, T* const dataOut, const T, const T)
        {
            std::memcpy(dataOut, dataIn, sizeof(T) * 4);
        }

        template<typename T>
        static constexpr void writeRGB(T* const dataOut, const T* const pixelValue)
        {
            std::memcpy(dataOut, pixelValue, sizeof(T) * 3);
        }

        template<typename T>
        static constexpr void writeRGBA(T* const dataOut, const T* const pixelValue)
        {
            std::memcpy(dataOut, pixelValue, sizeof(T) * 4);
        }
    };
}
