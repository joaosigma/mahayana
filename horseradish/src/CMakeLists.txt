cmake_minimum_required(VERSION 3.17.3)

target_sources(horseradish
	PRIVATE
		build.hpp
		main.cpp
)

add_subdirectory(common)
add_subdirectory(engine)
add_subdirectory(libs)
add_subdirectory(misc)
add_subdirectory(platform)
add_subdirectory(render)
