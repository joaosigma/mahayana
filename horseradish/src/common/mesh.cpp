#pragma once

#include "mesh.hpp"

#include "math.hpp"
#include "vector.hpp"
#include "triangle.hpp"

#include "libs/mikktspace/mikktspace.h"
#include "libs/meshoptimizer/meshoptimizer.h"

#include <vector>
#include <limits>

namespace hr::geom
{
	namespace
	{
		struct PackedUVec {
			int x : 10;
			int y : 10;
			int z : 10;
			int w : 2;
		}; //corresponds to GL_INT_2_10_10_10_REV

		Vector3f packedReadNormal(uint32_t packed)
		{
			PackedUVec value;
			*(reinterpret_cast<uint32_t*>(&value)) = packed;

			Vector3f out;
			out[0] = static_cast<float>(value.x) / 511.0f;
			out[1] = static_cast<float>(value.y) / 511.0f;
			out[2] = static_cast<float>(value.z) / 511.0f;
			return out;
		}

		uint32_t packedWriteNormal(float normal[3])
		{
			const uint32_t xs = normal[0] < 0;
			const uint32_t ys = normal[1] < 0;
			const uint32_t zs = normal[2] < 0;
			return (0 |
				zs << 29 | ((uint32_t)(normal[2] * 511.0f + (zs << 9)) & 511) << 20 |
				ys << 19 | ((uint32_t)(normal[1] * 511.0f + (ys << 9)) & 511) << 10 |
				xs << 9 | ((uint32_t)(normal[0] * 511.0f + (xs << 9)) & 511));
		}

		Vector4f packedReadTangent(uint32_t packed)
		{
			PackedUVec value;
			*(reinterpret_cast<uint32_t*>(&value)) = packed;

			Vector4f out;
			out[0] = static_cast<float>(value.x) / 511.0f;
			out[1] = static_cast<float>(value.y) / 511.0f;
			out[2] = static_cast<float>(value.z) / 511.0f;
			out[3] = static_cast<float>(value.w);
			return out;
		}

		uint32_t packedWriteTangent(float tangent[4])
		{
			const uint32_t xs = tangent[0] < 0;
			const uint32_t ys = tangent[1] < 0;
			const uint32_t zs = tangent[2] < 0;
			const uint32_t ws = tangent[3] < 0;
			return (ws << 31 | ((uint32_t)(tangent[3] + (ws << 1)) & 1) << 30 |
				zs << 29 | ((uint32_t)(tangent[2] * 511.0f + (zs << 9)) & 511) << 20 |
				ys << 19 | ((uint32_t)(tangent[1] * 511.0f + (ys << 9)) & 511) << 10 |
				xs << 9 | ((uint32_t)(tangent[0] * 511.0f + (xs << 9)) & 511));
		}
	}

