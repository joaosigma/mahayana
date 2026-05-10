cmake_minimum_required(VERSION 4.2.1)

file(GLOB_RECURSE HR_LIBS_SOURCES
	LIST_DIRECTORIES false
	${CMAKE_CURRENT_LIST_DIR} "*.c" "*.cc" "*.cpp" "*.h" "*.hpp")

target_sources(horseradish PRIVATE ${HR_LIBS_SOURCES})

if (BUILD_TESTING)
	target_sources(horseradish-utests PRIVATE ${HR_LIBS_SOURCES})
endif()
