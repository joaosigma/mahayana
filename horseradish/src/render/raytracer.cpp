module;

#include <libs/bvh/bvh.hpp>
#include <libs/bvh/primitive_intersectors.hpp>
#include <libs/bvh/ray.hpp>
#include <libs/bvh/single_ray_traverser.hpp>
#include <libs/bvh/sphere.hpp>
#include <libs/bvh/sweep_sah_builder.hpp>
#include <libs/bvh/triangle.hpp>
#include <libs/bvh/vector.hpp>

module RayTracer;

import std;

import core;

namespace hr::render
{
    namespace
    {
        static thread_local std::mt19937 sRandGen;

        double genRand(double min, double max) noexcept
        {
            return std::uniform_real_distribution<double>(min, max)(sRandGen);
        }

        class Material;

        struct Hit
        {
            double rayT;
            Vector3d rayPoint;
            Vector3d normal;
            bool frontFace;

            const Material* mat{nullptr};
        };

        class Material
        {
        public:
            enum class Type
            {
                None,
                Lambertian,
                Metal
            };

        private:
            Type mType = Type::None;
            Colord mAlbedo;
            double mFuzziness{0.0};

        private:
            static Vector3d reflect(const Vector3d& vec, const Vector3d& normal)
            {
                return vec - (normal * 2.0 * vec.dot(normal));
            }

        private:
            Material() = default;

            Vector3d randomInUnitSphere() const
            {
                Vector3d inUnitSphere;
                while (true) // algo: just pick a random point inside the "unit cube" and reject if outside the sphere
                {
                    inUnitSphere = Vector3d{genRand(-1.0, 1.0), genRand(-1.0, 1.0), genRand(-1.0, 1.0)};
                    if (inUnitSphere.dot() < 1.0)
                        return inUnitSphere;
                }
            }

            Vector3d randomInUnitVector() const
            {
                auto p = randomInUnitSphere();
                p.normalize();

                return p;
            }

        public:
            static Material makeLambertian(Colord albedo)
            {
                Material mat;
                mat.mType = Type::Lambertian;
                mat.mAlbedo = albedo;

                return mat;
            }

            static Material makeMetal(Colord albedo, double fuzziness = 0.0)
            {
                Material mat;
                mat.mType = Type::Metal;
                mat.mAlbedo = albedo;
                mat.mFuzziness = Math::fClamp(fuzziness, 0.0, 1.0);

                return mat;
            }

            bool scatter(const Ray<Vector3d>& ray, const Hit& hit, Colord& attenuation, Ray<Vector3d>& scattered) const
            {
                switch (mType)
                {
                    case Type::Lambertian:
                    {
                        auto scatteredDir = hit.normal + randomInUnitVector();
                        if (scatteredDir.isZero(1e-8))
                            scatteredDir = hit.normal;

                        scattered = Ray<Vector3d>(hit.rayPoint, scatteredDir);
                        attenuation = mAlbedo;

                        return true;
                    }
                    case Type::Metal:
                    {
                        auto reflected = reflect(ray.direction(), hit.normal);
                        if (mFuzziness > 0.0)
                            scattered = Ray<Vector3d>{hit.rayPoint, reflected + (randomInUnitSphere() * mFuzziness)};
                        else
                            scattered = Ray<Vector3d>{hit.rayPoint, reflected};
                        attenuation = mAlbedo;

                        return (scattered.direction().dot(hit.normal) > 0.0);
                    }
                    case Type::None:
                    default:
                        return false;
                }
            }
        };

        class Window
        {
            tools::Camera mCamera;
            gl::tools::Viewport mViewport;
            struct
            {
                Vector3d rayOrigin;
                Vector3d horizontal, vertical, lower_left_corner;
            } mData;

