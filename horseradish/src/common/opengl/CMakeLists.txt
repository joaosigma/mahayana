cmake_minimum_required(VERSION 3.17.3)

set(HR_SOURCES
	context.cpp
	context.hpp
	objects.hpp
	openGL.cpp
	openGL.hpp
	openGLext.cpp
	openGLext.hpp
)

target_sources(horseradish PRIVATE ${HR_SOURCES})
target_sources(horseradish-utests PRIVATE ${HR_SOURCES})

add_subdirectory(tools)
