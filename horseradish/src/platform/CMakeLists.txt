cmake_minimum_required(VERSION 3.17.3)

target_sources(horseradish
	PRIVATE
		platform.hpp
		platformWin32.cpp
		window.cpp
		window.hpp
		windowImplWin32.cpp
		windowImplWin32.hpp
)
