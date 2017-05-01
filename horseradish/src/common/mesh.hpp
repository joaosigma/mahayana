#pragma once

#include "vector.hpp"
#include "bvolumes.hpp"

#include <limits>
#include <memory>
#include <cassert>

namespace hr { namespace geom
{
	class Mesh
	{
	public:
		#pragma pack(push, 1)
		struct VertexData
		{
			float pos[3];
			float uv[2];
			short normal[4];
			short tangent[4];
		};
		#pragma pack(pop)
		static_assert(sizeof(VertexData) == 36, "VertexData must be tightly packed: sizeof() == 36");

		static short pack(const float value);
		static float unpack(const short value);
		static void pack(const float* const in, short* const out, size_t numValues);
		static void unpack(const short* const in, float* const out, size_t numValues);

		static size_t maxVertexCount();

		static Mesh genBox(float width, float height, float depth, size_t precision);
		static Mesh genSphere(const int slices, const int stacks);

	private:
		std::unique_ptr<VertexData[]> mData;
		std::unique_ptr<unsigned short[]> mIndices;
		size_t mNumVertices = 0, mNumIndices = 0;

	public:
		Mesh()
		{ }

		Mesh(size_t numVertices, size_t numIndices);
		Mesh(std::unique_ptr<VertexData[]> vertices, size_t numVertices, std::unique_ptr<unsigned short[]> indices, size_t numIndices);

		Mesh(const Mesh& mesh);
		Mesh& operator=(const Mesh& mesh);

		Mesh(Mesh&& mesh) = default;
		Mesh& operator=(Mesh&& mesh) = default;

		const VertexData* vertices() const
		{
			return mData.get();
		}

		VertexData* vertices()
		{
			return mData.get();
		}

		const unsigned short* indices() const
		{
			return mIndices.get();
		}

		unsigned short* indices()
		{
			return mIndices.get();
		}

		size_t sizeVertices() const;
		size_t sizeIndices() const;

		size_t numIndices() const;
		size_t numVertices() const;
		size_t numTris() const;

		bool check() const;

		BBox getBoundingBox() const;
		float getIndicesCacheRatio(size_t cacheSize) const;
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
