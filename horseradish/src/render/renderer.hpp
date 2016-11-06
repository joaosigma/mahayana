#pragma once

#include "../common/openGL/context.hpp"

namespace hr { namespace render
{
	class Renderer
	{
	protected:
		const hr::gl::objects::Context& mGlContext;

	public:
		Renderer(const hr::gl::objects::Context& glContext)
			: mGlContext(glContext)
		{ }

		virtual ~Renderer()
		{ }
	};
} }
