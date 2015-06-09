#pragma once

#include "..\objects.hpp"

#include <memory>

namespace HorseRadish { namespace OpenGL { namespace Tools {
			
class ImmediateMode
{
public:
	enum class GeometryType{ None, Quads, Tris, Lines, LineStrip };
	enum class InfoType{ FreeVertexCount, MaxVertexCount };

private:
	struct VertexDataLayout
	{
		float px, py, pz, tu, tv;
		unsigned char cr, cg, cb, ca;
	};

	struct
	{
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
	std::unique_ptr<VertexDataLayout[]> mBufferData;
	std::unique_ptr<unsigned short[]> mBufferIndices;
	int mMaxVertexCount;

	int draw();
	void resetState();
	bool checkStateDraw() const;

public:
	ImmediateMode(const int maxVertexCount);
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

