#pragma once

#include "world.hpp"
#include "common\FileSystem.hpp"
#include "common\OpenGL\objects.hpp"

namespace HorseRadish
{
	namespace Render
	{
		class Renderer
		{
		protected:

			const HorseRadish::OpenGL::Objects::Context& mGlContext;

		public:
			Renderer(const HorseRadish::OpenGL::Objects::Context& glContext);
			virtual ~Renderer();
		};
	}
}
