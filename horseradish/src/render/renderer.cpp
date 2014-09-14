#include "renderer.hpp"

namespace HorseRadish
{
	namespace Render
	{
		Renderer::Renderer(HorseRadish::OpenGL::Objects::Context * const glContext)
			: glContext(glContext)
		{
		}

		Renderer::~Renderer()
		{
		}
	}
}