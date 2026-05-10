cmake_minimum_required(VERSION 4.2.1)

set(HR_SOURCES
	engine.cpp
	engineRender.cpp
	profiler.cpp
	runtime.cpp
)

set(HR_MODULES
	engine.cppm
	profiler.cppm
	runtime.cppm
)

target_sources(horseradish
	PRIVATE
		${HR_SOURCES}
	PRIVATE
		FILE_SET CXX_MODULES
		BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
		FILES ${HR_MODULES}
)
