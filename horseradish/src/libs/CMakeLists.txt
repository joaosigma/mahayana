cmake_minimum_required(VERSION 3.17.3)

file(GLOB_RECURSE HR_LIBS_SOURCES
	LIST_DIRECTORIES false
	${CMAKE_CURRENT_LIST_DIR} "*.c" "*.cc" "*.cpp" "*.h" "*.hpp")

set(HR_LIBS_SOURCES_UTESTS ${HR_LIBS_SOURCES})

list(FILTER HR_LIBS_SOURCES EXCLUDE REGEX ".+\/libs\/catch2\/.+")

target_sources(horseradish PRIVATE ${HR_LIBS_SOURCES})
target_sources(horseradish-utests PRIVATE ${HR_LIBS_SOURCES_UTESTS})