	Mesh<VertexFull, uint32_t> Mesh<VertexFull, uint32_t>::genBox(size_t precision)
	{
		precision = (precision <= 0) ? 1 : precision;

		Mesh mesh((precision + 1) * (precision + 1) * 6, precision * precision * 6 * 6);

		auto indicesPtr = mesh.mIndices.get();
		for (size_t faceIndex = 0; faceIndex < 6; ++faceIndex)
		{
			size_t indexOffset = ((precision + 1) * (precision + 1)) * faceIndex;

			for (size_t y = 0; y < precision; y++)
			{
				for (size_t x = 0; x < precision; x++)
				{
					indicesPtr[0] = static_cast<uint32_t>(indexOffset + (x + 0) + ((y + 0) * (precision + 1)));
					indicesPtr[1] = static_cast<uint32_t>(indexOffset + (x + 1) + ((y + 1) * (precision + 1)));
					indicesPtr[2] = static_cast<uint32_t>(indexOffset + (x + 0) + ((y + 1) * (precision + 1)));

					indicesPtr[3] = indicesPtr[0];
					indicesPtr[4] = static_cast<uint32_t>(indexOffset + (x + 1) + ((y + 0) * (precision + 1)));
					indicesPtr[5] = indicesPtr[1];

					indicesPtr += 6;
				}
			}
		}

		auto dataPtr = mesh.mData.get();

		auto posRes = 2.0f / static_cast<float>(precision);
		auto uvRes = 1.0f / static_cast<float>(precision);

		//left face
		for (size_t y = 0; y <= precision; y++)
		{
			auto posY = static_cast<float>(y)* posRes - 1.0f;
			auto uvV = static_cast<float>(y)* uvRes;

			for (size_t x = 0; x <= precision; x++)
			{
				dataPtr->pos[0] = -1.0f;
				dataPtr->pos[1] = posY;
				dataPtr->pos[2] = static_cast<float>(x)* posRes - 1.0f;

				dataPtr->uv[0] = static_cast<float>(x)* uvRes;
				dataPtr->uv[1] = uvV;

				dataPtr->normal[1] = dataPtr->normal[2] = 0.0f;
				dataPtr->normal[0] = -1.0f;

				dataPtr++;
			}
		}

		//front face
		for (size_t y = 0; y <= precision; y++)
		{
			auto posY = static_cast<float>(y)* posRes - 1.0f;
			auto uvV = static_cast<float>(y)* uvRes;

			for (size_t x = 0; x <= precision; x++)
			{
				dataPtr->pos[0] = static_cast<float>(x)* posRes - 1.0f;
				dataPtr->pos[1] = posY;
				dataPtr->pos[2] = 1.0f;

				dataPtr->uv[0] = static_cast<float>(x)* uvRes;
				dataPtr->uv[1] = uvV;

				dataPtr->normal[0] = dataPtr->normal[1] = 0.0f;
				dataPtr->normal[2] = 1.0f;

				dataPtr++;
			}
		}

		//left face
		for (size_t y = 0; y <= precision; y++)
		{
			auto posY = static_cast<float>(y)* posRes - 1.0f;
			auto uvV = static_cast<float>(y)* uvRes;

			for (size_t x = 0; x <= precision; x++)
			{
				dataPtr->pos[0] = 1.0f;
				dataPtr->pos[1] = posY;
				dataPtr->pos[2] = -(static_cast<float>(x)* posRes - 1.0f);

				dataPtr->uv[0] = static_cast<float>(x)* uvRes;
				dataPtr->uv[1] = uvV;

				dataPtr->normal[1] = dataPtr->normal[2] = 0.0f;
				dataPtr->normal[0] = 1.0f;

				dataPtr++;
			}
		}

		//back face
		for (size_t y = 0; y <= precision; y++)
		{
			auto posY = static_cast<float>(y)* posRes - 1.0f;
			auto uvV = static_cast<float>(y)* uvRes;

			for (size_t x = 0; x <= precision; x++)
			{
				dataPtr->pos[0] = -(static_cast<float>(x)* posRes - 1.0f);
				dataPtr->pos[1] = posY;
				dataPtr->pos[2] = -1.0f;

				dataPtr->uv[0] = static_cast<float>(x)* uvRes;
				dataPtr->uv[1] = uvV;

				dataPtr->normal[0] = dataPtr->normal[1] = 0.0f;
				dataPtr->normal[2] = -1.0f;

				dataPtr++;
			}
		}

		//top face
		for (size_t y = 0; y <= precision; y++)
		{
			auto posZ = static_cast<float>(y)* posRes - 1.0f;
			auto uvV = static_cast<float>(y)* uvRes;

			for (size_t x = 0; x <= precision; x++)
			{
				dataPtr->pos[0] = static_cast<float>(x)* posRes - 1.0f;
				dataPtr->pos[1] = 1.0f;
				dataPtr->pos[2] = -posZ;

				dataPtr->uv[0] = static_cast<float>(x)* uvRes;
				dataPtr->uv[1] = uvV;

				dataPtr->normal[0] = dataPtr->normal[2] = 0.0f;
				dataPtr->normal[1] = 1.0f;

				dataPtr++;
			}
		}

		//bottom face
		for (size_t y = 0; y <= precision; y++)
		{
			auto posZ = static_cast<float>(y)* posRes - 1.0f;
			auto uvV = static_cast<float>(y)* uvRes;

			for (size_t x = 0; x <= precision; x++)
			{
				dataPtr->pos[0] = static_cast<float>(x)* posRes - 1.0f;
				dataPtr->pos[1] = -1.0f;
				dataPtr->pos[2] = posZ;

				dataPtr->uv[0] = static_cast<float>(x)* uvRes;
				dataPtr->uv[1] = uvV;

				dataPtr->normal[0] = dataPtr->normal[2] = 0.0f;
				dataPtr->normal[1] = -1.0f;

				dataPtr++;
			}
		}

		return mesh;
	}

