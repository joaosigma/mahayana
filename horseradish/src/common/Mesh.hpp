#pragma once

#include "Vector.hpp"
#include "BVolumes.hpp"

#include <limits>
#include <memory>
#include <cassert>

namespace HorseRadish { namespace Geometry {
		
class Mesh
{
public:
	#pragma pack(1)
	struct VertexData
	{
		float pos[3];
		float uv[2];
		short normal[4];
		short tangent[4];
	};
	static_assert(sizeof(VertexData) == 36, "VertexData must be tightly packed: sizeof() == 36");

	static short pack(const float value);
	static float unpack(const short value);
	static void pack(const float* const in, short* const out, unsigned int numValues);
	static void unpack(const short* const in, float* const out, unsigned int numValues);

	static Mesh genBox(const float width, const float height, const float depth, const int precision);
	static Mesh genSphere(const float radius, const int slices, const int stacks);

private:
	std::unique_ptr<VertexData[]> mData;
	std::unique_ptr<unsigned short[]> mIndices;
	unsigned int mNumVertices, mNumIndices;

public:
	Mesh();
	Mesh(unsigned int numVertices, unsigned int numIndices);
	Mesh(std::unique_ptr<VertexData[]> vertices, unsigned int numVertices, std::unique_ptr<unsigned short[]> indices, unsigned int numIndices);

	Mesh(const Mesh& mesh);
	Mesh& operator=(const Mesh& mesh);

	Mesh(Mesh&& mesh)
		: mData(std::move(mesh.mData)), mIndices(std::move(mesh.mIndices)), mNumVertices(std::move(mesh.mNumVertices)), mNumIndices(std::move(mesh.mNumIndices))
	{ }

	Mesh& operator=(Mesh&& mesh)
	{
		mData = std::move(mesh.mData);
		mIndices = std::move(mesh.mIndices);

		mNumVertices = std::move(mesh.mNumVertices);
		mNumIndices = std::move(mesh.mNumIndices);

		return *this;
	}

	const VertexData* dataVertices() const
	{
		return mData.get();
	}

	VertexData* dataVertices()
	{
		return mData.get();
	}

	const unsigned short* dataIndices() const
	{
		return mIndices.get();
	}

	unsigned short* dataIndices()
	{
		return mIndices.get();
	}

	unsigned int sizeVertices() const;
	unsigned int sizeIndices() const;

	unsigned int numIndices() const;
	unsigned int numVertices() const;
	unsigned int numTris() const;

	bool check() const;

	BBox getBoundingBox() const;
	float getIndicesCacheRatio(unsigned int cacheSize) const;
	bool getRayIntersect(const Vector3f& rayOrigin, const Vector3f& rayDir, float& hitDistance) const;

	void flipUV();
	void scale(float scaleAmount);
	void translate(const Vector3f& translate);
	void centerMass(const Vector3f& center);
	void confine(float maxAxis);
	void confine(const Vector3f& center, float maxAxis);
	void invertTriWinding();
	void optimizeIndices();

	void genNormals();
	void genTangents4();
};

} }
