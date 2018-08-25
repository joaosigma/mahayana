#pragma once

#include "mesh.hpp"
#include "math.hpp"
#include "vector.hpp"

#include "libs/forsyth/forsythtriangleorderoptimizer.h"
#include "libs/mikktspace/mikktspace.h"

#include <vector>
#include <limits>

namespace hr::geom
{
	namespace
	{
		const float shortScaleFrom = 1.0f / static_cast<float>(std::numeric_limits<short>::max());
		const float shortScaleTo = static_cast<float>(std::numeric_limits<short>::max());
	}

	short Mesh::pack(const float value)
	{
		return static_cast<short>(Math::fClamp(value, -1.0f, 1.0f) * shortScaleTo);
	}

	float Mesh::unpack(const short value)
	{
		return (static_cast<float>(value) * shortScaleFrom);
	}

	void Mesh::pack(const float* const in, short* const out, size_t numValues)
	{
		for (size_t i = 0; i < numValues; i++)
			out[i] = Mesh::pack(in[i]);
	}

	void Mesh::unpack(const short* const in, float* const out, size_t numValues)
	{
		for (size_t i = 0; i < numValues; i++)
			out[i] = Mesh::unpack(in[i]);
	}

	Mesh Mesh::genBox(size_t precision)
	{
		precision = (precision == 0) ? 1 : precision;

		Mesh mesh((precision + 1) * (precision + 1) * 6, precision * precision * 6 * 6);

		auto indicesPtr = mesh.mIndices.get();
		for (size_t faceIndex = 0; faceIndex < 6; ++faceIndex)
		{
			size_t indexOffset = ((precision + 1) * (precision + 1)) * faceIndex;

			for (size_t y = 0; y < precision; y++)
			{
				for (size_t x = 0; x < precision; x++)
				{
					indicesPtr[0] = static_cast<unsigned short>(indexOffset + (x + 0) + ((y + 0) * (precision + 1)));
					indicesPtr[1] = static_cast<unsigned short>(indexOffset + (x + 1) + ((y + 1) * (precision + 1)));
					indicesPtr[2] = static_cast<unsigned short>(indexOffset + (x + 0) + ((y + 1) * (precision + 1)));

					indicesPtr[3] = indicesPtr[0];
					indicesPtr[4] = static_cast<unsigned short>(indexOffset + (x + 1) + ((y + 0) * (precision + 1)));
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
			auto posY = static_cast<float>(y) * posRes - 1.0f;
			auto uvV = static_cast<float>(y) * uvRes;

			for (size_t x = 0; x <= precision; x++)
			{
				dataPtr->pos[0] = -1.0f;
				dataPtr->pos[1] = posY;
				dataPtr->pos[2] = static_cast<float>(x) * posRes - 1.0f;

				dataPtr->uv[0] = static_cast<float>(x) * uvRes;
				dataPtr->uv[1] = uvV;

				dataPtr->normal[1] = dataPtr->normal[1] = Mesh::pack(0.0f);
				dataPtr->normal[0] = Mesh::pack(-1.0f);

				dataPtr++;
			}
		}

		//front face
		for (size_t y = 0; y <= precision; y++)
		{
			auto posY = static_cast<float>(y) * posRes - 1.0f;
			auto uvV = static_cast<float>(y) * uvRes;

			for (size_t x = 0; x <= precision; x++)
			{
				dataPtr->pos[0] = static_cast<float>(x) * posRes - 1.0f;
				dataPtr->pos[1] = posY;
				dataPtr->pos[2] = 1.0f;
					
				dataPtr->uv[0] = static_cast<float>(x) * uvRes;
				dataPtr->uv[1] = uvV;
					
				dataPtr->normal[0] = dataPtr->normal[1] = Mesh::pack(0.0f);
				dataPtr->normal[2] = Mesh::pack(1.0f);

				dataPtr++;
			}
		}

		//left face
		for (size_t y = 0; y <= precision; y++)
		{
			auto posY = static_cast<float>(y) * posRes - 1.0f;
			auto uvV = static_cast<float>(y) * uvRes;

			for (size_t x = 0; x <= precision; x++)
			{
				dataPtr->pos[0] = 1.0f;
				dataPtr->pos[1] = posY;
				dataPtr->pos[2] = -(static_cast<float>(x) * posRes - 1.0f);

				dataPtr->uv[0] = static_cast<float>(x) * uvRes;
				dataPtr->uv[1] = uvV;

				dataPtr->normal[1] = dataPtr->normal[1] = Mesh::pack(0.0f);
				dataPtr->normal[0] = Mesh::pack(1.0f);

				dataPtr++;
			}
		}

		//back face
		for (size_t y = 0; y <= precision; y++)
		{
			auto posY = static_cast<float>(y) * posRes - 1.0f;
			auto uvV = static_cast<float>(y) * uvRes;

			for (size_t x = 0; x <= precision; x++)
			{
				dataPtr->pos[0] = -(static_cast<float>(x) * posRes - 1.0f);
				dataPtr->pos[1] = posY;
				dataPtr->pos[2] = -1.0f;

				dataPtr->uv[0] = static_cast<float>(x) * uvRes;
				dataPtr->uv[1] = uvV;

				dataPtr->normal[0] = dataPtr->normal[1] = Mesh::pack(0.0f);
				dataPtr->normal[2] = Mesh::pack(-1.0f);

				dataPtr++;
			}
		}

		//top face
		for (size_t y = 0; y <= precision; y++)
		{
			auto posZ = static_cast<float>(y) * posRes - 1.0f;
			auto uvV = static_cast<float>(y) * uvRes;

			for (size_t x = 0; x <= precision; x++)
			{
				dataPtr->pos[0] = static_cast<float>(x) * posRes - 1.0f;
				dataPtr->pos[1] = 1.0f;
				dataPtr->pos[2] = -posZ;

				dataPtr->uv[0] = static_cast<float>(x) * uvRes;
				dataPtr->uv[1] = uvV;

				dataPtr->normal[0] = dataPtr->normal[2] = Mesh::pack(0.0f);
				dataPtr->normal[1] = Mesh::pack(1.0f);

				dataPtr++;
			}
		}

		//bottom face
		for (size_t y = 0; y <= precision; y++)
		{
			auto posZ = static_cast<float>(y) * posRes - 1.0f;
			auto uvV = static_cast<float>(y) * uvRes;

			for (size_t x = 0; x <= precision; x++)
			{
				dataPtr->pos[0] = static_cast<float>(x) * posRes - 1.0f;
				dataPtr->pos[1] = -1.0f;
				dataPtr->pos[2] = posZ;

				dataPtr->uv[0] = static_cast<float>(x) * uvRes;
				dataPtr->uv[1] = uvV;

				dataPtr->normal[0] = dataPtr->normal[2] = Mesh::pack(0.0f);
				dataPtr->normal[1] = Mesh::pack(-1.0f);

				dataPtr++;
			}
		}

		return mesh;
	}

	Mesh Mesh::genSphere(size_t sDiv, size_t tDiv)
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
				const float y = sin(Math::constPiScaled(-0.5f) + (Math::constPi() * static_cast<float>(r) * stepR));
				const float v = static_cast<float>(r) * stepR;

				const float sinAngZ = sin(Math::constPi() * static_cast<float>(r) * stepR);

				for (size_t s = 0; s < tDiv; s++)
				{
					const float angX = Math::constPiScaled(2.0f) * static_cast<float>(s) * stepS;
					const float x = cos(angX) * sinAngZ;
					const float z = sin(angX) * sinAngZ;

					dataPtr->uv[0] = 1.0f - (static_cast<float>(s) * stepS);
					dataPtr->uv[1] = v;

					dataPtr->pos[0] = x;
					dataPtr->pos[1] = y;
					dataPtr->pos[2] = z;

					dataPtr->normal[0] = Mesh::pack(x);
					dataPtr->normal[1] = Mesh::pack(y);
					dataPtr->normal[2] = Mesh::pack(z);

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
					indicesPtr[0] = static_cast<unsigned short>((r + 1) * tDiv + s);
					indicesPtr[1] = static_cast<unsigned short>(r * tDiv + (s + 1));
					indicesPtr[2] = static_cast<unsigned short>(r * tDiv + s);

					indicesPtr[3] = indicesPtr[0];
					indicesPtr[4] = static_cast<unsigned short>((r + 1) * tDiv + (s + 1));
					indicesPtr[5] = indicesPtr[1];

					indicesPtr += 6;
				}
			}
		}