	Mesh<VertexFull, uint32_t> Mesh<VertexFull, uint32_t>::genSphere(size_t sDiv, size_t tDiv)
	{
		sDiv = (sDiv < 4) ? 4 : sDiv;
		tDiv = (tDiv < 4) ? 4 : tDiv;

		Mesh mesh(sDiv * tDiv, sDiv * tDiv * 6);

		{
			const float stepR = 1.0f / static_cast<float>(sDiv - 1);
			const float stepS = 1.0f / static_cast<float>(tDiv - 1);

			auto dataPtr = mesh.mData.get();
			for (size_t r = 0; r < sDiv; r++)
			{
				const float y = std::sin((Math::Pi<float> * -0.5f) + (Math::Pi<float> * static_cast<float>(r)* stepR));
				const float v = static_cast<float>(r)* stepR;

				const float sinAngZ = std::sin(Math::Pi<float> * static_cast<float>(r)* stepR);

				for (size_t s = 0; s < tDiv; s++)
				{
					const float angX = (Math::Pi<float> * 2.0f) * static_cast<float>(s)* stepS;
					const float x = std::cos(angX) * sinAngZ;
					const float z = std::sin(angX) * sinAngZ;

					dataPtr->uv[0] = 1.0f - (static_cast<float>(s)* stepS);
					dataPtr->uv[1] = v;

					dataPtr->pos[0] = x;
					dataPtr->pos[1] = y;
					dataPtr->pos[2] = z;

					dataPtr->normal[0] = x;
					dataPtr->normal[1] = y;
					dataPtr->normal[2] = z;

					dataPtr++;
				}
			}
		}

		{
			auto indicesPtr = mesh.mIndices.get();
			for (size_t r = 0; r < (sDiv - 1); r++)
			{
				for (size_t s = 0; s < (tDiv - 1); s++)
				{
					indicesPtr[0] = static_cast<uint32_t>((r + 1) * tDiv + s);
					indicesPtr[1] = static_cast<uint32_t>(r * tDiv + (s + 1));
					indicesPtr[2] = static_cast<uint32_t>(r * tDiv + s);

					indicesPtr[3] = indicesPtr[0];
					indicesPtr[4] = static_cast<uint32_t>((r + 1) * tDiv + (s + 1));
					indicesPtr[5] = indicesPtr[1];

					indicesPtr += 6;
				}
			}
		}

		return mesh;
	}

	Mesh<VertexFull, uint32_t> Mesh<VertexFull, uint32_t>::convertMesh(const Mesh<VertexShading, uint16_t>& source)
	{
		if (!source.check())
			return {};

		Mesh<VertexFull, uint32_t> newMesh{ source.numVertices(), source.numIndices() };
		if (!newMesh.check())
			return {};

		for (size_t i = 0; i < newMesh.mNumVertices; i++)
		{
			auto& srcData = source.mData[i];
			auto& dstData = newMesh.mData[i];

			std::memcpy(dstData.pos, srcData.pos, sizeof(float) * 3);

			types::unpackFloat(srcData.uv, dstData.uv, 2);

			packedReadNormal(srcData.normal).write(dstData.normal);
			packedReadTangent(srcData.tangent).write(dstData.tangent);
		}

		for (size_t i = 0; i < newMesh.mNumIndices; i++)
			newMesh.mIndices[i] = static_cast<uint32_t>(source.mIndices[i]);

		return newMesh;
	}

