#pragma once
#ifndef __HR_RENDER_RENDERER__
#define __HR_RENDER_RENDERER

#include "world.hpp"
#include "common\Common.hpp"
#include "common\Mesh.hpp"
#include "common\MeshFactory.hpp"
#include "common\Containers.hpp"
#include "common\FileSystem.hpp"
#include "common\OpenGL\objects.hpp"
#include "common\OpenGL\tools.hpp"

namespace HorseRadish
{

namespace Render
{

//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§   Classe Renderer	§§§§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
class Renderer
{
protected:
	
	struct Stats{
		unsigned int numTris, numGlDrawElements;

		Stats() : numTris(0), numGlDrawElements(0) { }
	};

	HorseRadish::OpenGL::Objects::Context *glContext;
	Stats stats;

public:
	Renderer(HorseRadish::OpenGL::Objects::Context * const glContext);
	virtual ~Renderer();
};

}//namespace Render
}//namespace HorseRadish

#endif