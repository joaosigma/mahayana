cmake_minimum_required(VERSION 3.17.3)

target_sources(horseradish
	PRIVATE
		engine.cpp
		engine.hpp
		engineRender.cpp
		logger.cpp
		logger.hpp
		profiler.cpp
		profiler.hpp
		runtime.cpp
		runtime.hpp
)
