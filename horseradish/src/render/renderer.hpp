#pragma once

#include "world.hpp"
#include "common\Mesh.hpp"
#include "common\MeshFactory.hpp"
#include "common\FileSystem.hpp"
#include "common\OpenGL\objects.hpp"

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

			const HorseRadish::OpenGL::Objects::Context& mGlContext;
			Stats mStats;

		public:
			Renderer(const HorseRadish::OpenGL::Objects::Context& glContext);
			virtual ~Renderer();
		};
	}
}
