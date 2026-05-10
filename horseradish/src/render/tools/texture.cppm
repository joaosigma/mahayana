module;

#include <cstdint>

export module Texture;

import core;
import gal;

namespace hr::render::tools
{
    export
    class TextureTools final
    {
    public:
        // diffuse

        static bool uploadDiffuse(const hr::imaging::ImageView<uint8_t, hr::imaging::ImageFormatRGBA>& imageSrc, hr::gl::objects::Texture& textureDst);
        static bool uploadDiffuse(const hr::imaging::ImageView<uint8_t, hr::imaging::ImageFormatRGB>& imageSrc, hr::gl::objects::Texture& textureDst);
        static bool uploadDiffuse(const hr::imaging::ImageView<float, hr::imaging::ImageFormatRGB>& imageSrc, hr::gl::objects::Texture& textureDst);

        static bool uploadCompressedDiffuse(hr::streams::StreamReader& streamIn, hr::gl::objects::Texture& textureDst);

        static bool storeCompressedDiffuse(hr::streams::StreamWriter& streamOut, const hr::imaging::ImageView<uint8_t, hr::imaging::ImageFormatRGB>& imageSrc);
        static bool storeCompressedDiffuse(hr::streams::StreamWriter& streamOut, const hr::imaging::ImageView<uint8_t, hr::imaging::ImageFormatRGBA>& imageSrc);

        // normals

        static bool uploadNormal(const hr::imaging::ImageView<uint8_t, hr::imaging::ImageFormatRGB>& imageSrc, hr::gl::objects::Texture& textureDst);

        static bool uploadCompressedNormal(hr::streams::StreamReader& streamIn, hr::gl::objects::Texture& textureDst);

        static bool storeCompressedNormal(hr::streams::StreamWriter& streamOut, const hr::imaging::ImageView<uint8_t, hr::imaging::ImageFormatRGB>& imageSrc);
        static bool storeCompressedNormal(hr::streams::StreamWriter& streamOut, const hr::imaging::ImageView<uint8_t, hr::imaging::ImageFormatRGBA>& imageSrc);
    };
}
