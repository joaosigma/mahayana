#pragma once

#include "serialize.hpp"
#include "stream.hpp"

namespace hr::serialize::archive
{
    class Stream final: public Archive<Stream>
    {
        streams::NullStream mNullStream;
        streams::StreamReader mReader;
        streams::StreamWriter mWriter;

    private:
        friend struct Archive<Stream>;
        friend class ArchiveReader<Stream>;
        friend class ArchiveWriter<Stream>;

        static constexpr bool isBinary() noexcept
        {
            return true;
        }

        template<typename T>
        T readFixed()
        {
            T val;
            mReader.read(val);
            return val;
        }

        bool readBytes(std::span<std::byte> buffer)
        {
            if (buffer.empty())
                return true;
            return (mReader.read(buffer.data(), buffer.size_bytes()) == buffer.size_bytes());
        }

        bool skipBytes(size_t size)
        {
            return mReader.skip(size);
        }

        void writeArrayBegin(const size_t numElements)
        {
            mWriter.write(static_cast<uint32_t>(numElements));
        }

        void writeArrayEnd()
        {}

        void writeObjectBegin()
        {}

        void writeObjectEnd()
        {}

        void writeObjectField(std::string_view)
        {}

        void writePtrState(const bool valid)
        {
            writeFixed<uint8_t>(valid ? 1 : 0);
        }

        template<typename T>
        void writeFixed(const T& val)
        {
            mWriter.write(val);
        }

        void writeString(std::string_view str)
        {
            mWriter.write(static_cast<uint32_t>(str.size()));
            if (str.size() > 0)
                mWriter.write(std::span{str.data(), str.size()});
        }

        void writeBuffer(std::span<const std::byte> buffer)
        {
            mWriter.write(static_cast<uint32_t>(buffer.size()));
            if (buffer.size() > 0)
                mWriter.write(buffer);
        }

        void writeBytes(std::span<const std::byte> buffer)
        {
            if (buffer.empty())
                return;
            mWriter.write(buffer);
        }

    public:
        Stream(streams::StreamReader reader)
          : mReader{std::move(reader)}, mWriter{mNullStream}
        {}

        Stream(streams::StreamWriter writer)
          : mReader{mNullStream}, mWriter{std::move(writer)}
        {}

        Stream(streams::StreamReader reader, streams::StreamWriter writer)
          : mReader{std::move(reader)}, mWriter{std::move(writer)}
        {}
    };
}
