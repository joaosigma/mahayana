#pragma once

#include "../common/openGL/context.hpp"

namespace HorseRadish { namespace Render
{
	class Renderer
	{
	protected:
		const HorseRadish::OpenGL::Objects::Context& mGlContext;

	public:
		Renderer(const HorseRadish::OpenGL::Objects::Context& glContext)
			: mGlContext(glContext)
		{ }

		virtual ~Renderer()
		{ }
	};
} }
