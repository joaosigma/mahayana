export module RayTracer;

import std;

import core;
import Camera;
import gal;

namespace hr::render
{
    export class Raytracer
    {
        std::mt19937 mRandGen;
        Dispatcher& mAsyncDispatcher;
        imaging::Image<float, imaging::ImageFormatRGB> mBuffer;

    private:
        Vector3d randomInHemisphere(const Vector3d& normal);

        Colord rayColor(const Ray<Vector3d>& r, size_t depth);

    public:
        Raytracer(Dispatcher& asyncDispatcher, size_t maxWidth, size_t maxHeight);

        bool trace(const tools::Camera& camera, const gl::tools::Viewport& viewport);

        const imaging::ImageView<float, imaging::ImageFormatRGB>& buffer() const
        {
            return mBuffer;
        }
    };
}
