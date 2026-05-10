module;

#include <cassert>
#include <cstdint>

export module core:mesh;

import std;
import :vector;
import :matrix;
import :bvolumes;

export namespace hr::geom
{
#pragma pack(push, 1)
    struct VertexFull;
    struct VertexShading;

    struct VertexFull
    {
        float pos[3];
        float uv[2];
        float normal[3];
        float tangent[4];

        Vector3f getPos() const noexcept
        {
            return Vector3f{pos};
        }
        Vector3f getNormal() const noexcept
        {
            return Vector3f{normal};
        }
        Vector4f getTangent() const noexcept
        {
            return Vector4f{tangent};
        }

        void convertTo(VertexShading& dest) const;
    };

    static_assert(sizeof(VertexFull) == 48);
    static_assert(std::is_trivial_v<VertexFull>);

    struct VertexShading
    {
        float pos[3];
        uint16_t uv[2];   // 65535 pixels precision
        uint32_t normal;  // GL_INT_2_10_10_10_REV (alpha is unused)
        uint32_t tangent; // GL_INT_2_10_10_10_REV (alpha is either -1.0 or 1.0 to recover the sign when calculating TBN: cross(normal, tangent) * sign)

        Vector3f getPos() const noexcept;
        Vector3f getNormal() const noexcept;
        Vector4f getTangent() const noexcept;

        void setPos(const Vector3f& newPos) noexcept;
        void setUV(float u, float v) noexcept;
        void setNormal(const Vector3f& newNormal) noexcept;
        void setTangent(const Vector4f& newTangent) noexcept;

        void convertTo(VertexFull& dest);
    };

    static_assert(sizeof(VertexShading) == 24);
    static_assert(std::is_trivial_v<VertexShading>);
#pragma pack(pop)

    template<typename, typename>
    class Mesh;

    template<class TVertex, class TIndex>
    class MeshBase
    {
        static_assert(std::is_trivial_v<TVertex>, "Vertex layout type must be trivial");
        static_assert(std::is_same_v<uint16_t, TIndex> || std::is_same_v<uint32_t, TIndex>, "Indices must either be uint16_t or uint32_t");

        template<typename, typename>
        friend class Mesh;

    public:
        using VertexType = TVertex;
        using IndexType = TIndex;

        static constexpr size_t maxVertexCount()
        {
            return static_cast<size_t>(std::numeric_limits<TIndex>::max());
        }

        static constexpr size_t sizeVertices(size_t numVertices)
        {
            return (sizeof(TVertex) * numVertices);
        }

        static constexpr size_t sizeIndices(size_t numIndices)
        {
            return (sizeof(TIndex) * numIndices);
        }

    private:
        std::unique_ptr<TVertex[]> mData;
        std::unique_ptr<TIndex[]> mIndices;
        size_t mNumVertices{0}, mNumIndices{0};

    public:
        MeshBase() = default;

        MeshBase(size_t numVertices, size_t numIndices)
          : mNumVertices{numVertices}, mNumIndices{numIndices}
        {
            assert((numVertices > 0) && (numIndices > 0));
            assert(numVertices < MeshBase::maxVertexCount());
            assert((numIndices % 3) == 0);

            mData = std::unique_ptr<TVertex[]>(new TVertex[numVertices]);
            mIndices = std::unique_ptr<TIndex[]>(new TIndex[numIndices]);

            std::memset(mData.get(), 0, sizeof(TVertex) * numVertices);
            std::memset(mIndices.get(), 0, sizeof(TIndex) * numIndices);
        }

        MeshBase(std::unique_ptr<TVertex[]> vertices, size_t numVertices, std::unique_ptr<TIndex[]> indices, size_t numIndices) noexcept
          : mData{std::move(vertices)}, mNumVertices{numVertices}, mIndices{std::move(indices)}, mNumIndices{numIndices}
        {
            assert((numVertices > 0) && (numIndices > 0));
            assert(numVertices < MeshBase::maxVertexCount());
            assert((numIndices % 3) == 0);

            assert(mData && mIndices);
        }

        MeshBase(std::span<const TVertex> vertices, std::span<const TIndex> indices)
          : mNumVertices{vertices.size()}, mNumIndices{indices.size()}
        {
            mData = std::unique_ptr<TVertex[]>(new TVertex[mNumVertices]);
            mIndices = std::unique_ptr<TIndex[]>(new TIndex[mNumIndices]);

            assert((sizeof(TVertex) * mNumVertices) == vertices.size_bytes());
            std::memcpy(mData.get(), vertices.get(), sizeof(TVertex) * mNumVertices);

            assert((sizeof(TIndex) * mNumIndices) == indices.size_bytes());
            std::memcpy(mIndices.get(), indices.get(), sizeof(TIndex) * mNumIndices);
        }

