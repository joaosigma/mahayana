#pragma once

#include "../objects.hpp"
#include "common/vector.hpp"

#include <array>
#include <memory>

namespace hr::gl::tools
{
    class ImmediateMode
    {
    public:
        enum class GeometryType
        {
            None,
            Quads,
            Tris,
            Lines,
            LineStrip
        };
        enum class InfoType
        {
            FreeVertexCount,
            MaxVertexCount
        };

    private:
        static constexpr size_t MaxVertexCount = 400; // 100 quads
        static constexpr size_t MaxIndexCount = ((MaxVertexCount / 4) * 6) + 6;

        struct VertexDataLayout
        {
            float pos[3], uv[2];
            unsigned char color[4];
        };

        struct
        {
            hr::gl::objects::FenceSync fence;
            hr::gl::objects::VertexArray vertexArray;
            hr::gl::objects::Buffer arrayBuffer, elementArrayBuffer;
        } mGl;

        struct
        {
            float uv[2];
            size_t curVertex;
            GeometryType geomType;
            unsigned char color[4];
        } mState;

        std::array<VertexDataLayout, MaxVertexCount> mBufferData;
        std::array<unsigned short, MaxIndexCount> mBufferIndices;

        void draw(bool keepLeftovers);
        void resetState();
        bool checkStateDraw() const;

    public:
        ImmediateMode();
        ~ImmediateMode();

        void beginDraw(const GeometryType geometryType);
        void endDraw();

        void setTexCoord(const float u, const float v);
        void setColor(const unsigned char r, const unsigned char g, const unsigned char b);
        void setColor(const unsigned char r, const unsigned char g, const unsigned char b, const unsigned char a);
        void setColorF(const float rgb);
        void setColorF(const float rgb, const float a);
        void setColorF(const float r, const float g, const float b);
        void setColorF(const float r, const float g, const float b, const float a);
        void setColorRGB(const unsigned char* const values);
        void setColorRGB(const float* const values);

        void addPosition(const float x);
        void addPosition(const float x, const float y);
        void addPosition(const float x, const float y, const float z);
        void addPosition(const Vector3f& vec);

        void addQuad(const float x, const float y, const float width, const float height);
        void addQuadTexCoords(const float x, const float y, const float width, const float height, bool normalizedTexCoords);
        void addLine(const float x1, const float y1, const float x2, const float y2);
        void addLineH(const float x1, const float x2, const float y);
        void addLineV(const float x, const float y1, const float y2);

        size_t info(const InfoType infoType) const;
    };
}
