#pragma once

#include "mesh.hpp"

#include "math.hpp"
#include "triangle.hpp"
#include "types.hpp"
#include "vector.hpp"

#include <meshoptimizer.h>
#include <mikktspace.h>

#include <unordered_map>
#include <vector>

namespace hr::geom
{
    namespace
    {
        struct PackedUVec
        {
            int x : 10;
            int y : 10;
            int z : 10;
            int w : 2;
        }; // corresponds to GL_INT_2_10_10_10_REV
        static_assert(sizeof(PackedUVec) == 4);

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

        uint32_t packedWriteNormal(std::span<const float, 3> normal)
        {
            const uint32_t xs = normal[0] < 0;
            const uint32_t ys = normal[1] < 0;
            const uint32_t zs = normal[2] < 0;
            return (0 | zs << 29 | ((uint32_t)(normal[2] * 511.0f + (zs << 9)) & 511) << 20 | ys << 19 | ((uint32_t)(normal[1] * 511.0f + (ys << 9)) & 511) << 10 | xs << 9 |
                    ((uint32_t)(normal[0] * 511.0f + (xs << 9)) & 511));
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

        uint32_t packedWriteTangent(std::span<const float, 4> tangent)
        {
            const uint32_t xs = tangent[0] < 0;
            const uint32_t ys = tangent[1] < 0;
            const uint32_t zs = tangent[2] < 0;
            const uint32_t ws = tangent[3] < 0;
            return (ws << 31 | ((uint32_t)(tangent[3] + (ws << 1)) & 1) << 30 | zs << 29 | ((uint32_t)(tangent[2] * 511.0f + (zs << 9)) & 511) << 20 | ys << 19 |
                    ((uint32_t)(tangent[1] * 511.0f + (ys << 9)) & 511) << 10 | xs << 9 | ((uint32_t)(tangent[0] * 511.0f + (xs << 9)) & 511));
        }
    }

    void VertexFull::convertTo(VertexShading& dest) const
    {
        std::memcpy(dest.pos, pos, sizeof(float) * 3);
        types::packFloat(uv, dest.uv, 2);
        dest.normal = packedWriteNormal(normal);
        dest.tangent = packedWriteTangent(tangent);
    }

    Vector3f VertexShading::getPos() const noexcept
    {
        return Vector3f{pos};
    }

    Vector3f VertexShading::getNormal() const noexcept
    {
        return packedReadNormal(normal);
    }

    Vector4f VertexShading::getTangent() const noexcept
    {
        return packedReadTangent(tangent);
    }

    void VertexShading::setPos(const Vector3f& newPos) noexcept
    {
        newPos.write(pos);
    }

    void VertexShading::setUV(float u, float v) noexcept
    {
        uv[0] = types::packFloat<uint16_t>(u);
        uv[1] = types::packFloat<uint16_t>(v);
    }

    void VertexShading::setNormal(const Vector3f& newNormal) noexcept
    {
        normal = packedWriteNormal(newNormal.data().first<3>());
    }

    void VertexShading::setTangent(const Vector4f& newTangent) noexcept
    {
        tangent = packedWriteTangent(newTangent.data().first<4>());
    }

    void VertexShading::convertTo(VertexFull& dest)
    {
        std::memcpy(dest.pos, pos, sizeof(float) * 3);
        types::unpackFloat(uv, dest.uv, 2);
        packedReadNormal(normal).write(dest.normal);
        packedReadTangent(tangent).write(dest.tangent);
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

            vecMin = Vector3f{minPoint};
            vecMax = Vector3f{maxPoint};
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
                simCache[j] = simCache[j - 1];
            ;
            simCache[0] = curIndex;
        }

        return (static_cast<float>(numHits) / static_cast<float>(mNumIndices));
    }

