cmake_minimum_required(VERSION 4.2.1)

set(HR_SOURCES
	raytracer.cpp
	renderer2D.cpp
	rendererDebug.cpp
	rendererMain.cpp
	consoleUI.cpp
	profilerUI.cpp
	scene.cpp
	stage.cpp
	world.cpp
	worldEditor.cpp
)

set(HR_MODULES
	raytracer.cppm
	renderer.cppm
	renderer2D.cppm
	rendererDebug.cppm
	rendererMain.cppm
	consoleUI.cppm
	profilerUI.cppm
	scene.cppm
	stage.cppm
	world.cppm
	worldEditor.cppm
)

target_sources(horseradish
	PRIVATE
		${HR_SOURCES}
	PRIVATE
		FILE_SET CXX_MODULES
		BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
		FILES ${HR_MODULES}
)

add_subdirectory(tools)
