#pragma once
#ifndef __HPRIMITIVES_2D__
#define __HPRIMITIVES_2D__

#include "Math.hpp"

namespace HorseRadish
{

namespace Primitives2D
{

//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§ Classe Rectangle		§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
template <typename T>
struct Rectangle{
	T x, y, width, height;

	//constructores
	inline Rectangle()
		{ x = y = width = height = 0; }
	inline Rectangle(const Rectangle &r)
		{ x = r.x; y = r.y; width = r.width; height = r.height; }
	explicit inline Rectangle(const T &x, const T &y, const T &width, const T &height)
		{ this->x = x; this->y = y; this->width = width; this->height = height; }

	//declarar destructor
	inline ~Rectangle()
		{ return; }
	
	//operadores para obter o ponteiro
	inline operator T *(void)
		{ return &x; }
	inline operator const T *(void) const
		{ return &x; }

	//o operador de igual
	inline Rectangle& operator=(const Rectangle &r)
	{
		x = r.x; y = r.y; width = r.width; height = r.height;
		return *this;
	}

	//os unitários para operações mais rápidas
	inline void operator+=(const Rectangle &r)
		{ x += r.x; y += r.y; width += r.width; height += r.height; }
	inline void operator-=(const Rectangle &r)
		{ x -= r.x; y -= r.y; width -= r.width; height -= r.height; }
	    
	//operadores binários (usar prudentemente por causa das cópias desnecessárias)
	inline Rectangle operator+(const Rectangle &r) const
		{ return Rectangle(x + r.x, y + r.y, width + r.width, height + r.height); }
	inline Rectangle operator-(const Rectangle &r) const
		{ return Rectangle(x - r.x, y - r.y, width - r.width, height - r.height); }

	//redundancia, mas dá jeito
	inline void Reset()
		{ x = y = width = height = 0; }
	inline void Set(const Rectangle &r)
		{ x = r.x; y = r.y; width = r.width; height = r.height; }
	inline void Set(const Rectangle * const r)
		{ x = r->x; y = r->y; width = r->width; height = r->height; }
	inline void Set(const T &x, const T &y, const T &width, const T &height)
		{ this->x = x; this->y = y; this->width = width; this->height = height; }

	//alguns funções de controlo
	void Move(const T &amount)
		{ x += amount; y += amount; }
	void Move(const T &x, const T &y)
		{ this->x += x; this->y += y; }
	void Grow(const T &amount)
		{ width += amount; height += amount; }
	void Grow(const T &width, const T &height)
		{ this->width += width; this->height += height; }

	//algumas funções para obter informações
	T Area() const
		{ return (width * height); }
};

//§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§ Classe Size		§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§§
template <typename T>
struct Size{
	T width, height;

	//constructores
	inline Size()
		{ width = height = 0; }
	inline Size(const Size &s)
		{ width = s.width; height = s.height; }
	explicit inline Size(const T &width, const T &height)
		{ this->width = width; this->height = height; }

	//declarar destructor
	inline ~Size()
		{ return; }
	
	//operadores para obter o ponteiro
	inline operator T *(void)
		{ return &x; }
	inline operator const T *(void) const
		{ return &x; }

	//o operador de igual
	inline Size& operator=(const Size &s)
	{
		width = s.width; height = s.height;
		return *this;
	}

	//os unitários para operações mais rápidas
	inline void operator+=(const Size &s)
		{ width += s.width; height += s.height; }
	inline void operator-=(const Size &s)
		{ width -= s.width; height -= s.height; }
	    
	//operadores binários (usar prudentemente por causa das cópias desnecessárias)
	inline Size operator+(const Size &s) const
		{ return Size(width + s.width, height + s.height); }
	inline Size operator-(const Size &s) const
		{ return Size(width - s.width, height - s.height); }

	//redundancia, mas dá jeito
	inline void Reset()
		{ width = height = 0; }
	inline void Set(const Size &s)
		{ width = s.width; height = s.height; }
	inline void Set(const Size * const s)
		{ width = s->width; height = s->height; }
	inline void Set(const T &width, const T &height)
		{ this->width = width; this->height = height; }

	//alguns funções de controlo
	void Grow(const T &amount)
		{ width += amount; height += amount; }
	void Grow(const T &width, const T &height)
		{ this->width += width; this->height += height; }

	//algumas funções para obter informações
	T Area() const
		{ return (width * height); }
};

//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
//§§§§§§ Classe Point		§§
//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§
template <typename T>
struct Point{
	T x, y;

	//constructores
	inline Point()
		{ x = y = 0; }
	inline Point(const Point &p)
		{ x = p.x; y = p.y; }
	explicit inline Point(const T &x, const T &y)
		{ this->x = x; this->y = y; }

	//declarar destructor
	inline ~Point()
		{ return; }
	
	//operadores para obter o ponteiro
	inline operator T *(void)
		{ return &x; }
	inline operator const T *(void) const
		{ return &x; }

	//o operador de igual
	inline Point& operator=(const Point &p)
	{
		x = p.x; y = p.y;
		return *this;
	}

	//os unitários para operações mais rápidas
	inline void operator+=(const Point &p)
		{ x += p.x; y += p.y; }
	inline void operator-=(const Point &p)
		{ x -= p.x; y -= p.y; }
	    
	//operadores binários (usar prudentemente por causa das cópias desnecessárias)
	inline Point operator+(const Point &p) const
		{ return Point(x + p.x, y + p.y); }
	inline Point operator-(const Point &p) const
		{ return Point(x - p.x, y - p.y); }

	//redundancia, mas dá jeito
	inline void Reset()
		{ x = y = 0; }
	inline void Set(const Point &p)
		{ x = p.x; y = p.y; }
	inline void Set(const Point * const p)
		{ x = p->x; y = p->y; }
	inline void Set(const T &x, const T &y)
		{ this->x = x; this->y = y; }

	//alguns funções de controlo
	void Move(const T &amount)
		{ x += amount; y += amount; }
	void Move(const T &x, const T &y)
		{ this->x += x; this->y += y; }
};

}//namespace Primitives2D
}//namespace HorseRadish

#endif