    bool Mesh<VertexFull, uint32_t>::intersects(const Ray<Vector3d>& ray, double rayDistMin, double rayDistMax, Hit& hit) const noexcept
    {
        auto wasHit{false};

        for (size_t i = 0; i < mNumIndices; i += 3)
        {
            Triangle<Vector3d> tri{Vector3f{mData[mIndices[i + 0]].pos}.convert<double, 3>(), Vector3f{mData[mIndices[i + 1]].pos}.convert<double, 3>(),
                                   Vector3f{mData[mIndices[i + 2]].pos}.convert<double, 3>()};

            Triangle<Vector3d>::Hit triHit;
            if (!tri.intersects(ray, rayDistMin, rayDistMax, triHit))
                continue;

            rayDistMax = triHit.rayT;

            hit.rayT = triHit.rayT;
            hit.triIndex = i / 3;
            hit.barycentricU = static_cast<float>(triHit.barycentricU);
            hit.barycentricV = static_cast<float>(triHit.barycentricV);
            wasHit = true;
        }

        return wasHit;
    }

    bool Mesh<VertexFull, uint32_t>::intersects(size_t triIndex, const Ray<Vector3d>& ray, double rayDistMin, double rayDistMax, Hit& hit) const noexcept
    {
        if ((triIndex * 3) >= mNumIndices)
            return false;

        Triangle<Vector3d> tri{Vector3f{mData[mIndices[(triIndex * 3) + 0]].pos}.convert<double, 3>(), Vector3f{mData[mIndices[(triIndex * 3) + 1]].pos}.convert<double, 3>(),
                               Vector3f{mData[mIndices[(triIndex * 3) + 2]].pos}.convert<double, 3>()};

        Triangle<Vector3d>::Hit triHit;
        if (!tri.intersects(ray, rayDistMin, rayDistMax, triHit))
            return false;

        hit.rayT = triHit.rayT;
        hit.triIndex = triIndex;
        hit.barycentricU = static_cast<float>(triHit.barycentricU);
        hit.barycentricV = static_cast<float>(triHit.barycentricV);
        return true;
    }

