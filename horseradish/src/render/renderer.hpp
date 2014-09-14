#pragma once

#include "world.hpp"
#include "common\Mesh.hpp"
#include "common\MeshFactory.hpp"
#include "common\FileSystem.hpp"
#include "common\OpenGL\objects.hpp"
#include "common\OpenGL\tools.hpp"

namespace HorseRadish
{
	namespace Render
	{
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
	}
}
