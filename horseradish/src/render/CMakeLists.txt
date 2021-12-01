cmake_minimum_required(VERSION 3.17.3)

target_sources(horseradish
	PRIVATE
		consoleUI.cpp
		consoleUI.hpp
		profilerUI.cpp
		profilerUI.hpp
		renderer.hpp
		renderer2D.cpp
		renderer2D.hpp
		rendererDebug.cpp
		rendererDebug.hpp
		rendererMain.cpp
		rendererMain.hpp
		scene.cpp
		scene.hpp
		stage.cpp
		stage.hpp
		world.cpp
		world.hpp
		worldEditor.cpp
		worldEditor.hpp
)

add_subdirectory(tools)