	BBox<> Mesh<VertexFull, uint32_t>::bbox() const noexcept
	{
		Vector3f vecMin, vecMax;

		{
			__m128 curPoint, minPoint, maxPoint;
			auto vertexData = mData.get();

			minPoint = maxPoint = _mm_loadu_ps(vertexData->pos);
			vertexData++;

			for (size_t i = 1; i < mNumVertices; i++, vertexData++)
			{
				curPoint = _mm_loadu_ps(vertexData->pos);
				minPoint = _mm_min_ps(minPoint, curPoint);
				maxPoint = _mm_max_ps(maxPoint, curPoint);
			}

			vecMin = Vector3f{ minPoint };
			vecMax = Vector3f{ maxPoint };
		}

		return BBox(vecMin, vecMax);
	}
	
	float Mesh<VertexFull, uint32_t>::indicesCacheRatio(size_t cacheSize) const noexcept
	{
		if (!mNumIndices || !cacheSize)
			return 0.0f;
		if (cacheSize >= mNumIndices)
			return 1.0f;

		std::vector<int> simCache(cacheSize, -1);

		size_t numHits = 0;
		for (size_t i = 0; i < mNumIndices; i++)
		{
			auto curIndex = static_cast<int>(mIndices[i]);

			bool cacheHit = false;
			for (auto& curCache : simCache)
			{
				cacheHit = (curCache == curIndex);
				if (cacheHit)
					break;
			}

			if (cacheHit)
			{
				numHits++;
				continue;
			}

			for (size_t j = (cacheSize - 1); j > 0; j--)
				simCache[j] = simCache[j - 1];;
			simCache[0] = curIndex;
		}

		return (static_cast<float>(numHits) / static_cast<float>(mNumIndices));
	}
	
	bool Mesh<VertexFull, uint32_t>::intersects(const Ray<Vector3d>& ray, double rayDistMin, double rayDistMax, Hit& hit) const noexcept
	{
		auto wasHit{ false };

		hit.rayT = std::numeric_limits<double>::infinity();

		for (size_t i = 0; i < mNumIndices; i += 3)
		{
			Triangle<Vector3d> tri{
				Vector3f{ mData[mIndices[i + 0]].pos }.convert<double>(),
				Vector3f{ mData[mIndices[i + 1]].pos }.convert<double>(),
				Vector3f{ mData[mIndices[i + 2]].pos }.convert<double>() };

			Triangle<Vector3d>::Hit triHit;
			if (!tri.intersects(ray, rayDistMin, rayDistMax, triHit))
				continue;

			if (triHit.rayT >= hit.rayT)
				continue;

			hit.rayT = triHit.rayT;
			hit.triIndex = i / 3;
			hit.barycentricU = static_cast<float>(triHit.barycentricU);
			hit.barycentricV = static_cast<float>(triHit.barycentricV);
			wasHit = true;
		}

		return wasHit;
	}

	Vector3f Mesh<VertexFull, uint32_t>::triNormal(size_t triIndex, float baryU, float baryV) const noexcept
	{
		if ((triIndex * 3) >= mNumIndices)
			return {};

		auto baryW = 1.0f - baryU - baryV;

		Vector3f n1(mData[mIndices[triIndex * 3 + 0]].normal);
		Vector3f n2(mData[mIndices[triIndex * 3 + 1]].normal);
		Vector3f n3(mData[mIndices[triIndex * 3 + 2]].normal);

		return Vector3f::calcNormalize((n1 * baryU) + (n2 * baryV) + (n3 * baryW));
	}
	
