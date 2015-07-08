#include "immediateMode.hpp"

#include "common\Color.hpp"

namespace HorseRadish { namespace OpenGL { namespace Tools {

int ImmediateMode::draw()
{
	if (checkStateDraw() == false)
		return 0;

	int numElementosDesenhados = 0;

	if (mState.geomType == GeometryType::Quads)
	{
		numElementosDesenhados = mState.curVertex / 4;

		if (numElementosDesenhados > 0)
		{
			auto newIndexWriter = mBufferIndices.get() + (numElementosDesenhados * 6 - 6);

			for (int curQuad = mState.curVertex - 4; curQuad >= 0; curQuad -= 4)
			{
				unsigned short index1 = mBufferIndices[curQuad + 0];
				unsigned short index2 = mBufferIndices[curQuad + 1];
				unsigned short index3 = mBufferIndices[curQuad + 2];
				unsigned short index4 = mBufferIndices[curQuad + 3];

				newIndexWriter[0] = newIndexWriter[3] = index1;
				newIndexWriter[1] = index2;
				newIndexWriter[2] = newIndexWriter[4] = index3;
				newIndexWriter[5] = index4;
				newIndexWriter -= 6;
			}

			mGl.vertexArray.bind();
			mGl.arrayBuffer.writeData(mBufferData.get(), numElementosDesenhados * 4 * sizeof(VertexDataLayout), 0);
			mGl.elementArrayBuffer.writeData(mBufferIndices.get(), numElementosDesenhados * 6 * sizeof(unsigned short), 0);

			HorseRadish::OpenGL::glDrawRangeElements(GL_TRIANGLES, 0, numElementosDesenhados * 4, numElementosDesenhados * 6, GL_UNSIGNED_SHORT, (void*)0);
			HorseRadish::OpenGL::glBindVertexArray(0);
		}
	}
	else if (mState.geomType == GeometryType::Tris)
	{
		numElementosDesenhados = mState.curVertex / 3;

		if (numElementosDesenhados > 0)
		{
			mGl.vertexArray.bind();
			mGl.arrayBuffer.writeData(mBufferData.get(), numElementosDesenhados * 3 * sizeof(VertexDataLayout), 0);
			mGl.elementArrayBuffer.writeData(mBufferIndices.get(), numElementosDesenhados * 3 * sizeof(unsigned short), 0);

			HorseRadish::OpenGL::glDrawRangeElements(GL_TRIANGLES, 0, numElementosDesenhados * 3, numElementosDesenhados * 3, GL_UNSIGNED_SHORT, (void*)0);
			HorseRadish::OpenGL::glBindVertexArray(0);
		}
	}
	else if (mState.geomType == GeometryType::Lines)
	{
		numElementosDesenhados = mState.curVertex / 2;

		if (numElementosDesenhados > 0)
		{
			mGl.vertexArray.bind();
			mGl.arrayBuffer.writeData(mBufferData.get(), numElementosDesenhados * 2 * sizeof(VertexDataLayout), 0);
			mGl.elementArrayBuffer.writeData(mBufferIndices.get(), numElementosDesenhados * 2 * sizeof(unsigned short), 0);

			HorseRadish::OpenGL::glDrawRangeElements(GL_LINES, 0, numElementosDesenhados * 2, numElementosDesenhados * 2, GL_UNSIGNED_SHORT, (void*)0);
			HorseRadish::OpenGL::glBindVertexArray(0);
		}
	}
	else if (mState.geomType == GeometryType::LineStrip)
	{
		numElementosDesenhados = mState.curVertex - 1;

		if (numElementosDesenhados > 0)
		{
			mGl.vertexArray.bind();
			mGl.arrayBuffer.writeData(mBufferData.get(), mState.curVertex * sizeof(VertexDataLayout), 0);
			mGl.elementArrayBuffer.writeData(mBufferIndices.get(), mState.curVertex * sizeof(unsigned short), 0);

			HorseRadish::OpenGL::glDrawRangeElements(GL_LINE_STRIP, 0, mState.curVertex, mState.curVertex, GL_UNSIGNED_SHORT, (void*)0);
			HorseRadish::OpenGL::glBindVertexArray(0);
		}
	}

	mState.curVertex = 0;

	return numElementosDesenhados;
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

ImmediateMode::ImmediateMode(const int maxVertexCount)
	: mMaxVertexCount((maxVertexCount < 20) ? 20 : maxVertexCount)
{
	resetState();

	auto maxElementArray = ((mMaxVertexCount / 4) * 6) + 6;

	mGl.arrayBuffer.init(HorseRadish::OpenGL::Objects::Buffer::Type::ArrayBuffer, mMaxVertexCount * sizeof(VertexDataLayout), HorseRadish::OpenGL::Objects::Buffer::UsageType::FrequentOnlyWrite);
	mGl.elementArrayBuffer.init(HorseRadish::OpenGL::Objects::Buffer::Type::ElementArrayBuffer, sizeof(unsigned short) * maxElementArray, HorseRadish::OpenGL::Objects::Buffer::UsageType::FrequentOnlyWrite);

	mGl.vertexArray.init();

	HorseRadish::OpenGL::glEnableVertexArrayAttrib(mGl.vertexArray.getId(), 0);
	HorseRadish::OpenGL::glEnableVertexArrayAttrib(mGl.vertexArray.getId(), 1);
	HorseRadish::OpenGL::glEnableVertexArrayAttrib(mGl.vertexArray.getId(), 4);

	HorseRadish::OpenGL::glVertexArrayAttribBinding(mGl.vertexArray.getId(), 0, 0);
	HorseRadish::OpenGL::glVertexArrayAttribFormat(mGl.vertexArray.getId(), 0, 3, GL_FLOAT, false, 0);

	HorseRadish::OpenGL::glVertexArrayAttribBinding(mGl.vertexArray.getId(), 1, 0);
	HorseRadish::OpenGL::glVertexArrayAttribFormat(mGl.vertexArray.getId(), 1, 2, GL_FLOAT, false, 12);

	HorseRadish::OpenGL::glVertexArrayAttribBinding(mGl.vertexArray.getId(), 4, 0);
	HorseRadish::OpenGL::glVertexArrayAttribFormat(mGl.vertexArray.getId(), 4, 4, GL_UNSIGNED_BYTE, true, 20);

	HorseRadish::OpenGL::glVertexArrayElementBuffer(mGl.vertexArray.getId(), mGl.elementArrayBuffer.getId());
	HorseRadish::OpenGL::glVertexArrayVertexBuffer(mGl.vertexArray.getId(), 0, mGl.arrayBuffer.getId(), 0, sizeof(VertexDataLayout));

	mBufferData.reset(new VertexDataLayout[mMaxVertexCount]);
	mBufferIndices.reset(new unsigned short[maxElementArray]);
}

ImmediateMode::~ImmediateMode()
{
	resetState();

	mGl.arrayBuffer.reset();
	mGl.elementArrayBuffer.reset();
	mGl.vertexArray.reset();

	mBufferData.reset();
	mBufferIndices.reset();
}

void ImmediateMode::beginDraw(const GeometryType geometryType)
{
	resetState();

	if (geometryType == GeometryType::None)
		return;

	mState.geomType = geometryType;
}

int ImmediateMode::endDraw()
{
	auto numElementosDesenhados = draw();

	resetState();

	return numElementosDesenhados;
}

void ImmediateMode::setTexCoord(const float &u, const float &v)
{
	mState.uv[0] = u;
	mState.uv[1] = v;
}

void ImmediateMode::setColor(const unsigned char &r, const unsigned char &g, const unsigned char &b)
{
	mState.color[0] = r;
	mState.color[1] = g;
	mState.color[2] = b;
	mState.color[3] = 255;
}

void ImmediateMode::setColor(const unsigned char &r, const unsigned char &g, const unsigned char &b, const unsigned char &a)
{
	mState.color[0] = r;
	mState.color[1] = g;
	mState.color[2] = b;
	mState.color[3] = a;
}

void ImmediateMode::setColorF(const float &rgb)
{
	mState.color[0] = Color::convertColor(rgb);
	mState.color[1] = mState.color[0];
	mState.color[2] = mState.color[0];
	mState.color[3] = 255;
}

void ImmediateMode::setColorF(const float &rgb, const float &a)
{
	mState.color[0] = Color::convertColor(rgb);
	mState.color[1] = mState.color[0];
	mState.color[2] = mState.color[0];
	mState.color[3] = Color::convertColor(a);
}

void ImmediateMode::setColorF(const float &r, const float &g, const float &b)
{
	mState.color[0] = Color::convertColor(r);
	mState.color[1] = Color::convertColor(g);
	mState.color[2] = Color::convertColor(b);
	mState.color[3] = 255;
}

void ImmediateMode::setColorF(const float &r, const float &g, const float &b, const float &a)
{
	mState.color[0] = Color::convertColor(r);
	mState.color[1] = Color::convertColor(g);
	mState.color[2] = Color::convertColor(b);
	mState.color[3] = Color::convertColor(a);
}

void ImmediateMode::setColorRGB(const unsigned char * const values)
{
	mState.color[0] = values[0];
	mState.color[1] = values[1];
	mState.color[2] = values[2];
	mState.color[3] = 255;
}

void ImmediateMode::setColorRGB(const float * const values)
{
	mState.color[0] = HorseRadish::Color::convertColor(values[0]);
	mState.color[1] = HorseRadish::Color::convertColor(values[1]);
	mState.color[2] = HorseRadish::Color::convertColor(values[2]);
	mState.color[3] = 255;
}

void ImmediateMode::addPosition(const float &x)
{
	addPosition(x, 0.0f, 0.0f);
}

void ImmediateMode::addPosition(const float &x, const float &y)
{
	addPosition(x, y, 0.0f);
}

void ImmediateMode::addPosition(const float &x, const float &y, const float &z)
{
	if ((mState.curVertex >= mMaxVertexCount) || (mState.geomType == GeometryType::None))
		return;

	auto& vertexData = mBufferData[mState.curVertex];
	vertexData.px = x;
	vertexData.py = y;
	vertexData.pz = z;
	vertexData.tu = mState.uv[0];
	vertexData.tv = mState.uv[1];
	vertexData.cr = mState.color[0];
	vertexData.cg = mState.color[1];
	vertexData.cb = mState.color[2];
	vertexData.ca = mState.color[3];

	mBufferIndices[mState.curVertex] = mState.curVertex;

	mState.curVertex++;
}

void ImmediateMode::addQuad(const float &x, const float &y, const float &width, const float &height)
{
	if (mState.geomType != GeometryType::Quads)
		return;

	this->addPosition(x, y);
	this->addPosition(x + width, y);
	this->addPosition(x + width, y + height);
	this->addPosition(x, y + height);
}

void ImmediateMode::addQuadTexCoords(const float &x, const float &y, const float &width, const float &height, const bool &normalizedTexCoords)
{
	if (mState.geomType != GeometryType::Quads)
		return;

	if (normalizedTexCoords == true)
	{
		mState.uv[0] = 0.0f;
		mState.uv[1] = 0.0f;
		this->addPosition(x, y);
		mState.uv[0] = 1.0f;
		this->addPosition(x + width, y);
		mState.uv[1] = 1.0f;
		this->addPosition(x + width, y + height);
		mState.uv[0] = 0.0f;
		this->addPosition(x, y + height);
	}
	else
	{
		mState.uv[0] = x;
		mState.uv[1] = y;
		this->addPosition(x, y);
		mState.uv[0] = x + width;
		this->addPosition(x + width, y);
		mState.uv[1] = y + height;
		this->addPosition(x + width, y + height);
		mState.uv[0] = x;
		this->addPosition(x, y + height);
	}
}

void ImmediateMode::addLine(const float &x1, const float &y1, const float &x2, const float &y2)
{
	if (mState.geomType != GeometryType::Lines)
		return;

	this->addPosition(x1, y1);
	this->addPosition(x2, y2);
}

void ImmediateMode::addLineH(const float &x1, const float &x2, const float &y)
{
	if (mState.geomType != GeometryType::Lines)
		return;

	this->addPosition(x1, y);
	this->addPosition(x2, y);
}

void ImmediateMode::addLineV(const float &x, const float &y1, const float &y2)
{
	if (mState.geomType != GeometryType::Lines)
		return;

	this->addPosition(x, y1);
	this->addPosition(x, y2);
}

unsigned int ImmediateMode::getInfo(const InfoType infoType) const
{
	switch (infoType)
	{
	case InfoType::FreeVertexCount:
		return (mMaxVertexCount - mState.curVertex);
	case InfoType::MaxVertexCount:
		return (mMaxVertexCount);
	}

	return 0;
}

} } }