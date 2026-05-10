module;

#include <cstdint>

export module core:imageFactory;

import std;
import :image;
import :imageFormats;

namespace hr::imaging
{
    export struct Factory
    {
        static Image<uint8_t, ImageFormatRGB> readPNG(const hr::streams::StreamReader& streamReader);
        static Image<uint8_t, ImageFormatRGBA> readPNGWithAlpha(const hr::streams::StreamReader& streamReader);

        static bool savePNG(hr::streams::StreamWriter& streamWriter, const ImageView<uint8_t, ImageFormatRGB>& imgView);
        static bool savePNG(hr::streams::StreamWriter& streamWriter, const ImageView<uint8_t, ImageFormatRGBA>& imgView);

        static Image<uint8_t, ImageFormatRGBA> readTGA(hr::streams::StreamReader& streamReader);

        static Image<uint8_t, ImageFormatRGB> readJPG(hr::streams::StreamReader& streamReader);

        static Image<float, ImageFormatRGB> readHDRI(hr::streams::StreamReader& streamReader);

        static Image<float, ImageFormatRGBA> readEXR(const char* const fileName);
    };
}
