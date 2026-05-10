module;

#include <cstddef>

#include "glcorearb.h"

export module gal:tools.immediateMode.impl;

import std;

import core;
import :tools;
import :opengl;

namespace hr::gl::tools
{
    void ImmediateMode::draw(bool keepLeftovers)
    {
        if (!checkStateDraw())
            return;

        size_t numElements = 0;

        if (mState.geomType == GeometryType::Quads)
        {
            numElements = mState.curVertex / 4;
            if (numElements > 0)
            {
                auto newIndexWriter = mBufferIndices.data() + (numElements * 6 - 6);

                for (size_t curQuad = mState.curVertex - 4; curQuad >= 0; curQuad -= 4)
                {
                    auto index1 = mBufferIndices[curQuad + 0];
                    auto index2 = mBufferIndices[curQuad + 1];
                    auto index3 = mBufferIndices[curQuad + 2];
                    auto index4 = mBufferIndices[curQuad + 3];

                    newIndexWriter[0] = newIndexWriter[3] = index1;
                    newIndexWriter[1] = index2;
                    newIndexWriter[2] = newIndexWriter[4] = index3;
                    newIndexWriter[5] = index4;
                    newIndexWriter -= 6;

                    if (curQuad < 4) // because curQuad is unsigned
                        break;
                }

                mGl.fence.wait();
                mGl.vertexArray.bind();
                mGl.arrayBuffer.writeData(mBufferData.data(), numElements * 4 * sizeof(VertexDataLayout), 0);
                mGl.elementArrayBuffer.writeData(mBufferIndices.data(), numElements * 6 * sizeof(unsigned short), 0);

                hr::gl::glDrawRangeElements(GL_TRIANGLES, 0, (numElements * 4) - 1, numElements * 6, GL_UNSIGNED_SHORT, (void*)0);
                mGl.fence.place();
            }
        }
        else if (mState.geomType == GeometryType::Tris)
        {
            numElements = mState.curVertex / 3;
            if (numElements > 0)
            {
                mGl.fence.wait();
                mGl.vertexArray.bind();
                mGl.arrayBuffer.writeData(mBufferData.data(), numElements * 3 * sizeof(VertexDataLayout), 0);
                mGl.elementArrayBuffer.writeData(mBufferIndices.data(), numElements * 3 * sizeof(unsigned short), 0);

                hr::gl::glDrawRangeElements(GL_TRIANGLES, 0, (numElements * 3) - 1, numElements * 3, GL_UNSIGNED_SHORT, (void*)0);
                mGl.fence.place();
            }
        }
        else if (mState.geomType == GeometryType::Lines)
        {
            numElements = mState.curVertex / 2;
            if (numElements > 0)
            {
                mGl.fence.wait();
                mGl.vertexArray.bind();
                mGl.arrayBuffer.writeData(mBufferData.data(), numElements * 2 * sizeof(VertexDataLayout), 0);
                mGl.elementArrayBuffer.writeData(mBufferIndices.data(), numElements * 2 * sizeof(unsigned short), 0);

                hr::gl::glDrawRangeElements(GL_LINES, 0, (numElements * 2) - 1, numElements * 2, GL_UNSIGNED_SHORT, (void*)0);
                mGl.fence.place();
            }
        }
        else if (mState.geomType == GeometryType::LineStrip)
        {
            if (mState.curVertex >= 2)
            {
                mGl.fence.wait();
                mGl.vertexArray.bind();
                mGl.arrayBuffer.writeData(mBufferData.data(), mState.curVertex * sizeof(VertexDataLayout), 0);
                mGl.elementArrayBuffer.writeData(mBufferIndices.data(), mState.curVertex * sizeof(unsigned short), 0);

                hr::gl::glDrawRangeElements(GL_LINE_STRIP, 0, mState.curVertex - 1, mState.curVertex, GL_UNSIGNED_SHORT, (void*)0);
                mGl.fence.place();
            }
        }

        if (!keepLeftovers)
        {
            mState.curVertex = 0;
            return;
        }

        if (mState.geomType == GeometryType::LineStrip)
        {
            mBufferData[0] = mBufferData[mState.curVertex - 1];
            mBufferIndices[0] = 0;
            mState.curVertex = 1;
            return;
        }

        if (numElements == 0)
            return;

        auto numLeftOvers = mState.curVertex - numElements;
        if (numLeftOvers <= 0)
            return;

        for (size_t i = 0; i < numLeftOvers; i++)
        {
            mBufferData[i] = mBufferData[numElements + i];
            mBufferIndices[i] = static_cast<unsigned short>(i);
        }

        mState.curVertex = numLeftOvers;
    }

