#pragma once

#include "mesh.hpp"
#include "Math.hpp"
#include "Vector.hpp"

#include "libs\forsyth\forsythtriangleorderoptimizer.h"

#include <vector>

namespace HorseRadish { namespace Geometry {

static const float ushortScaleFrom = 1.0f / static_cast<float>(std::numeric_limits<short>::max());
static const float ushortScaleTo = static_cast<float>(std::numeric_limits<short>::max());

short Mesh::pack(const float value)
{
	return static_cast<unsigned short>(value * ushortScaleTo);
}

float Mesh::unpack(const short value)
{
	return (static_cast<float>(value) * ushortScaleFrom);
}

void Mesh::pack(const float* const in, short* const out, unsigned int numValues)
{
	for (unsigned int i = 0; i < numValues; i++)
		out[i] = static_cast<unsigned short>(in[i] * ushortScaleTo);
}

void Mesh::unpack(const short* const in, float* const out, unsigned int numValues)
{
	for (unsigned int i = 0; i < numValues; i++)
		out[i] = static_cast<float>(in[i])* ushortScaleFrom;
}

Mesh Mesh::genBox(const float width, const float height, const float depth, const int precision)
{
	if (width <= 0.0f || height <= 0.0f || depth <= 0.0f || precision <= 0)
		return Mesh();

	Mesh mesh((2 + precision - 1)*(precision + 1) * 6, precision*precision * 6 * 6);

	for (int i = 0, index = 0, jump = 0, jump2 = 0; i < mesh.mNumVertices; i += 6)
	{
		mesh.mIndices[i + 0] = index;
		mesh.mIndices[i + 1] = index + 1;
		mesh.mIndices[i + 2] = index + precision + 1;

		mesh.mIndices[i + 3] = index + 1;
		mesh.mIndices[i + 4] = index + precision + 2;
		mesh.mIndices[i + 5] = index + precision + 1;

		index++;
		jump++;
		if (jump == precision)
		{
			index++;
			jump = 0;
			jump2++;
			if (jump2 == precision)
			{
				index += precision + 1;
				jump2 = 0;
			}
		}
	}

	float realWidth = width * 0.5f;
	float realHeight = height * 0.5f;
	float realDepth = depth * 0.5f;
	float offsetW = width / static_cast<float>(precision);
	float offsetH = height / static_cast<float>(precision);
	float offsetD = depth / static_cast<float>(precision);
	float offsetTex = 1.0f / static_cast<float>(precision);
	auto curVertex = 0;

	for (int i = 0; i < (precision + 1); i++)
	{
		for (int j = 0; j < (2 + precision - 1); j++, curVertex++)
		{
			mesh.mData[curVertex].pos[0] = offsetW*((float)j) - realWidth;
			mesh.mData[curVertex].pos[1] = offsetH*((float)i) - realHeight;
			mesh.mData[curVertex].pos[2] = realDepth;

			mesh.mData[curVertex].uv[0] = offsetTex*((float)j);
			mesh.mData[curVertex].uv[1] = offsetTex*((float)i);

			mesh.mData[curVertex].normal[0] = Mesh::pack(0.0f);
			mesh.mData[curVertex].normal[1] = Mesh::pack(0.0f);
			mesh.mData[curVertex].normal[2] = Mesh::pack(1.0f);
		}
	}

	for (int i = 0; i < (precision + 1); i++)
	{
		for (int j = 0; j < (2 + precision - 1); j++, curVertex++)
		{
			mesh.mData[curVertex].pos[0] = realWidth;
			mesh.mData[curVertex].pos[1] = offsetH*((float)i) - realHeight;
			mesh.mData[curVertex].pos[2] = realDepth - offsetD*((float)j);

			mesh.mData[curVertex].uv[0] = offsetTex*((float)j);
			mesh.mData[curVertex].uv[1] = offsetTex*((float)i);

			mesh.mData[curVertex].normal[0] = Mesh::pack(1.0f);
			mesh.mData[curVertex].normal[1] = Mesh::pack(0.0f);
			mesh.mData[curVertex].normal[2] = Mesh::pack(0.0f);
		}
	}

	for (int i = 0; i < (precision + 1); i++)
	{
		for (int j = 0; j < (2 + precision - 1); j++, curVertex++)
		{
			mesh.mData[curVertex].pos[0] = realWidth - offsetW*((float)j);
			mesh.mData[curVertex].pos[1] = offsetH*((float)i) - realHeight;
			mesh.mData[curVertex].pos[2] = -realDepth;

			mesh.mData[curVertex].uv[0] = offsetTex*((float)j);
			mesh.mData[curVertex].uv[1] = offsetTex*((float)i);

			mesh.mData[curVertex].normal[0] = Mesh::pack(0.0f);
			mesh.mData[curVertex].normal[1] = Mesh::pack(0.0f);
			mesh.mData[curVertex].normal[2] = Mesh::pack(-1.0f);
		}
	}

	for (int i = 0; i < (precision + 1); i++)
	{
		for (int j = 0; j < (2 + precision - 1); j++, curVertex++)
		{
			mesh.mData[curVertex].pos[0] = -realWidth;
			mesh.mData[curVertex].pos[1] = offsetH*((float)i) - realHeight;
			mesh.mData[curVertex].pos[2] = offsetD*((float)j) - realDepth;

			mesh.mData[curVertex].uv[0] = offsetTex*((float)j);
			mesh.mData[curVertex].uv[1] = offsetTex*((float)i);

			mesh.mData[curVertex].normal[0] = Mesh::pack(-1.0f);
			mesh.mData[curVertex].normal[1] = Mesh::pack(0.0f);
			mesh.mData[curVertex].normal[2] = Mesh::pack(0.0f);
		}
	}

	for (int i = 0; i < (precision + 1); i++)
	{
		for (int j = 0; j < (2 + precision - 1); j++, curVertex++)
		{
			mesh.mData[curVertex].pos[0] = offsetW*((float)j) - realWidth;
			mesh.mData[curVertex].pos[1] = realHeight;
			mesh.mData[curVertex].pos[2] = realDepth - offsetD*((float)i);

			mesh.mData[curVertex].uv[0] = offsetTex*((float)j);
			mesh.mData[curVertex].uv[1] = offsetTex*((float)i);

			mesh.mData[curVertex].normal[0] = Mesh::pack(0.0f);
			mesh.mData[curVertex].normal[1] = Mesh::pack(1.0f);
			mesh.mData[curVertex].normal[2] = Mesh::pack(0.0f);
		}
	}

	for (int i = 0; i < (precision + 1); i++)
	{
		for (int j = 0; j < (2 + precision - 1); j++, curVertex++)
		{
			mesh.mData[curVertex].pos[0] = offsetW*((float)j) - realWidth;
			mesh.mData[curVertex].pos[1] = -realHeight;
			mesh.mData[curVertex].pos[2] = offsetD*((float)i) - realDepth;

			mesh.mData[curVertex].uv[0] = offsetTex*((float)j);
			mesh.mData[curVertex].uv[1] = offsetTex*((float)i);

			mesh.mData[curVertex].normal[0] = Mesh::pack(0.0f);
			mesh.mData[curVertex].normal[1] = Mesh::pack(-1.0f);
			mesh.mData[curVertex].normal[2] = Mesh::pack(0.0f);
		}
	}

	return mesh;
}

Mesh Mesh::genSphere(const float radius, const int slices, const int stacks)
{
	if (radius <= 0.0f || slices <= 2 || stacks <= 2)
		return Mesh();

	Mesh mesh((stacks - 1)*slices + 2, (stacks - 2)*slices * 2 * 3 + slices * 2 * 3);

	float ds = 1.0f / static_cast<float>(slices);
	float dt = 1.0f / static_cast<float>(stacks);
	float t = 1.0f;
	float drho = Math::PI / static_cast<float>(stacks);
	float dtheta = Math::TWO_PI / static_cast<float>(slices - 1);

	auto pIndices = mesh.mIndices.get();

	for (int i = 0, index = 0; i <= stacks; i++, t -= dt)
	{
		float rho = static_cast<float>(i)* drho;
		float s = 0.0f;

		if (i == 0)
		{
			mesh.mData[index].pos[0] = 0.0f;
			mesh.mData[index].pos[1] = 1.0f;
			mesh.mData[index].pos[2] = 0.0f;

			mesh.mData[index].uv[0] = 0.5f;
			mesh.mData[index].uv[1] = 1.0f;

			mesh.mData[index].normal[0] = Mesh::pack(0.0f);
			mesh.mData[index].normal[1] = Mesh::pack(radius);
			mesh.mData[index].normal[2] = Mesh::pack(0.0f);

			index++;
			continue;
		}

		if (i == stacks)
		{
			mesh.mData[index].pos[0] = 0.0f;
			mesh.mData[index].pos[1] = -1.0f;
			mesh.mData[index].pos[2] = 0.0f;

			mesh.mData[index].uv[0] = 0.5f;
			mesh.mData[index].uv[1] = 0.0f;

			mesh.mData[index].normal[0] = Mesh::pack(0.0f);
			mesh.mData[index].normal[1] = Mesh::pack(-radius);
			mesh.mData[index].normal[2] = Mesh::pack(0.0f);

			index++;
			continue;
		}

		float sinRho, cosRho;
		Math::sinCosR(rho, sinRho, cosRho);

		for (int j = 0; j < slices; j++, s += ds)
		{
			float theta = static_cast<float>(j)* dtheta;

			HorseRadish::Vector calc;
			Math::sinCosR(theta, calc.x, calc.z);
			calc.x *= -sinRho;
			calc.z *= sinRho;
			calc.y = cosRho;

			mesh.mData[index].pos[0] = calc.x;
			mesh.mData[index].pos[1] = calc.y;
			mesh.mData[index].pos[2] = calc.z;

			mesh.mData[index].uv[0] = s;
			mesh.mData[index].uv[1] = t;

			mesh.mData[index].normal[0] = Mesh::pack(calc.x * radius);
			mesh.mData[index].normal[1] = Mesh::pack(calc.y * radius);
			mesh.mData[index].normal[2] = Mesh::pack(calc.z * radius);


			index++;
		}
	}

	int index = 1;
	for (int j = 0; j < (slices - 1); j++, index++)
	{
		pIndices[0] = 0;
		pIndices[1] = index + 1;
		pIndices[2] = index;
		pIndices += 3;
	}
	pIndices[0] = 0;
	pIndices[1] = 1;
	pIndices[2] = index;
	pIndices += 3;

	index = 1;
	for (int i = 0; i < (stacks - 2); i++)
	{
		for (int j = 0; j < (slices - 1); j++, index++)
		{
			pIndices[0] = index;
			pIndices[1] = index + slices + 1;
			pIndices[2] = index + slices;
			pIndices += 3;

			pIndices[0] = index;
			pIndices[1] = index + 1;
			pIndices[2] = index + slices + 1;
			pIndices += 3;
		}

		pIndices[0] = index;
		pIndices[1] = (i*slices + 1) + slices;
		pIndices[2] = index + slices;
		pIndices += 3;

		pIndices[0] = index;
		pIndices[1] = (i*slices + 1);
		pIndices[2] = (i*slices + 1) + slices;
		pIndices += 3;

		index++;
	}

	int last = (stacks - 1) * slices + 1;
	for (int j = 0; j < (slices - 1); j++, index++)
	{
		pIndices[0] = index;
		pIndices[1] = index + 1;
		pIndices[2] = last;
		pIndices += 3;
	}
	pIndices[0] = index;
	pIndices[1] = (stacks - 2) * slices + 1;
	pIndices[2] = last;

	return mesh;
}

Mesh::Mesh()
	: mNumVertices(0), mNumIndices(0)
{ }

Mesh::Mesh(unsigned int numVertices, unsigned int numIndices)
	: mNumVertices(numVertices), mNumIndices(numIndices)
{
	assert((numVertices > 0) && (numIndices > 0));
	assert(numVertices < (static_cast<unsigned int>(std::numeric_limits<unsigned short>::max()) * 2));
	assert((numIndices % 3) == 0);

	mData = std::unique_ptr<VertexData[]>(new VertexData[numVertices]);
	mIndices = std::unique_ptr<unsigned short[]>(new unsigned short[numIndices]);
}

Mesh::Mesh(std::unique_ptr<VertexData[]> vertices, unsigned int numVertices, std::unique_ptr<unsigned short[]> indices, unsigned int numIndices)
	: mData(std::move(vertices)), mNumVertices(numVertices), mIndices(std::move(indices)), mNumIndices(numIndices)
{
	assert((numVertices > 0) && (numIndices > 0));
	assert(numVertices < (static_cast<unsigned int>(std::numeric_limits<unsigned short>::max()) * 2));
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

unsigned int Mesh::sizeVertices() const
{
	return (sizeof(VertexData) * mNumVertices);
}

unsigned int Mesh::sizeIndices() const
{
	return (sizeof(unsigned short) * mNumIndices);
}

unsigned int Mesh::numIndices() const
{
	return mNumIndices;
}

unsigned int Mesh::numVertices() const
{
	return mNumVertices;
}

unsigned int Mesh::numTris() const
{
	return mNumIndices / 3;
}

bool Mesh::check() const
{
	if (!mData || !mIndices || (mNumVertices <= 0) && (mNumIndices <= 0))
		return false;

	if ((mNumIndices % 3) != 0)
		return false;

	for (unsigned int i = 0; i < mNumIndices; i++)
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
		auto vertexPtr = static_cast<VertexData*>(mData.get());

		minPoint = maxPoint = _mm_loadu_ps(vertexPtr->pos);
		for (unsigned int i = 1; i < mNumVertices; i++, vertexPtr++)
		{
			curPoint = _mm_loadu_ps(vertexPtr->pos);
			minPoint = _mm_min_ps(minPoint, curPoint);
			maxPoint = _mm_max_ps(maxPoint, curPoint);
		}
	}

	Vector tmpVecs[2];
	_mm_storeu_ps(tmpVecs[0], minPoint);
	_mm_storeu_ps(tmpVecs[1], maxPoint);

	return BBox(tmpVecs, 2);
}

float Mesh::getIndicesCacheRatio(unsigned int cacheSize) const
{
	if (cacheSize == 0)
		return 0.0f;
	if (cacheSize >= mNumIndices)
		return 1.0f;

	std::vector<int> simCache(cacheSize, -1);

	unsigned int numHits = 0;
	for (unsigned int i = 0; i < mNumIndices; i++)
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

		for (unsigned int j = (cacheSize - 1); j > 0; j--)
			simCache[j] = simCache[j - 1];;
		simCache[0] = curIndex;
	}