            void update()
            {
                auto h = std::tan(mViewport.yfov() * 0.5);
                auto viewport_height = 2.0 * h;
                auto viewport_width = mViewport.aspectRatio() * viewport_height;

                auto w = Vector3d::calcNormalize((mCamera.getPos() - mCamera.getTarget()).convert<double, 3>());
                auto u = Vector3d::calcNormalize(mCamera.getUp().convert<double, 3>().crossProduct(w));
                auto v = w.crossProduct(u);

                mData.horizontal = u * viewport_width;
                mData.vertical = v * viewport_height;
                mData.lower_left_corner = mCamera.getPos().convert<double, 3>() - (mData.horizontal / 2) - (mData.vertical / 2) - w;
                mData.rayOrigin = mCamera.getPos().convert<double, 3>();
            }

        public:
            Window(tools::Camera camera, gl::tools::Viewport viewport)
              : mCamera{std::move(camera)}, mViewport{std::move(viewport)}
            {
                update();
            }

            void setCamera(tools::Camera camera)
            {
                mCamera = std::move(camera);
                update();
            }

            void setViewport(gl::tools::Viewport viewport)
            {
                mViewport = std::move(viewport);
                update();
            }

            template<size_t NSamples>
            std::array<Ray<Vector3d>, NSamples> generateRays(size_t pixelX, size_t pixelY)
            {
                static_assert(NSamples >= 1, "Samples must be greater or equal to 1");

                std::array<Ray<Vector3d>, NSamples> rays;

                auto dx = static_cast<double>(pixelX);
                auto dy = static_cast<double>(pixelY);
                auto dWidthInv = 1.0 / mViewport.width();
                auto dHeightInv = 1.0 / mViewport.height();

                if constexpr (NSamples == 1)
                {
                    auto u = dx * dWidthInv;
                    auto v = dy * dHeightInv;

                    rays[0] = Ray<Vector3d>(mData.rayOrigin, Vector3d::calcNormalize(mData.lower_left_corner + (mData.horizontal * u) + (mData.vertical * v) - mData.rayOrigin));
                }
                else
                {
                    for (size_t curSample = 0; curSample < NSamples; curSample++)
                    {
                        auto u = (dx + genRand(0.0, 1.0)) * dWidthInv;
                        auto v = (dy + genRand(0.0, 1.0)) * dHeightInv;

                        rays[curSample] =
                          Ray<Vector3d>(mData.rayOrigin, Vector3d::calcNormalize(mData.lower_left_corner + (mData.horizontal * u) + (mData.vertical * v) - mData.rayOrigin));
                    }
                }

                return rays;
            }
        };

        class World
        {
            struct TriangleData: public Triangle<Vector3d>
            {
                TriangleData(const Vector3d& p1, const Vector3d& p2, const Vector3d& p3)
                  : Triangle(p1, p2, p3)
                {
                    centroid = Triangle::orthocenter();
                }

                Vector3d centroid;
            };

            struct MeshBVH
            {
                bvh::Bvh<double> bvh;
                std::vector<bvh::Triangle<double>> prims;
            };

            std::vector<Material> mMaterials;
            std::vector<std::tuple<BSphere<Vector3d>, Material*>> mObjects;
            std::vector<std::tuple<geom::Mesh<geom::VertexFull, uint32_t>, Material*>> mObjects2;
            std::vector<MeshBVH> mMeshBVHhData;

