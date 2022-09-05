cmake_minimum_required(VERSION 3.17.3)

set(HR_SOURCES
	camera.cpp
	camera.hpp
	font.cpp
	font.hpp
	texture.cpp
	texture.hpp
)

target_sources(horseradish PRIVATE ${HR_SOURCES})
target_sources(horseradish-utests PRIVATE ${HR_SOURCES})
