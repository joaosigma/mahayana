#pragma once

#include "mesh.hpp"

namespace hr::geom
{
	class Factory
	{
	public:
		//platonic solids (don't have UVs)
		static Mesh<VertexFull, uint32_t> genCube();

		//parametric surfaces
		static Mesh<VertexFull, uint32_t> genTorus(size_t slices, size_t stacks, float innerRadius);

		//spheres
		static Mesh<VertexFull, uint32_t> genSphereParametric(size_t slices, size_t stacks);
		static Mesh<VertexFull, uint32_t> genSphereSubdivided(size_t numSubdivisions);
		
	};
}
