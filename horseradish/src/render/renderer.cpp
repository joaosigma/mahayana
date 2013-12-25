#include "renderer.hpp"

namespace HorseRadish
{

namespace Render
{

//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§   Classe Renderer	§§§§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
Renderer::Renderer(HorseRadish::OpenGL::Objects::Context * const glContext)
	: glContext(glContext)
{
}

Renderer::~Renderer()
{
}

}//namespace Render
}//namespace HorseRadish