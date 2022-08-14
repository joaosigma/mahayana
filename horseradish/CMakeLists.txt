cmake_minimum_required(VERSION 3.17.3 FATAL_ERROR)

#######################
### declare all vars and global options

get_property(BUILD_IS_MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)

if (NOT BUILD_IS_MULTI_CONFIG AND NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
	message(WARNING "[WARN] CMAKE_BUILD_TYPE not specified: using ${CMAKE_BUILD_TYPE} by default")
endif()

if (BUILD_IS_MULTI_CONFIG)
	set(CMAKE_CONFIGURATION_TYPES "Debug" "Release" CACHE STRING "" FORCE) # limit configuration types to Debug and Release
endif()

set(HR_ENABLE_DEVEL OFF CACHE BOOL "Enables generic developer / debugging features")
set(HR_ENABLE_LOGGING OFF CACHE BOOL "Enables the logger and logging messages")
set(HR_ENABLE_PROFILLING OFF CACHE BOOL "Enables the profiler in the engine, otherwise, all features related to profilling are discarded")

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

#######################
### check some stuff

if (CMAKE_GENERATOR MATCHES "^Visual Studio")

	if (NOT ("${CMAKE_GENERATOR_PLATFORM}" STREQUAL "Win32" OR "${CMAKE_GENERATOR_PLATFORM}" STREQUAL "x64"))

		message(FATAL_ERROR "For Windows builds, only \"Win32\" and \"x64\" architectures are supported")

	endif()

endif()

#######################
### configure / update some files

execute_process(
	COMMAND git rev-parse --verify HEAD
	WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
	OUTPUT_VARIABLE HR_BUILD_HASH
	OUTPUT_STRIP_TRAILING_WHITESPACE
	COMMAND_ERROR_IS_FATAL ANY
)
execute_process(
	COMMAND git rev-parse --short --verify HEAD
	WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
	OUTPUT_VARIABLE HR_BUILD_HASH_SHORT
	OUTPUT_STRIP_TRAILING_WHITESPACE
	COMMAND_ERROR_IS_FATAL ANY
)

configure_file (
    "${CMAKE_CURRENT_LIST_DIR}/src/build.hpp.in"
    "${CMAKE_CURRENT_LIST_DIR}/src/build.hpp"
    @ONLY
    NEWLINE_STYLE UNIX
)

#######################
### the project

project(Mustard VERSION 0.9 LANGUAGES C CXX)

set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(horseradish WIN32 "")

target_compile_features(horseradish PRIVATE c_std_11)
target_compile_features(horseradish PRIVATE cxx_std_20)

if (HR_ENABLE_DEVEL)
	target_compile_definitions(horseradish PRIVATE HR_DEVEL)
endif()
if (HR_ENABLE_LOGGING)
	target_compile_definitions(horseradish PRIVATE HR_LOGGING)
endif()
if (HR_ENABLE_PROFILLING)
	target_compile_definitions(horseradish PRIVATE HR_PROFILLING)
endif()

# compiler specific

if (CMAKE_GENERATOR MATCHES "^Visual Studio")
	target_compile_definitions(horseradish
		PRIVATE
			WINVER=0x0601 # Windows 7
			_WIN32_WINNT=0x0601 # Windows 7
			NTDDI_VERSION=0x06010000

			_UNICODE #default Windows APIs to Unicode
			UNICODE

			WIN32_LEAN_AND_MEAN
			VC_EXTRALEAN

			NOMINMAX
			NOGDICAPMASKS
			NOMENUS
			NOICONS
			NOSYSCOMMANDS
			NORASTEROPS
			NOSHOWWINDOW
			OEMRESOURCE
			NOATOM
			NOCOLOR
			NOCTLMGR
			NODRAWTEXT
			NOMEMMGR
			NOMETAFILE
			NOMINMAX
			NOOPENFILE
			NOSCROLL
			NOSERVICE
			NOSOUND
			NOTEXTMETRIC
			NOWH
			NOCOMM
			NOKANJI
			NOHELP
			NOPROFILER
			NODEFERWINDOWPOS
			NOMCX

			_ITERATOR_DEBUG_LEVEL=0

			_CRT_SECURE_NO_WARNINGS
			#_SCL_SECURE_NO_WARNINGS
			_WINSOCK_DEPRECATED_NO_WARNINGS
	)

	target_compile_options(horseradish
		PRIVATE
			/GR- # disable RTTI
			/utf-8
			/permissive- # make sure we keep up with standards conformance
			/W4 # warning level
			#/WX # treat warnings as errors
			/MP # enable parallel builds
			/Bv # print toolchain info
			#/Bt # print time spent in the compiler

			/arch:AVX
	)

	target_link_options(horseradish PRIVATE "$<$<C_COMPILER_ID:MSVC>:-SAFESEH:NO>")
endif()

#######################
### the sub-projects

set(CMAKE_SUPPRESS_REGENERATION ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_subdirectory(src)

target_include_directories(horseradish PUBLIC ${CMAKE_CURRENT_LIST_DIR}/src)
target_include_directories(horseradish PUBLIC ${CMAKE_CURRENT_LIST_DIR}/external_includes/gl)
target_include_directories(horseradish PUBLIC ${CMAKE_CURRENT_LIST_DIR}/external_includes/ffmpeg)

if (CMAKE_GENERATOR MATCHES "^Visual Studio")

	if ("${CMAKE_GENERATOR_PLATFORM}" STREQUAL "Win32")

		set_target_properties(horseradish PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/../sandbox/bin_x86")
		
		target_link_libraries(horseradish PRIVATE ${CMAKE_CURRENT_LIST_DIR}/external_libs/x86/ffmpeg/avcodec.lib)
		target_link_libraries(horseradish PRIVATE ${CMAKE_CURRENT_LIST_DIR}/external_libs/x86/ffmpeg/avformat.lib)
		target_link_libraries(horseradish PRIVATE ${CMAKE_CURRENT_LIST_DIR}/external_libs/x86/ffmpeg/avutil.lib)
		target_link_libraries(horseradish PRIVATE ${CMAKE_CURRENT_LIST_DIR}/external_libs/x86/ffmpeg/swscale.lib)

	else ()

		set_target_properties(horseradish PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/../sandbox/bin_x64")

		target_link_libraries(horseradish PRIVATE ${CMAKE_CURRENT_LIST_DIR}/external_libs/x64/ffmpeg/avcodec.lib)
		target_link_libraries(horseradish PRIVATE ${CMAKE_CURRENT_LIST_DIR}/external_libs/x64/ffmpeg/avformat.lib)
		target_link_libraries(horseradish PRIVATE ${CMAKE_CURRENT_LIST_DIR}/external_libs/x64/ffmpeg/avutil.lib)
		target_link_libraries(horseradish PRIVATE ${CMAKE_CURRENT_LIST_DIR}/external_libs/x64/ffmpeg/swscale.lib)

	endif()

endif()

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT horseradish)

get_property(HR_SOURCES TARGET horseradish PROPERTY SOURCES)
source_group(TREE "${CMAKE_CURRENT_LIST_DIR}/src" FILES ${HR_SOURCES}) # just to make things pretty in IDEs
