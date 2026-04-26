#include "common/imageFactory.hpp"

#include <lodepng.h>
#include <stb_image.h>
#include <tinyexr.h>

#include <memory>

namespace hr::imaging
{
    namespace
    {
        int stbIORead(void* user, char* data, int size)
        {
            auto streamReader = static_cast<hr::streams::StreamReader*>(user);
            return static_cast<int>(streamReader->stream().read({reinterpret_cast<std::byte*>(data), static_cast<size_t>(size)}));
        }

        void stbIOSkip(void* user, int n)
        {
            auto streamReader = static_cast<hr::streams::StreamReader*>(user);
            streamReader->skip(n);
        }

        int stbIOEof(void* user)
        {
            auto streamReader = static_cast<hr::streams::StreamReader*>(user);
            return !streamReader->canRead();
        }
    }

    Image<uint8_t, ImageFormatRGB> Factory::readPNG(const hr::streams::StreamReader& streamReader)
    {
        hr::streams::MemoryStream streamContent;
        streamReader.stream().cloneAllContent(
          [&streamContent](size_t size)
          {
              streamContent.truncate(size);
              return streamContent.data();
          });

        unsigned int outW = 0, outH = 0;
        uint8_t* outBuffer = nullptr;

        auto fileData = streamContent.data();

        if (lodepng_decode24(&outBuffer, &outW, &outH, reinterpret_cast<const uint8_t*>(fileData.data()), fileData.size()) != 0)
            return {};

        if (outBuffer)
            return Image<uint8_t, ImageFormatRGB>::create(std::unique_ptr<uint8_t[]>(outBuffer), outW, outH);

        return {};
    }

    Image<uint8_t, ImageFormatRGBA> Factory::readPNGWithAlpha(const hr::streams::StreamReader& streamReader)
    {
        hr::streams::MemoryStream streamContent;
        streamReader.stream().cloneAllContent(
          [&streamContent](size_t size)
          {
              streamContent.truncate(size);
              return streamContent.data();
          });

        unsigned int outW = 0, outH = 0;
        uint8_t* outBuffer = nullptr;

        auto fileData = streamContent.data();

        if (lodepng_decode32(&outBuffer, &outW, &outH, reinterpret_cast<const uint8_t*>(fileData.data()), fileData.size()) != 0)
            return {};

        if (outBuffer)
            return Image<uint8_t, ImageFormatRGBA>::create(std::unique_ptr<uint8_t[]>(outBuffer), outW, outH);

        return {};
    }

    bool Factory::savePNG(hr::streams::StreamWriter& streamWriter, const ImageView<uint8_t, ImageFormatRGB>& imgView)
    {
        if (imgView.empty())
            return false;

        uint8_t* bufferOut = nullptr;
        size_t bufferOutSize = 0;
        lodepng_encode24(&bufferOut, &bufferOutSize, imgView.data().data(), imgView.width(), imgView.height());

        if (!bufferOut || (bufferOutSize <= 0))
            return false;

        streamWriter.write<uint8_t>(std::span{bufferOut, bufferOutSize});

        free(bufferOut);

        return true;
    }

    bool Factory::savePNG(hr::streams::StreamWriter& streamWriter, const ImageView<uint8_t, ImageFormatRGBA>& imgView)
    {
        if (imgView.empty())
            return false;

        uint8_t* bufferOut = nullptr;
        size_t bufferOutSize = 0;
        lodepng_encode32(&bufferOut, &bufferOutSize, imgView.data().data(), imgView.width(), imgView.height());

        if (!bufferOut || (bufferOutSize <= 0))
            return false;

        streamWriter.write<uint8_t>(std::span{bufferOut, bufferOutSize});

        free(bufferOut);

        return true;
    }

    Image<uint8_t, ImageFormatRGBA> Factory::readTGA(hr::streams::StreamReader& streamReader)
    {
        stbi_io_callbacks ioCbS;
        ioCbS.read = stbIORead;
        ioCbS.skip = stbIOSkip;
        ioCbS.eof = stbIOEof;

        int imgWidth, imgHeight, imgNumComponents;

        auto imgData = stbi_load_from_callbacks(&ioCbS, &streamReader, &imgWidth, &imgHeight, &imgNumComponents, 4);
        if (!imgData)
            return {};

        return Image<uint8_t, ImageFormatRGBA>::create(std::unique_ptr<uint8_t[]>(imgData), imgWidth, imgHeight);
    }

    Image<uint8_t, ImageFormatRGB> Factory::readJPG(hr::streams::StreamReader& streamReader)
    {
        stbi_io_callbacks ioCbS;
        ioCbS.read = stbIORead;
        ioCbS.skip = stbIOSkip;
        ioCbS.eof = stbIOEof;

        int imgWidth, imgHeight, imgNumComponents;

        auto imgData = stbi_load_from_callbacks(&ioCbS, &streamReader, &imgWidth, &imgHeight, &imgNumComponents, 3);
        if (!imgData)
            return {};

        return Image<uint8_t, ImageFormatRGB>::create(std::unique_ptr<uint8_t[]>(imgData), imgWidth, imgHeight);
    }

    Image<float, ImageFormatRGB> Factory::readHDRI(hr::streams::StreamReader& streamReader)
    {
        stbi_io_callbacks ioCbS;
        ioCbS.read = stbIORead;
        ioCbS.skip = stbIOSkip;
        ioCbS.eof = stbIOEof;

        int imgWidth, imgHeight, imgNumComponents;

        auto imgData = stbi_loadf_from_callbacks(&ioCbS, &streamReader, &imgWidth, &imgHeight, &imgNumComponents, 3);
        if (!imgData)
            return {};

        return Image<float, ImageFormatRGB>::create(std::unique_ptr<float[]>(imgData), imgWidth, imgHeight);
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

            return {};
        }

        return Image<float, ImageFormatRGBA>::create(std::unique_ptr<float[]>(bufferOut), width, height);
    }
}