    Vector3f Mesh<VertexFull, uint32_t>::triNormal(size_t triIndex, float baryU, float baryV) const noexcept
    {
        if ((triIndex * 3) >= mNumIndices)
            return {};

        auto baryW = 1.0f - baryU - baryV;

        Vector3f n1(mData[mIndices[(triIndex * 3) + 0]].normal);
        Vector3f n2(mData[mIndices[(triIndex * 3) + 1]].normal);
        Vector3f n3(mData[mIndices[(triIndex * 3) + 2]].normal);

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

    void Mesh<VertexFull, uint32_t>::transform(const Matrix4f& matFull, const Matrix3f& matRot) noexcept
    {
        auto vertexData = mData.get();
        for (size_t i = 0; i < mNumVertices; i++, vertexData++)
        {
            matFull.transform(vertexData->pos);

            matRot.transform(vertexData->normal);
            matRot.transform(std::span<float, 3>{vertexData->tangent, 3});
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

        // this optimizes for the vertex cache (reads from mIndices and writes to newIndices)
        meshopt_optimizeVertexCache(newIndices.get(), mIndices.get(), mNumIndices, mNumVertices);

        // this optimizes for overdraw (reads from newIndices and writes to mIndices)
        meshopt_optimizeOverdraw(mIndices.get(), newIndices.get(), mNumIndices, mData.get()[0].pos, mNumVertices, sizeof(VertexFull), 1.05f);

        // we changed the indices (already written to mIndices), so now, we can change the position of vertex data to be more memory friendly using the new indices
        meshopt_optimizeVertexFetch(mData.get(), mIndices.get(), mNumIndices, mData.get(), mNumVertices, sizeof(VertexFull));
    }

    bool Mesh<VertexFull, uint32_t>::genUVs(UVGenType genType) noexcept
    {
        // based on https://www.khronos.org/opengl/wiki/Mathematics_of_glTexGen

        if (genType == UVGenType::Sphere)
        {
            for (size_t i = 0; i < mNumVertices; i++)
            {
                auto& vertex = mData[i];

                auto eyeVec = Vector3f::calcNormalize(vertex.getPos());
                auto eyeNormal = vertex.getNormal();

                auto reflectionVec = eyeVec - (eyeNormal * 2.0 * eyeVec.dot(eyeNormal));
                reflectionVec[2] += 1.0f;

                auto m = 1.0f / (2.0f * reflectionVec.magnitude());

                vertex.uv[0] = (reflectionVec[0] * m) + 0.5f;
                vertex.uv[1] = (reflectionVec[1] * m) + 0.5f;
            }

            return true;
        }

        if (genType == UVGenType::Reflection)
        {
            for (size_t i = 0; i < mNumVertices; i++)
            {
                auto& vertex = mData[i];

                auto eyeVec = Vector3f::calcNormalize(vertex.getPos());
                auto eyeNormal = vertex.getNormal();

                auto dotResult = 2.0f * eyeVec.dot(eyeNormal);

                vertex.uv[0] = eyeVec[0] - (eyeNormal[0] * dotResult);
                vertex.uv[1] = eyeVec[1] - (eyeNormal[2] * dotResult);
                // missing the R (z) componente to sample from the cubemap
            }

            return true;
        }

        return false;
    }

    void Mesh<VertexFull, uint32_t>::genNormals() noexcept
    {
        auto normals = std::unique_ptr<Vector3f[]>(new Vector3f[mNumVertices]);

        for (size_t i = 0; i < mNumVertices; i++)
            normals[i] = Vector3f::zero();

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

        DataWrapper dataWrapper{mNumIndices, mData.get(), mIndices.get()};

        SMikkTSpaceContext ctx;
        ctx.m_pInterface = &inter;
        ctx.m_pUserData = &dataWrapper;
        genTangSpaceDefault(&ctx);
    }

    std::vector<Mesh<VertexFull, uint32_t>> Mesh<VertexFull, uint32_t>::split(size_t maxVertexCount) const
    {
        if (mNumVertices <= maxVertexCount)
            return {};

        std::vector<Mesh<VertexFull, uint32_t>> newMeshes;

        auto addMesh = [this, maxVertexCount, &newMeshes](const std::unordered_map<size_t, size_t>& vertexMapping, const std::vector<uint32_t>& newIndices)
        {
            Mesh<VertexFull, uint32_t> newMesh{vertexMapping.size(), newIndices.size()};

            for (auto [oldIndex, newIndex] : vertexMapping)
                newMesh.vertex(newIndex) = mData[oldIndex];
            std::copy(newIndices.begin(), newIndices.end(), newMesh.indices().data());

            assert(newMesh.check(maxVertexCount));
            newMesh.optimizeIndices();

            newMeshes.push_back(std::move(newMesh));
        };

        size_t newVertexIndex{0};
        std::vector<uint32_t> newIndices;
        std::unordered_map<size_t, size_t> vertexMapping;
        for (size_t curIndex = 0; curIndex < mNumIndices; curIndex += 3)
        {
            // process triangle
            for (size_t i = 0; i < 3; i++)
            {
                auto oldVertexIndex = mIndices[curIndex + i];
                if (auto it = vertexMapping.find(oldVertexIndex); it != vertexMapping.end())
                {
                    newIndices.push_back(it->second);
                    continue;
                }

                vertexMapping[oldVertexIndex] = newVertexIndex;
                newIndices.push_back(newVertexIndex);
                newVertexIndex++;
            }

            if ((vertexMapping.size() + 3) < maxVertexCount)
                continue;

            addMesh(vertexMapping, newIndices);

            newVertexIndex = 0;
            newIndices.clear();
            vertexMapping.clear();
        }

        // final remaining triangles
        if (!newIndices.empty() || vertexMapping.empty())
        {
            assert(!newIndices.empty());
            assert(!vertexMapping.empty());
            addMesh(vertexMapping, newIndices);
        }

        return newMeshes;
    }
}
