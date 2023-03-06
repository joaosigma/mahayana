cmake_minimum_required(VERSION 3.17.3)

set(HR_SOURCES
	app.cpp
	app.hpp
	build_tools.hpp
	objects.cpp
	objects.hpp
)

target_sources(horseradish PRIVATE ${HR_SOURCES})
target_sources(horseradish-utests PRIVATE ${HR_SOURCES})
