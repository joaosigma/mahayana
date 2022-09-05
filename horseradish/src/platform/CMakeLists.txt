cmake_minimum_required(VERSION 3.17.3)

set(HR_SOURCES
	platform.hpp
	platformWin32.cpp
	window.cpp
	window.hpp
	windowImplWin32.cpp
	windowImplWin32.hpp
)

target_sources(horseradish PRIVATE ${HR_SOURCES})
target_sources(horseradish-utests PRIVATE ${HR_SOURCES})