            void buildBHV()
            {
                mMeshBVHhData.reserve(mObjects2.size());

                for (const auto& objData : mObjects2)
                {
                    auto& obj = std::get<0>(objData);

                    // mBBox.merge(obj.bbox().convert<Vector3d>());

                    MeshBVH meshData;

                    // fill elements
                    meshData.prims.reserve(obj.numTris());
                    obj.iterateTris(
                      [&prims = meshData.prims](size_t triIndex, const geom::VertexFull& p1, const geom::VertexFull& p2, const geom::VertexFull& p3) -> bool
                      {
                          // Triangle<Vector3d> tri{ Vector3f{ p1.pos }.convert<double>(), Vector3f{ p2.pos }.convert<double>(), Vector3f{ p3.pos }.convert<double>() };

                          bvh::Triangle<double> tri{bvh::Vector3<double>{p1.pos[0], p1.pos[1], p1.pos[2]}, bvh::Vector3<double>{p2.pos[0], p2.pos[1], p2.pos[2]},
                                                    bvh::Vector3<double>{p3.pos[0], p3.pos[1], p3.pos[2]}};
                          prims.push_back(std::move(tri));

                          // tri.id = triIndex;
                          // tri.bbox.reset();
                          // tri.bbox.mergeAll(tri[0], tri[1], tri[2]);
                          // tri.centroid = tri.orthocenter();
                          // prims.push_back(std::move(tri));

                          return true;
                      });

                    // build bvh
                    auto [bboxes, centers] = bvh::compute_bounding_boxes_and_centers(meshData.prims.data(), meshData.prims.size());
                    auto global_bbox = bvh::compute_bounding_boxes_union(bboxes.get(), meshData.prims.size());

                    bvh::SweepSahBuilder<bvh::Bvh<double>> builder(meshData.bvh);
                    builder.build(global_bbox, bboxes.get(), centers.get(), meshData.prims.size());

                    // meshData.bvh.build(elements);

                    mMeshBVHhData.push_back(std::move(meshData));
                }
            }

        public:
            World()
            {
                mMaterials.emplace_back(Material::makeLambertian(Colord{0.7, 0.3, 0.0}));
                mMaterials.emplace_back(Material::makeLambertian(Colord{1.0, 0.6, 0.3}));
                mMaterials.emplace_back(Material::makeMetal(Colord{0.0, 0.6, 1.0}, 1.0));
                mMaterials.emplace_back(Material::makeMetal(Colord{0.0, 0.6, 1.0}, 0.3));

                mObjects.emplace_back(BSphere<Vector3d>{Vector3d{0.0, -100.5, 0.0}, 100.0}, nullptr);
                mObjects.emplace_back(BSphere<Vector3d>{Vector3d{0.0, 0.0, 0.0}, 0.5}, mMaterials.data() + 0);
                mObjects.emplace_back(BSphere<Vector3d>{Vector3d{-1.0, 0.0, 0.0}, 0.5}, mMaterials.data() + 1);
                mObjects.emplace_back(BSphere<Vector3d>{Vector3d{1.0, 0.0, 0.0}, 0.5}, mMaterials.data() + 2);

                mObjects2.emplace_back(geom::Factory::genCube(), mMaterials.data() + 3);
                std::get<0>(mObjects2.back()).scale(2.0f);
                std::get<0>(mObjects2.back()).centerMass(Vector3f{0.0f, 0.5f, -1.6f});

                mObjects2.emplace_back(geom::Factory::genCube(), mMaterials.data() + 0);
                std::get<0>(mObjects2.back()).scale(2.0f);
                std::get<0>(mObjects2.back()).centerMass(Vector3f{3.0f, 0.5f, -1.6f});

                mObjects2.emplace_back(geom::Factory::genCube(), mMaterials.data() + 0);
                std::get<0>(mObjects2.back()).scale(2.0f);
                std::get<0>(mObjects2.back()).centerMass(Vector3f{-3.0f, 0.5f, -1.6f});

                // debug: 2,08 minutes
                // release: 16s

                buildBHV();
            }

