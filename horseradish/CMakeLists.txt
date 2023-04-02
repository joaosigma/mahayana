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

	if (NOT "${CMAKE_GENERATOR_PLATFORM}" STREQUAL "x64")
		message(FATAL_ERROR "Only \"x64\" builds are supported")
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

configure_file(
    "${CMAKE_CURRENT_LIST_DIR}/src/build.hpp.in"
    "${CMAKE_CURRENT_LIST_DIR}/src/build.hpp"
    @ONLY
    NEWLINE_STYLE UNIX
)

#######################
### the project

project(Mustard VERSION 0.9 LANGUAGES C CXX)

add_executable(horseradish WIN32 "")
add_executable(horseradish-utests "")

target_compile_features(horseradish PRIVATE c_std_11)
target_compile_features(horseradish PRIVATE cxx_std_20)

target_compile_features(horseradish-utests PRIVATE c_std_11)
target_compile_features(horseradish-utests PRIVATE cxx_std_20)

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

if (WIN32)

	set(HR_COMPILER_DEFS

		WINVER=0x0A00 # Windows 10
		_WIN32_WINNT=0x0A00 # Windows 10
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

	target_compile_definitions(horseradish PRIVATE ${HR_COMPILER_DEFS})
	target_compile_definitions(horseradish-utests PRIVATE ${HR_COMPILER_DEFS})

endif()

if (CMAKE_GENERATOR MATCHES "^Visual Studio")

	set(HR_COMPILER_OPTIONS

		/GR- # disable RTTI
		/utf-8
		/permissive- # make sure we keep up with standards conformance
		/W4 # warning level
		#/WX # treat warnings as errors
		/MP # enable parallel builds
		/Bv # print toolchain info
		#/Bt # print time spent in the compiler

		/arch:AVX2
	)

	target_compile_options(horseradish PRIVATE ${HR_COMPILER_OPTIONS})
	target_link_options(horseradish PRIVATE "$<$<C_COMPILER_ID:MSVC>:-SAFESEH:NO>")

	target_compile_options(horseradish-utests PRIVATE ${HR_COMPILER_OPTIONS})
	target_link_options(horseradish-utests PRIVATE "$<$<C_COMPILER_ID:MSVC>:-SAFESEH:NO>")

endif()

#######################
### the sub-projects

set(CMAKE_SUPPRESS_REGENERATION ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_subdirectory(src)

#######################
### external dependencies (includes and libs)

target_include_directories(horseradish PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src)
target_include_directories(horseradish PRIVATE ${CMAKE_CURRENT_LIST_DIR}/external_includes/gl)
target_include_directories(horseradish PRIVATE ${CMAKE_CURRENT_LIST_DIR}/external_includes/ffmpeg)

target_include_directories(horseradish-utests PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src)
target_include_directories(horseradish-utests PRIVATE ${CMAKE_CURRENT_LIST_DIR}/external_includes/gl)
target_include_directories(horseradish-utests PRIVATE ${CMAKE_CURRENT_LIST_DIR}/external_includes/ffmpeg)

if (WIN32)

	set_target_properties(horseradish PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/../sandbox/bin")

	target_link_libraries(horseradish PRIVATE ${CMAKE_CURRENT_LIST_DIR}/external_libs/ffmpeg_x64/avcodec.lib)
	target_link_libraries(horseradish PRIVATE ${CMAKE_CURRENT_LIST_DIR}/external_libs/ffmpeg_x64/avformat.lib)
	target_link_libraries(horseradish PRIVATE ${CMAKE_CURRENT_LIST_DIR}/external_libs/ffmpeg_x64/avutil.lib)
	target_link_libraries(horseradish PRIVATE ${CMAKE_CURRENT_LIST_DIR}/external_libs/ffmpeg_x64/swscale.lib)

	set_target_properties(horseradish-utests PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/../sandbox/bin")

	target_link_libraries(horseradish-utests PRIVATE ${CMAKE_CURRENT_LIST_DIR}/external_libs/ffmpeg_x64/avcodec.lib)
	target_link_libraries(horseradish-utests PRIVATE ${CMAKE_CURRENT_LIST_DIR}/external_libs/ffmpeg_x64/avformat.lib)
	target_link_libraries(horseradish-utests PRIVATE ${CMAKE_CURRENT_LIST_DIR}/external_libs/ffmpeg_x64/avutil.lib)
	target_link_libraries(horseradish-utests PRIVATE ${CMAKE_CURRENT_LIST_DIR}/external_libs/ffmpeg_x64/swscale.lib)

endif()

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT horseradish)

#######################
### external dependencies (modules)

find_package(Vulkan REQUIRED)

if (NOT Vulkan_FOUND)
	message(FATAL_ERROR "Unable to find Vulkan library. Does the VULKAN_SDK environment variable point to the root of the Vulkan SDK?")
endif()

target_include_directories(horseradish PRIVATE ${Vulkan_INCLUDE_DIR})
target_include_directories(horseradish-utests PRIVATE ${Vulkan_INCLUDE_DIR})

target_link_libraries(horseradish PRIVATE ${Vulkan_LIBRARY})
target_link_libraries(horseradish-utests PRIVATE ${Vulkan_LIBRARY})

configure_file(
    "${CMAKE_CURRENT_LIST_DIR}/src/common/vulkan/build_tools.hpp.in"
    "${CMAKE_CURRENT_LIST_DIR}/src/common/vulkan/build_tools.hpp"
    @ONLY
    NEWLINE_STYLE UNIX
)

#######################
### final configs

# enable pre-compiled headers

if (${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.16.3")

	target_precompile_headers(horseradish

		PRIVATE

		"$<$<COMPILE_LANGUAGE:CXX>:<vulkan/vulkan.h$<ANGLE-R>>"

		"$<$<COMPILE_LANGUAGE:CXX>:<algorithm$<ANGLE-R>>"
		"$<$<COMPILE_LANGUAGE:CXX>:<array$<ANGLE-R>>"
		"$<$<COMPILE_LANGUAGE:CXX>:<atomic$<ANGLE-R>>"
		"$<$<COMPILE_LANGUAGE:CXX>:<cstring$<ANGLE-R>>"
		"$<$<COMPILE_LANGUAGE:CXX>:<format$<ANGLE-R>>"
		"$<$<COMPILE_LANGUAGE:CXX>:<functional$<ANGLE-R>>"
		"$<$<COMPILE_LANGUAGE:CXX>:<map$<ANGLE-R>>"
		"$<$<COMPILE_LANGUAGE:CXX>:<memory$<ANGLE-R>>"
		"$<$<COMPILE_LANGUAGE:CXX>:<mutex$<ANGLE-R>>"
		"$<$<COMPILE_LANGUAGE:CXX>:<optional$<ANGLE-R>>"
		"$<$<COMPILE_LANGUAGE:CXX>:<set$<ANGLE-R>>"
		"$<$<COMPILE_LANGUAGE:CXX>:<span$<ANGLE-R>>"
		"$<$<COMPILE_LANGUAGE:CXX>:<sstream$<ANGLE-R>>"
		"$<$<COMPILE_LANGUAGE:CXX>:<string$<ANGLE-R>>"
		"$<$<COMPILE_LANGUAGE:CXX>:<string_view$<ANGLE-R>>"
		"$<$<COMPILE_LANGUAGE:CXX>:<tuple$<ANGLE-R>>"
		"$<$<COMPILE_LANGUAGE:CXX>:<type_traits$<ANGLE-R>>"
		"$<$<COMPILE_LANGUAGE:CXX>:<unordered_map$<ANGLE-R>>"
		"$<$<COMPILE_LANGUAGE:CXX>:<unordered_set$<ANGLE-R>>"
		"$<$<COMPILE_LANGUAGE:CXX>:<utility$<ANGLE-R>>"
		"$<$<COMPILE_LANGUAGE:CXX>:<vector$<ANGLE-R>>"
	)

endif()

# just to make things pretty in IDEs

get_property(HR_SOURCES TARGET horseradish PROPERTY SOURCES)
source_group(TREE "${CMAKE_CURRENT_LIST_DIR}/src" FILES ${HR_SOURCES})

get_property(HR_SOURCES TARGET horseradish-utests PROPERTY SOURCES)
source_group(TREE "${CMAKE_CURRENT_LIST_DIR}/src" FILES ${HR_SOURCES}) # just to make things pretty in IDEs
