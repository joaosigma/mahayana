cmake_minimum_required(VERSION 3.17.3)

target_sources(horseradish
	PRIVATE
		context.cpp
		context.hpp
		objects.hpp
		openGL.cpp
		openGL.hpp
		openGLext.cpp
		openGLext.hpp
)

add_subdirectory(tools)
