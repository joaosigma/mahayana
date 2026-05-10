cmake_minimum_required(VERSION 4.2.1)

target_sources(horseradish
	PRIVATE
		FILE_SET HEADERS
		BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
		FILES
			build.hpp

	PRIVATE
		main.cpp
)

if (BUILD_TESTING)
	target_sources(horseradish-utests
		PRIVATE
			main_t.cpp
	)
endif()

add_subdirectory(common)
add_subdirectory(gal)
add_subdirectory(engine)
add_subdirectory(libs)
add_subdirectory(misc)
add_subdirectory(render)
