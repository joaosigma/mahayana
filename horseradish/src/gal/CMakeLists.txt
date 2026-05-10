cmake_minimum_required(VERSION 4.2.1)

set(HR_SOURCES
	context.cpp
	frustum.cpp
	immediateMode.cpp
	openGL.cpp
	openGLext.cpp
	viewport.cpp
)

set(HR_MODULES
	gal.cpp

	context.cppm
	objects.cppm
	openGL.cppm
	openGLext.cppm
	tools.cppm
	window.cppm
)

if (BUILD_ENV STREQUAL "windows")

	list(APPEND HR_SOURCES
		windowWin64.cpp
	)

endif()

target_sources(horseradish
	PRIVATE
		${HR_SOURCES}
	PRIVATE
		FILE_SET CXX_MODULES
		BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
		FILES ${HR_MODULES}
)