    void ImmediateMode::resetState()
    {
        mState.curVertex = 0;
        mState.geomType = GeometryType::None;
        mState.uv[0] = mState.uv[1] = 0.0f;
        mState.color[0] = mState.color[1] = mState.color[2] = 0;
        mState.color[3] = 255;
    }

    bool ImmediateMode::checkStateDraw() const
    {
        if ((mState.geomType == GeometryType::None) || (mState.curVertex <= 0))
            return false;

        if ((mState.geomType == GeometryType::Quads) && (mState.curVertex >= 4))
            return true;

        if ((mState.geomType == GeometryType::Tris) && (mState.curVertex >= 3))
            return true;

        if (((mState.geomType == GeometryType::Lines) || (mState.geomType == GeometryType::LineStrip)) && (mState.curVertex >= 2))
            return true;

        return false;
    }

    ImmediateMode::ImmediateMode()
    {
        resetState();

        mGl.arrayBuffer.init(hr::gl::objects::Buffer::Type::ArrayBuffer, ImmediateMode::MaxVertexCount * sizeof(VertexDataLayout),
                             hr::gl::objects::Buffer::UsageType::PersistentOnlyWrite);
        mGl.elementArrayBuffer.init(hr::gl::objects::Buffer::Type::ElementArrayBuffer, sizeof(unsigned short) * ImmediateMode::MaxIndexCount,
                                    hr::gl::objects::Buffer::UsageType::PersistentOnlyWrite);

        mGl.vertexArray.init();

        hr::gl::glEnableVertexArrayAttrib(mGl.vertexArray.id(), 0);
        hr::gl::glEnableVertexArrayAttrib(mGl.vertexArray.id(), 1);
        hr::gl::glEnableVertexArrayAttrib(mGl.vertexArray.id(), 4);

        hr::gl::glVertexArrayAttribBinding(mGl.vertexArray.id(), 0, 0);
        hr::gl::glVertexArrayAttribFormat(mGl.vertexArray.id(), 0, 3, GL_FLOAT, false, offsetof(VertexDataLayout, pos));

        hr::gl::glVertexArrayAttribBinding(mGl.vertexArray.id(), 1, 0);
        hr::gl::glVertexArrayAttribFormat(mGl.vertexArray.id(), 1, 2, GL_FLOAT, false, offsetof(VertexDataLayout, uv));

        hr::gl::glVertexArrayAttribBinding(mGl.vertexArray.id(), 4, 0);
        hr::gl::glVertexArrayAttribFormat(mGl.vertexArray.id(), 4, 4, GL_UNSIGNED_BYTE, true, offsetof(VertexDataLayout, color));

        hr::gl::glVertexArrayElementBuffer(mGl.vertexArray.id(), mGl.elementArrayBuffer.id());
        hr::gl::glVertexArrayVertexBuffer(mGl.vertexArray.id(), 0, mGl.arrayBuffer.id(), 0, sizeof(VertexDataLayout));
    }

    ImmediateMode::~ImmediateMode()
    {
        resetState();

        mGl.arrayBuffer.reset();
        mGl.elementArrayBuffer.reset();
        mGl.vertexArray.reset();
    }

    void ImmediateMode::beginDraw(const GeometryType geometryType)
    {
        resetState();

        if (geometryType == GeometryType::None)
            return;

        mState.geomType = geometryType;
    }

    void ImmediateMode::endDraw()
    {
        draw(false);
        resetState();
    }

    void ImmediateMode::setTexCoord(const float u, const float v)
    {
        mState.uv[0] = u;
        mState.uv[1] = v;
    }

    void ImmediateMode::setColor(const unsigned char r, const unsigned char g, const unsigned char b)
    {
        mState.color[0] = r;
        mState.color[1] = g;
        mState.color[2] = b;
        mState.color[3] = 255;
    }

    void ImmediateMode::setColor(const unsigned char r, const unsigned char g, const unsigned char b, const unsigned char a)
    {
        mState.color[0] = r;
        mState.color[1] = g;
        mState.color[2] = b;
        mState.color[3] = a;
    }

    void ImmediateMode::setColorF(const float rgb)
    {
        mState.color[0] = Colorf::convertColor(rgb);
        mState.color[1] = mState.color[0];
        mState.color[2] = mState.color[0];
        mState.color[3] = 255;
    }

    void ImmediateMode::setColorF(const float rgb, const float a)
    {
        mState.color[0] = Colorf::convertColor(rgb);
        mState.color[1] = mState.color[0];
        mState.color[2] = mState.color[0];
        mState.color[3] = Colorf::convertColor(a);
    }