            bool hit(const Ray<Vector3d>& ray, double tMin, double tMax, Hit& hit) const
            {
                bool hitSomething{false};

                // spheres
                {
                    Hit curHit;

                    for (const auto& [obj, mat] : mObjects)
                    {
                        if (!obj.intersects(ray, tMin, tMax, curHit.rayT))
                            continue;

                        {
                            curHit.rayPoint = ray.pointAt(curHit.rayT);

                            auto outNormal = (curHit.rayPoint - obj.center()) / obj.radius();
                            outNormal.normalize();

                            curHit.frontFace = (ray.direction().dot(outNormal) < 0.0);
                            curHit.normal = hit.frontFace ? outNormal : -outNormal; // normal is always out (as if the ray hit from the outside)

                            curHit.mat = mat;
                        }

                        hitSomething = true;
                        hit = curHit;
                        tMax = hit.rayT;
                    }
                }

                // meshes

                {
                    auto translate = [](const Vector3d& vec)
                    {
                        return bvh::Vector3<double>{vec[0], vec[1], vec[2]};
                    };

                    bvh::Ray<double> newRay{translate(ray.origin()), translate(ray.direction()), tMin, tMax};

                    for (size_t objIndex{0}; const auto& mData : mMeshBVHhData)
                    {
                        auto& targetObj = mObjects2[objIndex++];

                        bvh::ClosestPrimitiveIntersector<bvh::Bvh<double>, bvh::Triangle<double>> primitive_intersector(mData.bvh, mData.prims.data());
                        bvh::SingleRayTraverser<bvh::Bvh<double>> traverser(mData.bvh);

                        newRay.tmax = tMax;

                        auto meshHit = traverser.traverse(newRay, primitive_intersector);
                        if (!meshHit)
                            continue;

                        hitSomething = true;
                        tMax = meshHit->distance();

                        hit.rayT = meshHit->intersection.t;
                        hit.mat = std::get<1>(targetObj);
                        hit.rayPoint = ray.pointAt(hit.rayT);

                        auto outNormal = std::get<0>(targetObj)
                                           .triNormal(meshHit->primitive_index, static_cast<float>(meshHit->intersection.u), static_cast<float>(meshHit->intersection.v))
                                           .convert<double, 3>();

                        hit.frontFace = (ray.direction().dot(outNormal) < 0.0);
                        hit.normal = hit.frontFace ? outNormal : -outNormal; // normal is always out (as if the ray hit from the outside)
                    }
                }

                // for (size_t objIndex{ 0 }; const auto & bvh : mMeshBVHhData)
                //{
                //	auto& targetObj = mObjects2[objIndex++];
                //
                //	geom::Mesh<geom::VertexFull, uint32_t>::Hit meshHit;
                //	meshHit.rayT = std::numeric_limits<double>::infinity();
                //
                //	double rayT;
                //	auto wasHit = bvh.bvh.intersects(ray, tMin, tMax, [&obj = std::get<0>(targetObj), &meshHit](std::span<size_t> triIndices, const Ray<Vector3d>& ray, double
                // rayDistMin, double rayDistMax, double& rayT)
                //	{
                //		auto wasHit{ false };
                //		for (auto triIndex : triIndices)
                //		{
                //			geom::Mesh<geom::VertexFull, uint32_t>::Hit hit;
                //			if (!obj.intersects(triIndex, ray, rayDistMin, rayDistMax, hit))
                //				continue;
                //
                //			rayDistMax = hit.rayT;
                //
                //			if (hit.rayT < meshHit.rayT)
                //				meshHit = hit;
                //
                //			wasHit = true;
                //		}
                //
                //		if (wasHit)
                //			rayT = meshHit.rayT;
                //
                //		return wasHit;
                //
                //	}, rayT);
                //
                //	if (!wasHit)
                //		continue;
                //
                //	hitSomething = true;
                //	tMax = meshHit.rayT;
                //
                //	hit.rayT = rayT;
                //	hit.mat = std::get<1>(targetObj);
                //	hit.rayPoint = ray.pointAt(rayT);
                //
                //	auto outNormal = Vector3d{ std::get<0>(targetObj).triNormal(meshHit.triIndex, meshHit.barycentricU, meshHit.barycentricV) };
                //
                //	hit.frontFace = (ray.direction().getDot(outNormal) < 0.0);
                //	hit.normal = hit.frontFace ? outNormal : -outNormal; //normal is always out (as if the ray hit from the outside)
                // }

                return hitSomething;
            }
        };

        const World world;
    }

