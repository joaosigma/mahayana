export module gal:tools;

import std;

import core;
import :objects;

namespace hr::gl::tools
{
    export class Frustum
    {
    public:
        enum class IntersectionType
        {
            FullInside,
            FullOutside,
            FrustumIntersect
        };
        enum class PlaneIndex : size_t
        {
            Left = 0,
            Right = 1,
            Top = 2,
            Bottom = 3,
            Near = 4,
            Far = 5
        };

    private:
        std::array<hr::Plane<float>, 6> mPlanes{hr::Plane<float>::zero(), hr::Plane<float>::zero(), hr::Plane<float>::zero(),
                                                hr::Plane<float>::zero(), hr::Plane<float>::zero(), hr::Plane<float>::zero()};
        hr::Vector3f mPosition;
        float mZNear{0.0f}, mZFar{0.0f};

        void extractPlanes(const hr::Vector4f& col1, const hr::Vector4f& col2, const hr::Vector4f& col3, const hr::Vector4f& col4);
        bool sweptSpherePlaneIntersect(float& t0,
                                       float& t1,
                                       const hr::Plane<float>& plane,
                                       const hr::Vector3f& sphereCenter,
                                       const float& sphereRadius,
                                       const hr::Vector3f& sweepDir) const;

    public:
        Frustum() = default;

        bool testCube(const hr::Vector3f& point, const float& size) const;
        bool testBox(const hr::Vector3f& min, const hr::Vector3f& max) const;
        bool testBox(const hr::BBox<>& bbox) const;
        bool testSphere(const hr::Vector3f& center, const float& radius) const;
        bool testSphere(const hr::BSphere<hr::Vector3f>& bsphere) const;
        bool testSphereBox(const hr::BSphere<hr::Vector3f>& bsphere, const hr::BBox<>& bbox) const;
        bool testPoint(const hr::Vector3f& point) const;
        bool testPolygon(const hr::Vector3f* const points, size_t numPoints) const;
        bool testSquare(const hr::Vector3f points[4]) const;
        bool testTri(const hr::Vector3f points[3]) const;
        bool testSweptSphere(const hr::Vector3f& sphereCenter, const float& sphereRadius, const hr::Vector3f& sweepDir) const;

        float getZNear(void) const
        {
            return mZNear;
        }

        float getZFar(void) const
        {
            return mZFar;
        }

        const hr::Vector3f& getCamPosition(void) const
        {
            return mPosition;
        }

        const hr::Plane<float>& getPlane(PlaneIndex planeIndex) const
        {
            return mPlanes[static_cast<size_t>(planeIndex)];
        }

        void getCorners(hr::Vector3f points[8]) const;

        IntersectionType classifyFrustum(const Frustum& frustum) const;

        float dotNormals(const PlaneIndex planeA, const PlaneIndex planeB) const;

        void calculateFrustum(const hr::Matrix4f& transformation);
        void calculateFrustum(const hr::Matrix4f& projection, const hr::Matrix4f& modelview);
        void calculateFrustum(const hr::Matrix4f& modelView, const hr::Matrix4f& projection, const hr::Vector3f& pos, float zNear, float zFar);

        void setIndividualPlane(const PlaneIndex planeIndex, const hr::Plane<float>& plane);
        void setFrustum(const hr::Vector3f& bboxMin, const hr::Vector3f& bboxMax);
        void setFrustum(const hr::Vector3f& center, const float radius);
        void setFrustum(const hr::BBox<>& bbox);

        void setZNear(const float ZNear)
        {
            mZNear = ZNear;
        }

        void setZFar(const float ZFar)
        {
            mZFar = ZFar;
        }

        void setCamPosition(const float x, const float y, const float z)
        {
            mPosition = Vector3f{x, y, z};
        }

        void setCamPosition(const hr::Vector3f& pos)
        {
            mPosition = pos;
        }
    };

    export class Viewport
    {
    public:
        enum class ProjectionType
        {
            Proj3D,
            Proj2D
        };