	void Mesh<VertexFull, uint32_t>::flipUV() noexcept
	{
		auto vertexData = mData.get();
		for (size_t i = 0; i < mNumVertices; i++, vertexData++)
			vertexData->uv[1] = 1.0f - vertexData->uv[1];
	}
	
	void Mesh<VertexFull, uint32_t>::mirrorUV() noexcept
	{
		auto vertexData = mData.get();
		for (size_t i = 0; i < mNumVertices; i++, vertexData++)
			vertexData->uv[0] = 1.0f - vertexData->uv[0];
	}
	
	void Mesh<VertexFull, uint32_t>::scaleUV(float scaleAmount) noexcept
	{
		scaleUV(scaleAmount, scaleAmount);
	}
	
	void Mesh<VertexFull, uint32_t>::scaleUV(float scaleU, float scaleV) noexcept
	{
		auto vertexData = mData.get();
		for (size_t i = 0; i < mNumVertices; i++, vertexData++)
		{
			vertexData->uv[0] *= scaleU;
			vertexData->uv[1] *= scaleV;
		}
	}
	
	void Mesh<VertexFull, uint32_t>::scale(float scaleAmount) noexcept
	{
		auto vertexData = mData.get();
		for (size_t i = 0; i < mNumVertices; i++, vertexData++)
		{
			vertexData->pos[0] *= scaleAmount;
			vertexData->pos[1] *= scaleAmount;
			vertexData->pos[2] *= scaleAmount;
		}
	}
	
	void Mesh<VertexFull, uint32_t>::translate(const Vector3f& translate) noexcept
	{
		auto vertexData = mData.get();
		for (size_t i = 0; i < mNumVertices; i++, vertexData++)
		{
			vertexData->pos[0] += translate[0];
			vertexData->pos[1] += translate[1];
			vertexData->pos[2] += translate[2];
		}
	}
	
	void Mesh<VertexFull, uint32_t>::centerMass(const Vector3f& center) noexcept
	{
		translate(center - bbox().center());
	}
	
	void Mesh<VertexFull, uint32_t>::confine(float maxAxis) noexcept
	{
		auto box = bbox();
		auto distance = (box.max() - box.min()).abs();

		scale(maxAxis / std::fmax(std::fmax(distance[0], distance[1]), distance[2]));
	}
	
	void Mesh<VertexFull, uint32_t>::transform(const Matrix& matFull, const Matrix3& matRot) noexcept
	{
		auto vertexData = mData.get();
		for (size_t i = 0; i < mNumVertices; i++, vertexData++)
		{
			matFull.transform(vertexData->pos);

			matRot.transform(vertexData->normal);
			matRot.transform(vertexData->tangent);
		}
	}
	
	void Mesh<VertexFull, uint32_t>::invertTriWinding() noexcept
	{
		auto indices = mIndices.get();
		for (size_t i = 0; i < mNumIndices; i += 3, indices += 3)
			std::swap(indices[0], indices[2]);
	}
	
	void Mesh<VertexFull, uint32_t>::optimizeIndices() noexcept
	{
		auto newIndices = std::unique_ptr<uint32_t[]>(new uint32_t[mNumIndices]);

		//this optimizes for the vertex cache (reads from mIndices and writes to newIndices)
		meshopt_optimizeVertexCache(newIndices.get(), mIndices.get(), mNumIndices, mNumVertices);

		//this optimizes for overdraw (reads from newIndices and writes to mIndices)
		meshopt_optimizeOverdraw(mIndices.get(), newIndices.get(), mNumIndices, mData.get()[0].pos, mNumVertices, sizeof(VertexFull), 1.05f);

		//we changed the indices (already written to mIndices), so now, we can change the position of vertex data to be more memory friendly using the new indices
		meshopt_optimizeVertexFetch(mData.get(), mIndices.get(), mNumIndices, mData.get(), mNumVertices, sizeof(VertexFull));
	}
	
