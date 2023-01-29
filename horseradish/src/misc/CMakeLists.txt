cmake_minimum_required(VERSION 3.17.3)

set(HR_SOURCES
	videoStream.cpp
	videoStream.hpp
)

target_sources(horseradish PRIVATE ${HR_SOURCES})