        MeshBase(const MeshBase& mesh)
          : mNumVertices{mesh.mNumVertices}, mNumIndices{mesh.mNumIndices}
        {
            mData = std::unique_ptr<TVertex[]>(new TVertex[mNumVertices]);
            mIndices = std::unique_ptr<TIndex[]>(new TIndex[mNumIndices]);

            std::memcpy(mData.get(), mesh.mData.get(), sizeof(TVertex) * mNumVertices);
            std::memcpy(mIndices.get(), mesh.mIndices.get(), sizeof(TIndex) * mNumIndices);
        }

        MeshBase& operator=(const MeshBase& mesh)
        {
            mNumVertices = mesh.mNumVertices;
            mNumIndices = mesh.mNumIndices;

            mData = std::unique_ptr<TVertex[]>(new TVertex[mNumVertices]);
            mIndices = std::unique_ptr<TIndex[]>(new TIndex[mNumIndices]);

            std::memcpy(mData.get(), mesh.mData.get(), sizeof(TVertex) * mNumVertices);
            std::memcpy(mIndices.get(), mesh.mIndices.get(), sizeof(TIndex) * mNumIndices);

            return *this;
        }

        MeshBase(MeshBase&& mesh) = default;
        MeshBase& operator=(MeshBase&& mesh) = default;

    public:
        template<typename Self>
        constexpr auto vertices(this Self&& self) noexcept
        {
            return std::span{self.mData.get(), self.mNumVertices};
        }

        template<typename Self>
        constexpr auto& vertex(this Self&& self, size_t index) noexcept
        {
            assert(index < self.mNumVertices);
            return self.mData[index];
        }

        template<typename Self>
        constexpr auto indices(this Self&& self) noexcept
        {
            return std::span{self.mIndices.get(), self.mNumIndices};
        }

        size_t sizeVertices() const noexcept
        {
            return MeshBase::sizeVertices(mNumVertices);
        }
        size_t sizeIndices() const noexcept
        {
            return MeshBase::sizeIndices(mNumIndices);
        }

        size_t numIndices() const noexcept
        {
            return mNumIndices;
        }
        size_t numVertices() const noexcept
        {
            return mNumVertices;
        }
        size_t numTris() const noexcept
        {
            return mNumIndices / 3;
        }

        template<typename Self, class TCallback>
        size_t iterateVertices(this Self&& self, TCallback&& cb) noexcept
        {
            static_assert(std::is_invocable_r_v<bool, TCallback, size_t, TVertex&>);

            size_t i = 0;
            for (; i < self.mNumVertices; i++)
            {
                if (!cb(i, self.mData[i]))
                    break;
            }

            return i;
        }

        template<typename Self, class TCallback>
        size_t iterateTris(this Self&& self, TCallback&& cb) noexcept
        {
            static_assert(std::is_invocable_r_v<bool, TCallback, size_t, TVertex&, TVertex&, TVertex&>);

            size_t count{0};
            for (size_t i = 0; i < self.mNumIndices; i += 3, count++)
            {
                if (!cb(count, self.mData[self.mIndices[i + 0]], self.mData[self.mIndices[i + 1]], self.mData[self.mIndices[i + 2]]))
                    break;
            }

            return count;
        }

        bool check(std::optional<size_t> maxNumVertices = std::nullopt) const noexcept
        {
            if (!mData || !mIndices || (mNumVertices <= 0) || (mNumVertices > MeshBase::maxVertexCount()) || (mNumIndices <= 0) || ((mNumIndices % 3) != 0))
                return false;

            auto maxVertices = !maxNumVertices ? mNumVertices : std::min(mNumVertices, *maxNumVertices);
            for (size_t i = 0; i < mNumIndices; i++)
            {
                if (mIndices[i] >= maxVertices)
                    return false;
            }

            return true;
        }

        BBox<> bbox() const noexcept
        {
            if (mNumVertices <= 0)
                return {};

            auto vecMin = mData[0].getPos();
            auto vecMax = vecMin;

            for (size_t i = 1; i < mNumVertices; i++)
            {
                auto curPos = mData[i].getPos();
                vecMin = Vector3f::calcMin(vecMin, curPos);
                vecMax = Vector3f::calcMax(vecMax, curPos);
            }

            return BBox(vecMin, vecMax);
        }