	void Mesh<VertexFull, uint32_t>::genNormals() noexcept
	{
		auto normals = std::unique_ptr<Vector3f[]>(new Vector3f[mNumVertices]);

		for (size_t i = 0; i < mNumVertices; i++)
			normals[i].set(0.0f);

		for (size_t i = 0; i < mNumIndices; i += 3)
		{
			auto i1 = mIndices[i + 0];
			auto i2 = mIndices[i + 1];
			auto i3 = mIndices[i + 2];

			auto faceNormal = Triangle<Vector3f>::calcNormal(mData[i1].pos, mData[i2].pos, mData[i3].pos);

			normals[i1] += faceNormal;
			normals[i2] += faceNormal;
			normals[i3] += faceNormal;
		}

		for (size_t i = 0; i < mNumVertices; i++)
		{
			normals[i].normalize();
			normals[i].write(mData[i].normal);
		}
	}
	
	void Mesh<VertexFull, uint32_t>::genTangents4() noexcept
	{
		struct DataWrapper
		{
			size_t numIndices;
			VertexFull* vertexData;
			const uint32_t* indices;
		};

		SMikkTSpaceInterface inter;
		inter.m_getNumFaces = [](const SMikkTSpaceContext* pContext) -> int
		{
			auto instance = reinterpret_cast<DataWrapper*>(pContext->m_pUserData);
			return (instance->numIndices / 3);
		};

		inter.m_getNumVerticesOfFace = [](const SMikkTSpaceContext*, const int)
		{
			return 3;
		};

		inter.m_getPosition = [](const SMikkTSpaceContext* pContext, float fvPosOut[], const int iFace, const int iVert)
		{
			auto instance = reinterpret_cast<DataWrapper*>(pContext->m_pUserData);

			auto vData = instance->vertexData + instance->indices[(iFace * 3) + iVert];
			std::memcpy(fvPosOut, vData->pos, sizeof(float) * 3);
		};

		inter.m_getNormal = [](const SMikkTSpaceContext* pContext, float fvNormOut[], const int iFace, const int iVert)
		{
			auto instance = reinterpret_cast<DataWrapper*>(pContext->m_pUserData);

			auto vData = instance->vertexData + instance->indices[(iFace * 3) + iVert];
			std::memcpy(fvNormOut, vData->normal, sizeof(float) * 3);
		};

		inter.m_getTexCoord = [](const SMikkTSpaceContext* pContext, float fvTexcOut[], const int iFace, const int iVert)
		{
			auto instance = reinterpret_cast<DataWrapper*>(pContext->m_pUserData);

			auto vData = instance->vertexData + instance->indices[(iFace * 3) + iVert];
			std::memcpy(fvTexcOut, vData->uv, sizeof(float) * 2);
		};

		inter.m_setTSpace = nullptr;
		inter.m_setTSpaceBasic = [](const SMikkTSpaceContext* pContext, const float fvTangent[], const float fSign, const int iFace, const int iVert)
		{
			auto instance = reinterpret_cast<DataWrapper*>(pContext->m_pUserData);

			auto vData = instance->vertexData + instance->indices[(iFace * 3) + iVert];
			std::memcpy(vData->tangent, fvTangent, sizeof(float) * 3);
			vData->tangent[3] = fSign;
		};

		DataWrapper dataWrapper{ mNumIndices, mData.get(), mIndices.get() };

		SMikkTSpaceContext ctx;
		ctx.m_pInterface = &inter;
		ctx.m_pUserData = &dataWrapper;
		genTangSpaceDefault(&ctx);
	}

	Mesh<VertexShading, uint16_t> Mesh<VertexShading, uint16_t>::convertMesh(const Mesh<VertexFull, uint32_t>& source)
	{
		if (!source.check())
			return {};

		Mesh<VertexShading, uint16_t> newMesh{ source.numVertices(), source.numIndices() };
		if (!newMesh.check())
			return {};

		for (size_t i = 0; i < newMesh.mNumVertices; i++)
		{
			auto& srcData = source.mData[i];
			auto& dstData = newMesh.mData[i];

			std::memcpy(dstData.pos, srcData.pos, sizeof(float) * 3);

			types::packFloat(srcData.uv, dstData.uv, 2);

			dstData.normal = packedWriteNormal(srcData.normal);
			dstData.tangent = packedWriteTangent(srcData.tangent);
		}

		for (size_t i = 0; i < newMesh.mNumIndices; i++)
			newMesh.mIndices[i] = static_cast<uint16_t>(source.mIndices[i]);

		return newMesh;
	}

