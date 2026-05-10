export module core:imageFormats;

import std;

namespace hr::imaging
{
    namespace concepts
    {
        export template<class T, class TDataType>
        concept ImageFormat = requires(std::span<const TDataType> dataIn,
                                       std::span<const TDataType, 4> dataInRGBA,
                                       std::span<TDataType> dataOut,
                                       std::span<TDataType, 3> dataOutRGB,
                                       std::span<TDataType, 4> dataOutRGBA,
                                       TDataType defaultValue) {
            { T::size } -> std::common_reference_with<size_t>;
            { T::hasAlpha } -> std::common_reference_with<bool>;
            T::template readRGB<TDataType>(dataIn, dataOutRGB, defaultValue);
            T::template readRGBA<TDataType>(dataIn, dataOutRGBA, defaultValue, defaultValue);
            T::template writeRGB<TDataType>(dataOut, dataInRGBA);
            T::template writeRGBA<TDataType>(dataOut, dataInRGBA);
        };
    }

    export struct ImageFormatR
    {
        static constexpr size_t size = 1;
        static constexpr bool hasAlpha = false;

        template<typename T>
        static constexpr void readRGB(std::span<const T> dataIn, std::span<T, 3> dataOut, const T defaultColorValue)
        {
            dataOut[0] = dataIn[0];
            dataOut[1] = dataOut[2] = defaultColorValue;
        }

        template<typename T>
        static constexpr void readRGBA(std::span<const T> dataIn, std::span<T, 4> dataOut, const T defaultColorValue, const T defaultAlphaValue)
        {
            dataOut[0] = dataIn[0];
            dataOut[1] = dataOut[2] = defaultColorValue;
            dataOut[3] = defaultAlphaValue;
        }

        template<typename T>
        static constexpr void writeRGB(std::span<T> dataOut, std::span<const T, 4> pixelValue)
        {
            dataOut[0] = pixelValue[0];
        }

        template<typename T>
        static constexpr void writeRGBA(std::span<T> dataOut, std::span<const T, 4> pixelValue)
        {
            dataOut[0] = pixelValue[0];
        }
    };

    export struct ImageFormatRA
    {
        static constexpr size_t size = 2;
        static constexpr bool hasAlpha = true;

        template<typename T>
        static constexpr void readRGB(std::span<const T> dataIn, std::span<T, 3> dataOut, const T defaultColorValue)
        {
            dataOut[0] = dataIn[0];
            dataOut[1] = dataOut[2] = defaultColorValue;
        }

        template<typename T>
        static constexpr void readRGBA(std::span<const T> dataIn, std::span<T, 4> dataOut, const T defaultColorValue, const T)
        {
            dataOut[0] = dataIn[0];
            dataOut[1] = dataOut[2] = defaultColorValue;
            dataOut[3] = dataIn[1];
        }

        template<typename T>
        static constexpr void writeRGB(std::span<T> dataOut, std::span<const T, 4> pixelValue)
        {
            dataOut[0] = pixelValue[0];
        }

        template<typename T>
        static constexpr void writeRGBA(std::span<T> dataOut, std::span<const T, 4> pixelValue)
        {
            dataOut[0] = pixelValue[0];
            dataOut[1] = pixelValue[3];
        }
    };

    export struct ImageFormatRGB
    {
        static constexpr size_t size = 3;
        static constexpr bool hasAlpha = false;

        template<typename T>
        static constexpr void readRGB(std::span<const T> dataIn, std::span<T, 3> dataOut, const T)
        {
            std::memcpy(dataOut.data(), dataIn.data(), sizeof(T) * 3);
        }

        template<typename T>
        static constexpr void readRGBA(std::span<const T> dataIn, std::span<T, 4> dataOut, const T, const T defaultAlphaValue)
        {
            std::memcpy(dataOut.data(), dataIn.data(), sizeof(T) * 3);
            dataOut[3] = defaultAlphaValue;
        }

        template<typename T>
        static constexpr void writeRGB(std::span<T> dataOut, std::span<const T, 4> pixelValue)
        {
            std::memcpy(dataOut.data(), pixelValue.data(), sizeof(T) * 3);
        }

        template<typename T>
        static constexpr void writeRGBA(std::span<T> dataOut, std::span<const T, 4> pixelValue)
        {
            std::memcpy(dataOut.data(), pixelValue.data(), sizeof(T) * 3);
        }
    };

    export struct ImageFormatRGBA
    {
        static constexpr size_t size = 4;
        static constexpr bool hasAlpha = true;

        template<typename T>
        static constexpr void readRGB(std::span<const T> dataIn, std::span<T, 3> dataOut, const T)
        {
            std::memcpy(dataOut.data(), dataIn.data(), sizeof(T) * 3);
        }

        template<typename T>
        static constexpr void readRGBA(std::span<const T> dataIn, std::span<T, 4> dataOut, const T, const T)
        {
            std::memcpy(dataOut.data(), dataIn.data(), sizeof(T) * 4);
        }

        template<typename T>
        static constexpr void writeRGB(std::span<T> dataOut, std::span<const T, 4> pixelValue)
        {
            std::memcpy(dataOut.data(), pixelValue.data(), sizeof(T) * 3);
        }

        template<typename T>
        static constexpr void writeRGBA(std::span<T> dataOut, std::span<const T, 4> pixelValue)
        {
            std::memcpy(dataOut.data(), pixelValue.data(), sizeof(T) * 4);
        }
    };
}
