#pragma once

#include "..\objects.hpp"

#include <array>
#include <memory>

namespace HorseRadish { namespace OpenGL { namespace Tools {
			
class ImmediateMode
{
public:
	enum class GeometryType{ None, Quads, Tris, Lines, LineStrip };
	enum class InfoType{ FreeVertexCount, MaxVertexCount };

private:
	static constexpr unsigned int MaxVertexCount = 400; //100 quads
	static constexpr unsigned int MaxIndexCount = ((MaxVertexCount / 4) * 6) + 6;

	struct VertexDataLayout
	{
		float pos[3], uv[2];
		unsigned char color[4];
	};

	struct
	{
		HorseRadish::OpenGL::Objects::FenceSync fence;
		HorseRadish::OpenGL::Objects::VertexArray vertexArray;
		HorseRadish::OpenGL::Objects::Buffer arrayBuffer, elementArrayBuffer;
	} mGl;

	struct
	{
		float uv[2];
		int curVertex;
		GeometryType geomType;
		unsigned char color[4];
	} mState;

	std::array<VertexDataLayout, MaxVertexCount> mBufferData;
	std::array<unsigned short, MaxIndexCount> mBufferIndices;

	int draw();
	void resetState();
	bool checkStateDraw() const;

public:
	ImmediateMode();
	~ImmediateMode();

	void beginDraw(const GeometryType geometryType);
	int endDraw();
	
	void setTexCoord(const float &u, const float &v);
	void setColor(const unsigned char &r, const unsigned char &g, const unsigned char &b);
	void setColor(const unsigned char &r, const unsigned char &g, const unsigned char &b, const unsigned char &a);
	void setColorF(const float &rgb);
	void setColorF(const float &rgb, const float &a);
	void setColorF(const float &r, const float &g, const float &b);
	void setColorF(const float &r, const float &g, const float &b, const float &a);
	void setColorRGB(const unsigned char * const values);
	void setColorRGB(const float * const values);
	
	void addPosition(const float &x);
	void addPosition(const float &x, const float &y);
	void addPosition(const float &x, const float &y, const float &z);

	void addQuad(const float &x, const float &y, const float &width, const float &height);
	void addQuadTexCoords(const float &x, const float &y, const float &width, const float &height, const bool &normalizedTexCoords);
	void addLine(const float &x1, const float &y1, const float &x2, const float &y2);
	void addLineH(const float &x1, const float &x2, const float &y);
	void addLineV(const float &x, const float &y1, const float &y2);

	unsigned int getInfo(const InfoType infoType) const;
};

} } }

