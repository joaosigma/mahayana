#pragma once

#include "vector.hpp"
#include "matrix.hpp"
#include "bvolumes.hpp"

#include <limits>
#include <memory>
#include <cassert>
#include <type_traits>

namespace hr::geom
{
	namespace detail
	{
		#pragma pack(push, 1)
		struct VertexData
		{
			float pos[3];
			float uv[2];
			int16_t normal[4]; //extra component at normal[3]
			int16_t tangent[4]; //tangent[3] is unused
		};
		#pragma pack(pop)

		static_assert(std::is_trivial_v<VertexData>);
		static_assert(sizeof(VertexData) == 36, "Mesh vertex data must be tightly packed: sizeof() == 36");
	}

	class Mesh
	{
	public:
		using VertexData = detail::VertexData;

		static int16_t pack(const float value);
		static float unpack(const int16_t value);
		static void pack(const float* const in, int16_t* const out, size_t numValues);
		static void unpack(const int16_t* const in, float* const out, size_t numValues);

		static constexpr size_t maxVertexCount()
		{
			return static_cast<size_t>(std::numeric_limits<uint16_t>::max());
		}

		static constexpr size_t sizeVertices(size_t numVertices)
		{
			return (sizeof(VertexData) * numVertices);
		}

		static constexpr size_t sizeIndices(size_t numIndices)
		{
			return (sizeof(uint16_t) * numIndices);
		}

		static Mesh genBox(size_t precision);
		static Mesh genSphere(size_t sDiv, size_t tDiv);

	private:
		std::unique_ptr<VertexData[]> mData;
		std::unique_ptr<uint16_t[]> mIndices;
		size_t mNumVertices = 0, mNumIndices = 0;

	public:
		Mesh() = default;

		Mesh(size_t numVertices, size_t numIndices);
		Mesh(std::unique_ptr<VertexData[]> vertices, size_t numVertices, std::unique_ptr<uint16_t[]> indices, size_t numIndices);

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

		const uint16_t* indices() const
		{
			return mIndices.get();
		}
		uint16_t* indices()
		{
			return mIndices.get();
		}

		size_t sizeVertices() const
		{
			return Mesh::sizeVertices(mNumVertices);
		}
		size_t sizeIndices() const
		{
			return Mesh::sizeIndices(mNumIndices);
		}

		size_t numIndices() const
		{
			return mNumIndices;
		}
		size_t numVertices() const
		{
			return mNumVertices;
		}
		size_t numTris() const
		{
			return mNumIndices / 3;
		}

		bool check() const;

		BBox getBoundingBox() const;
		float getIndicesCacheRatio(size_t cacheSize) const;
		bool getRayIntersect(const Vector3f& rayOrigin, const Vector3f& rayDir, float& hitDistance) const;

		void flipUV();
		void mirrorUV();
		void scaleUV(float scaleAmount);
		void scaleUV(float scaleU, float scaleV);

		void scale(float scaleAmount);
		void translate(const Vector3f& translate);
		void centerMass(const Vector3f& center);
		void confine(float maxAxis);
		void transform(const Matrix& matFull, const Matrix3& matRot);

		void invertTriWinding();
		void optimizeIndices();

		void genNormals();
		void genTangents4();
	};
}
