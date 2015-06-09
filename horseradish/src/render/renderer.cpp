#include "renderer.hpp"

namespace HorseRadish
{
	namespace Render
	{
		Renderer::Renderer(const HorseRadish::OpenGL::Objects::Context& glContext)
			: mGlContext(glContext)
		{
		}

		Renderer::~Renderer()
		{
		}
	}
}