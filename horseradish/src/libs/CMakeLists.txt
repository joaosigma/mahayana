cmake_minimum_required(VERSION 3.17.3)

file(GLOB_RECURSE HR_LIBS_SOURCES
	LIST_DIRECTORIES false
	${CMAKE_CURRENT_LIST_DIR} "*.c" "*.cc" "*.cpp" "*.h" "*.hpp")

target_sources(horseradish
	PRIVATE
		${HR_LIBS_SOURCES}
)