		return mesh;
	}

	Mesh::Mesh(size_t numVertices, size_t numIndices)
		: mNumVertices(numVertices), mNumIndices(numIndices)
	{
		assert((numVertices > 0) && (numIndices > 0));
		assert(numVertices < Mesh::maxVertexCount());
		assert((numIndices % 3) == 0);

		mData = std::unique_ptr<VertexData[]>(new VertexData[numVertices]);
		mIndices = std::unique_ptr<unsigned short[]>(new unsigned short[numIndices]);

		std::memset(mData.get(), 0, sizeof(VertexData) * numVertices);
		std::memset(mIndices.get(), 0, sizeof(unsigned short) * numIndices);
	}

	Mesh::Mesh(std::unique_ptr<VertexData[]> vertices, size_t numVertices, std::unique_ptr<unsigned short[]> indices, size_t numIndices)
		: mData(std::move(vertices)), mNumVertices(numVertices), mIndices(std::move(indices)), mNumIndices(numIndices)
	{
		assert((numVertices > 0) && (numIndices > 0));
		assert(numVertices < Mesh::maxVertexCount());
		assert((numIndices % 3) == 0);

		assert(mData && mIndices);
	}

	Mesh::Mesh(const Mesh& mesh)
		: mNumVertices(mesh.mNumVertices), mNumIndices(mesh.mNumIndices)
	{
		mData = std::unique_ptr<VertexData[]>(new VertexData[mNumVertices]);
		mIndices = std::unique_ptr<unsigned short[]>(new unsigned short[mNumIndices]);

		std::memcpy(mData.get(), mesh.mData.get(), sizeof(VertexData) * mNumVertices);
		std::memcpy(mIndices.get(), mesh.mIndices.get(), sizeof(unsigned short) * mNumIndices);
	}

	Mesh& Mesh::operator=(const Mesh& mesh)
	{
		mNumVertices = mesh.mNumVertices;
		mNumIndices = mesh.mNumIndices;

		mData = std::unique_ptr<VertexData[]>(new VertexData[mNumVertices]);
		mIndices = std::unique_ptr<unsigned short[]>(new unsigned short[mNumIndices]);

		std::memcpy(mData.get(), mesh.mData.get(), sizeof(VertexData) * mNumVertices);
		std::memcpy(mIndices.get(), mesh.mIndices.get(), sizeof(unsigned short) * mNumIndices);

		return *this;
	}

	bool Mesh::check() const
	{
		if (!mData || !mIndices || (mNumVertices <= 0) && (mNumIndices <= 0))
			return false;

		if ((mNumIndices % 3) != 0)
			return false;

		for (size_t i = 0; i < mNumIndices; i++)
		{
			if (mIndices[i] >= mNumVertices)
				return false;
		}

		return true;
	}

	BBox Mesh::getBoundingBox() const
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

	float Mesh::getIndicesCacheRatio(size_t cacheSize) const
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

	bool Mesh::getRayIntersect(const Vector3f& rayOrigin, const Vector3f& rayDir, float& hitDistance) const
	{
		hitDistance = 0.0f;

		float minHistDist = std::numeric_limits<float>::max();
		auto hit = false;

		for (size_t i = 0; i < mNumIndices; i += 3)
		{
			Vector3f p1(mData[mIndices[i * 3 + 0]].pos);
			Vector3f p2(mData[mIndices[i * 3 + 1]].pos);
			Vector3f p3(mData[mIndices[i * 3 + 2]].pos);

			Vector3f normal;
			normal.storeNormal(p1, p2, p3);
			normal.normalize();

			if (normal.getDot(rayDir) > 0.0f) //normal isn't facing the ray (we're simulating culling faces)
				continue;

			Vector3f hitPoint;
			{
				hr::Vector3f edge1, edge2, tvec, pvec, qvec;
				float det, u, v;

				//find vectors for two edges sharing vert0
				edge1 = p2 - p1;
				edge2 = p3 - p1;

				//begin calculating determinant - also used to calculate U parameter
				pvec.storeCrossProduct(rayDir, edge2);

				//check if ray is in the same plane as the tri
				det = edge1.getDot(pvec);
				if (hr::Math::isZero(det))
					continue;

				//calculate distance from vert0 to ray origin
				tvec = rayOrigin - p1;
				qvec.storeCrossProduct(tvec, edge1);

				u = tvec.getDot(pvec);
				if (det > 0.0f)
				{
					if (u<0.0 || u>det)
						continue;

					v = qvec.getDot(rayDir);
					if ((v < 0.0) || ((u + v) > det))
						continue;
				}
				else
				{
					if (u > 0.0 || u < det)
						continue;

					v = qvec.getDot(rayDir);
					if ((v > 0.0) || ((u + v) < det))
						continue;
				}

				det = 1.0f / det;
				u *= det;
				v *= det;

				hitPoint = (p1 * (1.0f - u - v)) + (p2 * u) + (p3 * v);
			}

			hit = true;
			minHistDist = std::fmin(minHistDist, hitPoint.getDistance(rayOrigin));
		}

		if (!hit)
			return false;

		hitDistance = minHistDist;
		return true;
	}

	void Mesh::flipUV()
	{
		auto vertexData = mData.get();
		for (size_t i = 0; i < mNumVertices; i++, vertexData++)
			vertexData->uv[1] = 1.0f - vertexData->uv[1];
	}

	void Mesh::mirrorUV()
	{
		auto vertexData = mData.get();
		for (size_t i = 0; i < mNumVertices; i++, vertexData++)
			vertexData->uv[0] = 1.0f - vertexData->uv[0];
	}

	void Mesh::scaleUV(float scaleAmount)
	{
		scaleUV(scaleAmount, scaleAmount);
	}

	void Mesh::scaleUV(float scaleU, float scaleV)
	{
		auto vertexData = mData.get();
		for (size_t i = 0; i < mNumVertices; i++, vertexData++)
		{
			vertexData->uv[0] *= scaleU;
			vertexData->uv[1] *= scaleV;
		}
	}

	void Mesh::scale(float scaleAmount)
	{
		auto vertexData = mData.get();
		for (size_t i = 0; i < mNumVertices; i++, vertexData++)
		{
			vertexData->pos[0] *= scaleAmount;
			vertexData->pos[1] *= scaleAmount;
			vertexData->pos[2] *= scaleAmount;
		}
	}

	void Mesh::translate(const Vector3f& translate)
	{
		auto vertexData = mData.get();
		for (size_t i = 0; i < mNumVertices; i++, vertexData++)
		{
			vertexData->pos[0] += translate[0];
			vertexData->pos[1] += translate[1];
			vertexData->pos[2] += translate[2];
		}
	}

	void Mesh::centerMass(const Vector3f& center)
	{
		translate(center - getBoundingBox().center());
	}

	void Mesh::confine(float maxAxis)
	{
		auto bbox = getBoundingBox();
		auto distance = (bbox.max() - bbox.min()).abs();

		scale(maxAxis / std::fmax(std::fmax(distance[0], distance[1]), distance[2]));
	}

	void Mesh::transform(const Matrix& matFull, const Matrix3& matRot)
	{
		auto vertexData = mData.get();
		for (size_t i = 0; i < mNumVertices; i++, vertexData++)
		{
			//position
			{
				Vector3f pos(vertexData->pos);
				matFull.transform(pos);
				pos.write(vertexData->pos);
			}

			//normal
			{
				Vector3f normal;
				Mesh::unpack(vertexData->normal, normal.data(), 3);
				matRot.transform(normal);
				Mesh::pack(normal.data(), vertexData->normal, 3);
			}
			
			//tangent
			{
				Vector3f tangent;
				Mesh::unpack(vertexData->tangent, tangent.data(), 3);
				matRot.transform(tangent);
				Mesh::pack(tangent.data(), vertexData->tangent, 3);
			}
		}
	}

	void Mesh::invertTriWinding()
	{
		auto indices = mIndices.get();
		for (size_t i = 0; i < mNumIndices; i += 3, indices += 3)
			std::swap(indices[0], indices[2]);
	}

	void Mesh::optimizeIndices()
	{
		auto newIndices = std::unique_ptr<unsigned short[]>(new unsigned short[mNumIndices]);
		Forsyth::OptimizeFaces(mIndices.get(), mNumIndices, mNumVertices, newIndices.get(), 32);

		std::swap(mIndices, newIndices);
	}

	void Mesh::genNormals()
	{
		auto normals = std::unique_ptr<Vector3f[]>(new Vector3f[mNumVertices]);

		for (size_t i = 0; i < mNumVertices; i++)
			normals[i].set(0.0f);

		for (size_t i = 0; i < mNumIndices; i += 3)
		{
			auto i1 = mIndices[i + 0];
			auto i2 = mIndices[i + 1];
			auto i3 = mIndices[i + 2];

			Vector3f faceNormal;
			faceNormal.storeNormal(mData[i1].pos, mData[i2].pos, mData[i3].pos);

			normals[i1] += faceNormal;
			normals[i2] += faceNormal;
			normals[i3] += faceNormal;
		}

		for (size_t i = 0; i < mNumVertices; i++)
		{
			normals[i].normalize();
			Mesh::pack(normals[i].data(), mData[i].normal, 3);
		}
	}

	void Mesh::genTangents4()
	{
		struct DataWrapper
		{
			size_t numIndices;
			VertexData* vertexData;
			const unsigned short* indices;
		};

		SMikkTSpaceInterface inter;
		inter.m_getNumFaces = [](const SMikkTSpaceContext * pContext) -> int
		{
			auto instance = reinterpret_cast<DataWrapper*>(pContext->m_pUserData);
			return (instance->numIndices / 3);
		};

		inter.m_getNumVerticesOfFace = [](const SMikkTSpaceContext *, const int)
		{
			return 3;
		};

		inter.m_getPosition = [](const SMikkTSpaceContext * pContext, float fvPosOut[], const int iFace, const int iVert)
		{
			auto instance = reinterpret_cast<DataWrapper*>(pContext->m_pUserData);

			auto vData = instance->vertexData + instance->indices[(iFace * 3) + iVert];
			memcpy(fvPosOut, vData->pos, sizeof(float) * 3);
		};

		inter.m_getNormal = [](const SMikkTSpaceContext * pContext, float fvNormOut[], const int iFace, const int iVert)
		{
			auto instance = reinterpret_cast<DataWrapper*>(pContext->m_pUserData);

			auto vData = instance->vertexData + instance->indices[(iFace * 3) + iVert];
			Mesh::unpack(vData->normal, fvNormOut, 3);
		};

		inter.m_getTexCoord = [](const SMikkTSpaceContext * pContext, float fvTexcOut[], const int iFace, const int iVert)
		{
			auto instance = reinterpret_cast<DataWrapper*>(pContext->m_pUserData);

			auto vData = instance->vertexData + instance->indices[(iFace * 3) + iVert];
			memcpy(fvTexcOut, vData->uv, sizeof(float) * 2);
		};

		inter.m_setTSpace = nullptr;
		inter.m_setTSpaceBasic = [](const SMikkTSpaceContext * pContext, const float fvTangent[], const float fSign, const int iFace, const int iVert)
		{
			auto instance = reinterpret_cast<DataWrapper*>(pContext->m_pUserData);

			auto vData = instance->vertexData + instance->indices[(iFace * 3) + iVert];
			Mesh::pack(fvTangent, vData->tangent, 3);
			vData->tangent[3] = Mesh::pack(fSign);
		};

		DataWrapper dataWrapper{ mNumIndices, mData.get(), mIndices.get() };

		SMikkTSpaceContext ctx;
		ctx.m_pInterface = &inter;
		ctx.m_pUserData = &dataWrapper;
		genTangSpaceDefault(&ctx);
	}
}
