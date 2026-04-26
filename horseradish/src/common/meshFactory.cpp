#pragma once

#include "meshFactory.hpp"

#include "libs/par/par_shapes.h"

#include <memory>

namespace hr::geom
{
    namespace
    {
        auto par_destroyer = [](par_shapes_mesh* m)
        {
            if (m)
                par_shapes_free_mesh(m);
        };
        using par_ptr = std::unique_ptr<par_shapes_mesh, decltype(par_destroyer)>;

        Mesh<VertexFull, uint32_t> convert(const par_shapes_mesh& mesh)
        {
            assert(mesh.points && mesh.normals);
            assert(mesh.triangles);

            Mesh<VertexFull, uint32_t> meshOut(static_cast<size_t>(mesh.npoints), static_cast<size_t>(mesh.ntriangles) * 3);

            for (int i = 0; i < mesh.npoints; ++i)
            {
                auto& vertex = meshOut.vertex(i);

                std::memcpy(vertex.pos, mesh.points + (i * 3), sizeof(float) * 3);
                std::memcpy(vertex.normal, mesh.normals + (i * 3), sizeof(float) * 3);
                if (mesh.tcoords)
                    std::memcpy(vertex.uv, mesh.tcoords + (i * 2), sizeof(float) * 2);
            }

            auto numIndices = static_cast<size_t>(mesh.ntriangles) * 3;
            assert(numIndices == meshOut.numIndices());

            auto indices = meshOut.indices();
            for (size_t i = 0; i < numIndices; ++i)
                indices[i] = static_cast<uint32_t>(mesh.triangles[i]);

            return meshOut;
        }
    }

    Mesh<VertexFull, uint32_t> Factory::genCube()
    {
        par_ptr m{par_shapes_create_cube(), par_destroyer};
        par_shapes_unweld(m.get(), true);
        par_shapes_compute_normals(m.get());
        return convert(*m);
    }

    Mesh<VertexFull, uint32_t> Factory::genCylinder(size_t slices, size_t stacks)
    {
        par_ptr m{par_shapes_create_cylinder(static_cast<int>(slices), static_cast<int>(stacks)), par_destroyer};
        return convert(*m);
    }

    Mesh<VertexFull, uint32_t> Factory::genTorus(size_t slices, size_t stacks, float innerRadius)
    {
        par_ptr m{par_shapes_create_torus(static_cast<int>(slices), static_cast<int>(stacks), innerRadius), par_destroyer};
        return convert(*m);
    }

    Mesh<VertexFull, uint32_t> Factory::genSphereParametric(size_t slices, size_t stacks)
    {
        // creates a sphere with texture coordinates and small triangles near the poles
        par_ptr m{par_shapes_create_parametric_sphere(static_cast<int>(slices), static_cast<int>(stacks)), par_destroyer};
        return convert(*m);
    }

    Mesh<VertexFull, uint32_t> Factory::genSphereSubdivided(size_t numSubdivisions)
    {
        // creates a sphere from a subdivided icosahedron, which produces a nicer distribution of triangles, but no texture coordinates
        par_ptr m{par_shapes_create_subdivided_sphere(static_cast<int>(numSubdivisions)), par_destroyer};
        return convert(*m);
    }
}