	return (static_cast<float>(numHits) / static_cast<float>(mNumIndices));
}

bool Mesh::getRayIntersect(const Vector& rayOrigin, const Vector& rayDir, float& hitDistance) const
{
	hitDistance = 0.0f;

	float minHistDist = HorseRadish::Math::INFINITY;
	auto hit = false;

	for (unsigned int i = 0; i < mNumIndices; i += 3)
	{
		Vector p1(mData[i * 3 + 0].pos);
		Vector p2(mData[i * 3 + 1].pos);
		Vector p3(mData[i * 3 + 2].pos);

		Vector normal;
		normal.StoreNormal(p1, p2, p3);
		normal.Normalize();

		if (normal.GetDot(rayDir) > 0.0f) //normal isn't facing the ray (we're simulating culling faces)
			continue;

		Vector hitPoint;
		{
			HorseRadish::Vector edge1, edge2, tvec, pvec, qvec;
			float det, u, v;

			//find vectors for two edges sharing vert0
			edge1 = p2 - p1;
			edge2 = p3 - p1;

			//begin calculating determinant - also used to calculate U parameter
			pvec.StoreCrossProduct(rayDir, (const float*)edge2);

			//check if ray is in the same plane as the tri
			det = edge1.GetDot(pvec);
			if (HorseRadish::Math::isZero(det))
				continue;

			//calculate distance from vert0 to ray origin
			tvec = rayOrigin - p1;
			qvec.StoreCrossProduct(tvec, edge1);

			u = tvec.GetDot(pvec);
			if (det > 0.0f)
			{
				if (u<0.0 || u>det)
					continue;

				v = qvec.GetDot(rayDir);
				if ((v<0.0) || ((u + v)>det))
					continue;
			}
			else
			{
				if (u > 0.0 || u<det)
					continue;

				v = qvec.GetDot(rayDir);
				if ((v>0.0) || ((u + v) < det))
					continue;
			}

			det = 1.0f / det;
			u *= det;
			v *= det;

			hitPoint = (p1 * (1.0f - u - v)) + (p2 * u) + (p3 * v);
		}

		hit = true;
		minHistDist = HorseRadish::Math::fMin(minHistDist, hitPoint.GetDistance(rayOrigin));
	}

	if (!hit)
		return false;

	hitDistance = minHistDist;
	return true;
}

void Mesh::flipUV()
{
	auto vertexPtr = static_cast<VertexData*>(mData.get());
	for (unsigned int i = 0; i < mNumVertices; i++, vertexPtr++)
		vertexPtr->uv[1] = 1.0f - vertexPtr->uv[1];
}

void Mesh::scale(float scaleAmount)
{
	auto vertexPtr = static_cast<VertexData*>(mData.get());
	for (unsigned int i = 0; i < mNumVertices; i++, vertexPtr++)
	{
		vertexPtr->pos[0] *= scaleAmount;
		vertexPtr->pos[1] *= scaleAmount;
		vertexPtr->pos[2] *= scaleAmount;
	}
}

void Mesh::translate(const Vector& translate)
{
	auto vertexPtr = static_cast<VertexData*>(mData.get());
	for (unsigned int i = 0; i < mNumVertices; i++, vertexPtr++)
	{
		vertexPtr->pos[0] += translate.x;
		vertexPtr->pos[1] += translate.y;
		vertexPtr->pos[2] += translate.z;
	}
}

void Mesh::centerMass(const Vector& center)
{
	auto bbox = getBoundingBox();

	Vector minP, maxP;
	bbox.GetMin(minP);
	bbox.GetMax(maxP);
	auto distance = maxP - minP;

	distance.x = HorseRadish::Math::fAbs(distance.x)*0.5f;
	distance.y = HorseRadish::Math::fAbs(distance.y)*0.5f;
	distance.z = HorseRadish::Math::fAbs(distance.z)*0.5f;

	distance = center - (minP + distance);

	auto vertexPtr = static_cast<VertexData*>(mData.get());
	for (unsigned int i = 0; i < mNumVertices; i++, vertexPtr++)
	{
		vertexPtr->pos[0] += distance.x;
		vertexPtr->pos[1] += distance.y;
		vertexPtr->pos[2] += distance.z;
	}
}

void Mesh::confine(float maxAxis)
{
	auto bbox = getBoundingBox();

	Vector minP, maxP;
	bbox.GetMin(minP);
	bbox.GetMax(maxP);
	auto distance = maxP - minP;

	distance.x = HorseRadish::Math::fAbs(distance.x);
	distance.y = HorseRadish::Math::fAbs(distance.y);
	distance.z = HorseRadish::Math::fAbs(distance.z);

	auto distanceMax = HorseRadish::Math::fMax(distance.x, distance.y);
	distanceMax = HorseRadish::Math::fMax(distanceMax, distance.z);

	auto scale = maxAxis / distanceMax;

	auto vertexPtr = static_cast<VertexData*>(mData.get());
	for (unsigned int i = 0; i < mNumVertices; i++, vertexPtr++)
	{
		vertexPtr->pos[0] *= scale;
		vertexPtr->pos[1] *= scale;
		vertexPtr->pos[2] *= scale;
	}
}

void Mesh::confine(const Vector& center, float maxAxis)
{
	auto bbox = getBoundingBox();

	Vector minP, maxP;
	bbox.GetMin(minP);
	bbox.GetMax(maxP);
	auto distance = maxP - minP;

	distance.x = HorseRadish::Math::fAbs(distance.x);
	distance.y = HorseRadish::Math::fAbs(distance.y);
	distance.z = HorseRadish::Math::fAbs(distance.z);

	auto distanceMax = HorseRadish::Math::fMax(distance.x, distance.y);
	distanceMax = HorseRadish::Math::fMax(distanceMax, distance.z);

	auto scale = maxAxis / distanceMax;

	distance *= 0.5f;
	distance = center - (minP + distance);

	auto vertexPtr = static_cast<VertexData*>(mData.get());
	for (unsigned int i = 0; i < mNumVertices; i++, vertexPtr++)
	{
		vertexPtr->pos[0] = (vertexPtr->pos[0] + distance.x) * scale;
		vertexPtr->pos[1] = (vertexPtr->pos[1] + distance.y) * scale;
		vertexPtr->pos[2] = (vertexPtr->pos[2] + distance.z) * scale;
	}
}

void Mesh::invertTriWinding()
{
	for (unsigned int i = 0; i < mNumIndices; i += 3)
		std::swap(mIndices[0], mIndices[2]);
}

void Mesh::optimizeIndices()
{
	auto newIndices = std::unique_ptr<unsigned short[]>(new unsigned short[mNumIndices]);
	Forsyth::OptimizeFaces(mIndices.get(), mNumIndices, mNumVertices, newIndices.get(), 32);

	std::swap(mIndices, newIndices);
}

void Mesh::genNormals()
{
	auto normals = std::unique_ptr<Vector[]>(new Vector[mNumVertices]);

	for (unsigned int i = 0; i < mNumVertices; i++)
		normals[i].Set(0.0f);

	for (unsigned int i = 0; i < mNumIndices; i += 3)
	{
		Vector faceNormal;
		faceNormal.StoreNormal(mData[i * 3 + 0].pos, mData[i * 3 + 1].pos, mData[i * 3 + 2].pos);

		normals[i * 3 + 0] += faceNormal;
		normals[i * 3 + 1] += faceNormal;
		normals[i * 3 + 2] += faceNormal;
	}

	for (unsigned int i = 0; i < mNumVertices; i++)
	{
		normals[i].Normalize();

		Mesh::pack(normals[i], mData[i].normal, 3);
	}
}

void Mesh::genTangents4()
{
	auto tan1 = std::unique_ptr<Vector[]>(new Vector[mNumVertices]);
	auto tan2 = std::unique_ptr<Vector[]>(new Vector[mNumVertices]);

	for (unsigned int i = 0; i < mNumVertices; i++)
	{
		tan1[i].Set(0.0f);
		tan2[i].Set(0.0f);
	}

	for (unsigned int i = 0; i < mNumIndices; i += 3)
	{
		auto i1 = mIndices[i + 0];
		auto i2 = mIndices[i + 1];
		auto i3 = mIndices[i + 2];

		auto& v1 = mData[i1];
		auto& v2 = mData[i2];
		auto& v3 = mData[i3];

		float x1 = v2.pos[0] - v1.pos[0];
		float x2 = v3.pos[0] - v1.pos[0];
		float y1 = v2.pos[1] - v1.pos[1];
		float y2 = v3.pos[1] - v1.pos[1];
		float z1 = v2.pos[2] - v1.pos[2];
		float z2 = v3.pos[2] - v1.pos[2];

		float s1 = v2.uv[0] - v1.uv[0];
		float s2 = v3.uv[0] - v1.uv[0];
		float t1 = v2.uv[1] - v1.uv[1];
		float t2 = v3.uv[1] - v1.uv[1];

		float r = 1.0f / (s1 * t2 - s2 * t1);
		Vector sdir((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r, (t2 * z1 - t1 * z2) * r);
		Vector tdir((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r, (s1 * z2 - s2 * z1) * r);

		tan1[i1] += sdir;
		tan1[i2] += sdir;
		tan1[i3] += sdir;

		tan2[i1] += tdir;
		tan2[i2] += tdir;
		tan2[i3] += tdir;
	}

	for (unsigned int i = 0; i < mNumVertices; i++)
	{
		Vector n(Mesh::unpack(mData[i].normal[0]), Mesh::unpack(mData[i].normal[1]), Mesh::unpack(mData[i].normal[2]));
		const Vector& t1 = tan1[i];
		const Vector& t2 = tan2[i];

		//Gram-Schmidt orthogonalize
		Vector tangent = t1 - n * n.GetDot(t1);
		tangent.Normalize();

		//handedness
		tangent.w = (n.CrossProduct(t1).GetDot(t2) < 0.0f) ? -1.0f : 1.0f;

		Mesh::pack(tangent, mData[i].tangent, 4);
	}
}

} }