        static hr::Matrix4f genMatrix2DProj(size_t width, size_t height);

    private:
        double mYFov, mZNear;
        struct
        {
            double aspectRatio;
            size_t width, height;
        } mDims;

        struct
        {
            hr::Matrix4f mp2D = hr::Matrix4f::identity();
            hr::Matrix4f mp3D = hr::Matrix4f::identity();
        } mMatrices;

        void calcMatrices() noexcept;

    public:
        Viewport() = delete;

        explicit Viewport(const size_t width, const size_t height) noexcept
          : Viewport(Math::Deg2Rad<double> * 90.0, width, height)
        {}

        explicit Viewport(const double yfov, const size_t width, const size_t height) noexcept
          : Viewport(yfov, width, height, 0.1)
        {}

        explicit Viewport(const double yfov, const size_t width, const size_t height, const double zNear) noexcept
          : mYFov(yfov), mZNear(zNear)
        {
            mDims.width = width ? width : 1;
            mDims.height = height ? height : 1;
            mDims.aspectRatio = (static_cast<double>(mDims.width) / static_cast<double>(mDims.height));
            calcMatrices();
        }

        const hr::Matrix4f& getProjection(ProjectionType projectionType) const;

        double yfov() const
        {
            return mYFov;
        }
        double znear() const
        {
            return mZNear;
        }

        size_t width() const
        {
            return mDims.width;
        }
        size_t height() const
        {
            return mDims.height;
        }
        double aspectRatio() const
        {
            return mDims.aspectRatio;
        }

        void projectPoint(ProjectionType projType, const hr::Matrix4f& modelView, std::span<hr::Vector3f> points) const;
    };

    export class ImmediateMode
    {
    public:
        enum class GeometryType
        {
            None,
            Quads,
            Tris,
            Lines,
            LineStrip
        };
        enum class InfoType
        {
            FreeVertexCount,
            MaxVertexCount
        };

    private:
        static constexpr size_t MaxVertexCount = 400; // 100 quads
        static constexpr size_t MaxIndexCount = ((MaxVertexCount / 4) * 6) + 6;

        struct VertexDataLayout
        {
            float pos[3], uv[2];
            unsigned char color[4];
        };

        struct
        {
            hr::gl::objects::FenceSync fence;
            hr::gl::objects::VertexArray vertexArray;
            hr::gl::objects::Buffer arrayBuffer, elementArrayBuffer;
        } mGl;

        struct
        {
            float uv[2];
            size_t curVertex;
            GeometryType geomType;
            unsigned char color[4];
        } mState;

        std::array<VertexDataLayout, MaxVertexCount> mBufferData;
        std::array<unsigned short, MaxIndexCount> mBufferIndices;

        void draw(bool keepLeftovers);
        void resetState();
        bool checkStateDraw() const;

    public:
        ImmediateMode();
        ~ImmediateMode();

        void beginDraw(const GeometryType geometryType);
        void endDraw();

        void setTexCoord(const float u, const float v);
        void setColor(const unsigned char r, const unsigned char g, const unsigned char b);
        void setColor(const unsigned char r, const unsigned char g, const unsigned char b, const unsigned char a);
        void setColorF(const float rgb);
        void setColorF(const float rgb, const float a);
        void setColorF(const float r, const float g, const float b);
        void setColorF(const float r, const float g, const float b, const float a);
        void setColorRGB(const unsigned char* const values);
        void setColorRGB(const float* const values);

        void addPosition(const float x);
        void addPosition(const float x, const float y);
        void addPosition(const float x, const float y, const float z);
        void addPosition(const Vector3f& vec);

        void addQuad(const float x, const float y, const float width, const float height);
        void addQuadTexCoords(const float x, const float y, const float width, const float height, bool normalizedTexCoords);
        void addLine(const float x1, const float y1, const float x2, const float y2);
        void addLineH(const float x1, const float x2, const float y);
        void addLineV(const float x, const float y1, const float y2);

        size_t info(const InfoType infoType) const;
    };
}