    Vector3d Raytracer::randomInHemisphere(const Vector3d& normal)
    {
        Vector3d inUnitSphere;
        while (true) // algo: just pick a random point inside the "unit cube" and reject if outside the sphere
        {
            inUnitSphere = Vector3d{genRand(-1.0, 1.0), genRand(-1.0, 1.0), genRand(-1.0, 1.0)};
            if (inUnitSphere.dot() < 1.0)
                break;
        }

        if (normal.dot(inUnitSphere) > 0.0)
            return inUnitSphere;
        return -inUnitSphere;
    }

    Colord Raytracer::rayColor(const Ray<Vector3d>& r, size_t depth)
    {
        if (depth <= 0)
            return Colord{0.0, 0.0, 0.0}; // don't gather any more light

        Hit hit;
        if (world.hit(r, 0.00001, std::numeric_limits<double>::infinity(), hit))
        {
            if (!hit.mat)
            {
                auto target = hit.rayPoint + randomInHemisphere(hit.normal);
                return (rayColor(Ray<Vector3d>{hit.rayPoint, Vector3d::calcNormalize(target - hit.rayPoint)}, depth - 1) * 0.5);
            }

            Ray<Vector3d> scattered;
            Colord attenuation;

            if (hit.mat->scatter(r, hit, attenuation, scattered))
                return attenuation * rayColor(scattered, depth - 1);
            return Colord{0.0, 0.0, 0.0};
        }

        return Colord::calcInterpolate(Colord{1.0, 1.0, 1.0}, Colord{0.5, 0.7, 1.0}, (r.direction()[1] + 1.0) * 0.5); // blueish sky
    }

    Raytracer::Raytracer(Dispatcher& asyncDispatcher, size_t maxWidth, size_t maxHeight)
      : mAsyncDispatcher{asyncDispatcher}
    {
        mBuffer = imaging::Image<float, imaging::ImageFormatRGB>::create(maxWidth, maxHeight);
    }

    bool Raytracer::trace(const tools::Camera& camera, const gl::tools::Viewport& viewport)
    {
        auto imgWidth = mBuffer.width();
        auto imgHeight = mBuffer.height();
        assert((imgWidth == viewport.width()) && (imgHeight == viewport.height()));

        auto myCam = camera;
        myCam.setPos(-1.5f, 1.5f, 1.5f);
        // myCam.setPos(0.0f, 0.0f, 1.0f);
        myCam.setTarget(0.0f, 0.0f, 0.0f);

        Window window(myCam, viewport);

        constexpr size_t raysPerSample{80};
        constexpr size_t raysMaxDepth{30};

        if (true) // multithread
        {
            auto doLine = [&](size_t row)
            {
                assert((row >= 0) && (row < imgHeight));
                for (size_t i = 0; i < imgWidth; ++i)
                {
                    Colord pixelColor(0.0);

                    auto rays = window.generateRays<raysPerSample>(i, row);
                    for (const auto& ray : rays)
                        pixelColor += rayColor(ray, raysMaxDepth);

                    if (rays.size() > 1)
                        pixelColor /= static_cast<double>(rays.size());

                    mBuffer.setPixel(i, imgHeight - row - 1, pixelColor.convert<float>());
                }
            };

            for (size_t j = 0; j < imgHeight; ++j)
                mAsyncDispatcher.post(doLine, j);

            mAsyncDispatcher.wait();
        }
        else
        {
            for (size_t j = 0; j < imgHeight; ++j)
            {
                for (size_t i = 0; i < imgWidth; ++i)
                {
                    Colord pixelColor(0.0);

                    auto rays = window.generateRays<raysPerSample>(i, j);
                    for (const auto& ray : rays)
                        pixelColor += rayColor(ray, raysMaxDepth);

                    if (rays.size() > 1)
                        pixelColor /= static_cast<double>(rays.size());

                    mBuffer.setPixel(i, imgHeight - j - 1, pixelColor.convert<float>());
                }
            }
        }

        // TODO: apply tone mapping to image

        return true;
    }
}
