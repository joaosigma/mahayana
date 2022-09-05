cmake_minimum_required(VERSION 3.17.3)

set(HR_SOURCES
	frustum.cpp
	frustum.hpp
	immediateMode.cpp
	immediateMode.hpp
	viewport.cpp
	viewport.hpp
)

target_sources(horseradish PRIVATE ${HR_SOURCES})
target_sources(horseradish-utests PRIVATE ${HR_SOURCES})