        template<typename TTargetVertex, class TTargetIndex>
        Mesh<TTargetVertex, TTargetIndex> convert() const noexcept
        {
            if (!check())
                return {};

            // if the amount of vertices doesn't fit
            if (numVertices() > Mesh<TTargetVertex, TTargetIndex>::maxVertexCount())
                return {};

            if constexpr (std::is_same_v<TTargetVertex, TVertex> && std::is_same_v<TTargetIndex, TIndex>)
            {
                // glorified clone
                return Mesh<TTargetVertex, TTargetIndex>{this};
            }
            else
            {
                Mesh<TTargetVertex, TTargetIndex> newMesh{numVertices(), numIndices()};
                if (!newMesh.check())
                    return {};

                // try to optimize vertex copy
                if constexpr (std::is_same_v<TTargetVertex, TVertex>)
                {
                    assert(sizeVertices() == newMesh.sizeVertices());
                    std::memcpy(newMesh.vertices().data(), mData, sizeVertices());
                }
                else
                {
                    for (size_t i = 0; i < mNumVertices; i++)
                        mData[i].convertTo(newMesh.vertex(i));
                }

                // try to optimize indices copy
                if constexpr (std::is_same_v<TTargetIndex, TIndex>)
                {
                    assert(sizeIndices() == newMesh.sizeIndices());
                    std::memcpy(newMesh.indices().data(), mIndices, sizeIndices());
                }
                else
                {
                    auto newIndices = newMesh.indices().data();
                    for (size_t i = 0; i < mNumIndices; i++)
                        newIndices[i] = static_cast<TTargetIndex>(mIndices[i]);
                }

                return newMesh;
            }
        }
    };

    template<class TVertex, class TIndex>
    class Mesh: public MeshBase<TVertex, TIndex>
    {
        using BaseType = MeshBase<TVertex, TIndex>;

    public:
        using BaseType::MeshBase;
    };

    template<>
    class Mesh<VertexFull, uint32_t>: public MeshBase<VertexFull, uint32_t>
    {
        using BaseType = MeshBase<VertexFull, uint32_t>;

    public:
        enum class UVGenType
        {
            Sphere,
            Reflection
        };

        struct Hit
        {
            double rayT;
            size_t triIndex;
            float barycentricU, barycentricV;
        };

    public:
        using BaseType::MeshBase;

        BBox<> bbox() const noexcept;
        float indicesCacheRatio(size_t cacheSize) const noexcept;

        bool intersects(const Ray<Vector3d>& ray, double rayDistMin, double rayDistMax, Hit& hit) const noexcept;
        bool intersects(const Ray<Vector3d>& ray, Hit& hit) const noexcept
        {
            constexpr auto tMin = -std::numeric_limits<double>::infinity();
            constexpr auto tMax = std::numeric_limits<double>::infinity();
            return intersects(ray, tMin, tMax, hit);
        }

        bool intersects(size_t triIndex, const Ray<Vector3d>& ray, double rayDistMin, double rayDistMax, Hit& hit) const noexcept;
        bool intersects(size_t triIndex, const Ray<Vector3d>& ray, Hit& hit) const noexcept
        {
            constexpr auto tMin = -std::numeric_limits<double>::infinity();
            constexpr auto tMax = std::numeric_limits<double>::infinity();
            return intersects(triIndex, ray, tMin, tMax, hit);
        }

        Vector3f triNormal(size_t triIndex, float baryU, float baryV) const noexcept;

        void flipUV() noexcept;
        void mirrorUV() noexcept;

        void scaleUV(float scaleAmount) noexcept;
        void scaleUV(float scaleU, float scaleV) noexcept;
        void scale(float scaleAmount) noexcept;

        void translate(const Vector3f& translate) noexcept;
        void centerMass(const Vector3f& center) noexcept;
        void confine(float maxAxis) noexcept;

        void transform(const Matrix4f& matFull, const Matrix3f& matRot) noexcept;

        void invertTriWinding() noexcept;

        void optimizeIndices() noexcept;

        bool genUVs(UVGenType genType) noexcept;
        void genNormals() noexcept;
        void genTangents4() noexcept;

        std::vector<Mesh> split(size_t maxVertexCount) const;
    };
}
