cmake_minimum_required(VERSION 4.2.1)

set(HR_SOURCES
	camera.cpp
	font.cpp
	texture.cpp
)

set(HR_MODULES
	camera.cppm
	font.cppm
	texture.cppm
)

target_sources(horseradish
	PRIVATE
		${HR_SOURCES}
	PRIVATE
		FILE_SET CXX_MODULES
		FILES ${HR_MODULES}
)