    void ImmediateMode::setColorF(const float r, const float g, const float b)
    {
        mState.color[0] = Colorf::convertColor(r);
        mState.color[1] = Colorf::convertColor(g);
        mState.color[2] = Colorf::convertColor(b);
        mState.color[3] = 255;
    }

    void ImmediateMode::setColorF(const float r, const float g, const float b, const float a)
    {
        mState.color[0] = Colorf::convertColor(r);
        mState.color[1] = Colorf::convertColor(g);
        mState.color[2] = Colorf::convertColor(b);
        mState.color[3] = Colorf::convertColor(a);
    }

    void ImmediateMode::setColorRGB(const unsigned char* const values)
    {
        mState.color[0] = values[0];
        mState.color[1] = values[1];
        mState.color[2] = values[2];
        mState.color[3] = 255;
    }

    void ImmediateMode::setColorRGB(const float* const values)
    {
        mState.color[0] = hr::Colorf::convertColor(values[0]);
        mState.color[1] = hr::Colorf::convertColor(values[1]);
        mState.color[2] = hr::Colorf::convertColor(values[2]);
        mState.color[3] = 255;
    }

    void ImmediateMode::addPosition(const float x)
    {
        addPosition(x, 0.0f, 0.0f);
    }

    void ImmediateMode::addPosition(const float x, const float y)
    {
        addPosition(x, y, 0.0f);
    }

    void ImmediateMode::addPosition(const float x, const float y, const float z)
    {
        if (mState.geomType == GeometryType::None)
            return;

        if (mState.curVertex >= ImmediateMode::MaxVertexCount)
            draw(true);

        auto& vertexData = mBufferData[mState.curVertex];
        vertexData.pos[0] = x;
        vertexData.pos[1] = y;
        vertexData.pos[2] = z;
        vertexData.uv[0] = mState.uv[0];
        vertexData.uv[1] = mState.uv[1];
        vertexData.color[0] = mState.color[0];
        vertexData.color[1] = mState.color[1];
        vertexData.color[2] = mState.color[2];
        vertexData.color[3] = mState.color[3];

        mBufferIndices[mState.curVertex] = static_cast<unsigned short>(mState.curVertex);

        mState.curVertex++;
    }

    void ImmediateMode::addPosition(const Vector3f& vec)
    {
        addPosition(vec[0], vec[1], vec[2]);
    }

    void ImmediateMode::addQuad(const float x, const float y, const float width, const float height)
    {
        if (mState.geomType != GeometryType::Quads)
            return;

        addPosition(x, y);
        addPosition(x, y + height);
        addPosition(x + width, y + height);
        addPosition(x + width, y);
    }

    void ImmediateMode::addQuadTexCoords(const float x, const float y, const float width, const float height, bool normalizedTexCoords)
    {
        if (mState.geomType != GeometryType::Quads)
            return;

        if (normalizedTexCoords)
        {
            mState.uv[0] = 0.0f;
            mState.uv[1] = 0.0f;
            addPosition(x, y);

            mState.uv[1] = 1.0f;
            addPosition(x, y + height);

            mState.uv[0] = 1.0f;
            addPosition(x + width, y + height);

            mState.uv[1] = 0.0f;
            addPosition(x + width, y);
        }
        else
        {
            mState.uv[0] = x;
            mState.uv[1] = y;
            addPosition(x, y);

            mState.uv[1] = y + height;
            addPosition(x, y + height);

            mState.uv[0] = x + width;
            addPosition(x + width, y + height);

            mState.uv[1] = y;
            addPosition(x + width, y);
        }
    }

    void ImmediateMode::addLine(const float x1, const float y1, const float x2, const float y2)
    {
        if (mState.geomType != GeometryType::Lines)
            return;

        addPosition(x1, y1);
        addPosition(x2, y2);
    }

    void ImmediateMode::addLineH(const float x1, const float x2, const float y)
    {
        if (mState.geomType != GeometryType::Lines)
            return;

        addPosition(x1, y);
        addPosition(x2, y);
    }

    void ImmediateMode::addLineV(const float x, const float y1, const float y2)
    {
        if (mState.geomType != GeometryType::Lines)
            return;

        addPosition(x, y1);
        addPosition(x, y2);
    }

    size_t ImmediateMode::info(const InfoType infoType) const
    {
        switch (infoType)
        {
            case InfoType::FreeVertexCount:
                return (ImmediateMode::MaxVertexCount - mState.curVertex);
            case InfoType::MaxVertexCount:
                return (ImmediateMode::MaxVertexCount);
        }

        return 0;
    }
}