	BBox<> Mesh<VertexShading, uint16_t>::getBoundingBox() const noexcept
	{
		__m128 minPoint, maxPoint;

		{
			__m128 curPoint;
			auto vertexData = mData.get();

			minPoint = maxPoint = _mm_loadu_ps(vertexData->pos);
			vertexData++;

			for (size_t i = 1; i < mNumVertices; i++, vertexData++)
			{
				curPoint = _mm_loadu_ps(vertexData->pos);
				minPoint = _mm_min_ps(minPoint, curPoint);
				maxPoint = _mm_max_ps(maxPoint, curPoint);
			}
		}

		Vector3f tmpVecs[2];
		_mm_storeu_ps(tmpVecs[0].data(), minPoint);
		_mm_storeu_ps(tmpVecs[1].data(), maxPoint);

		return BBox(tmpVecs, 2);
	}

	Vector3f Mesh<VertexShading, uint16_t>::getPos(size_t vertexIndex) const noexcept
	{
		assert(vertexIndex < mNumVertices);
		return Vector3f{ mData[vertexIndex].pos };
	}

	Vector3f Mesh<VertexShading, uint16_t>::getNormal(size_t vertexIndex) const noexcept
	{
		assert(vertexIndex < mNumVertices);
		return packedReadNormal(mData[vertexIndex].normal);
	}

	Vector4f Mesh<VertexShading, uint16_t>::getTangent(size_t vertexIndex) const noexcept
	{
		assert(vertexIndex < mNumVertices);
		return packedReadTangent(mData[vertexIndex].tangent);
	}

	void Mesh<VertexShading, uint16_t>::setPos(size_t vertexIndex, Vector3f pos) noexcept
	{
		assert(vertexIndex < mNumVertices);
		pos.write(mData[vertexIndex].pos);
	}

	void Mesh<VertexShading, uint16_t>::setUV(size_t vertexIndex, float u, float v) noexcept
	{
		assert(vertexIndex < mNumVertices);
		mData[vertexIndex].uv[0] = types::packFloat<uint16_t>(u);
		mData[vertexIndex].uv[1] = types::packFloat<uint16_t>(v);
	}

	void Mesh<VertexShading, uint16_t>::setNormal(size_t vertexIndex, Vector3f normal) noexcept
	{
		assert(vertexIndex < mNumVertices);
		mData[vertexIndex].normal = packedWriteNormal(normal.data());
	}

	void Mesh<VertexShading, uint16_t>::setTangent(size_t vertexIndex, Vector4f tangent) noexcept
	{
		assert(vertexIndex < mNumVertices);
		mData[vertexIndex].tangent = packedWriteTangent(tangent.data());
	}

	void Mesh<VertexShading, uint16_t>::transform(const Matrix& matFull, const Matrix3& matRot) noexcept
	{
		auto vertexData = mData.get();
		for (size_t i = 0; i < mNumVertices; i++, vertexData++)
		{
			matFull.transform(vertexData->pos);

			{
				auto normal = packedReadNormal(vertexData->normal);
				matRot.transform(normal);
				vertexData->normal = packedWriteNormal(normal.data());
			}

			{
				auto tangent4 = packedReadTangent(vertexData->tangent);
				Vector3f tangent3{ tangent4.data() };
				matRot.transform(tangent3);

				tangent4 = Vector4f{ tangent3, tangent4[3] };
				vertexData->tangent = packedWriteTangent(tangent4.data());
			}
		}
	}
}
