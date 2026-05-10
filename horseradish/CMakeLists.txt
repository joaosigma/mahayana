cmake_minimum_required(VERSION 4.2.1 FATAL_ERROR)

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

set(BUILD_ENV "windows" CACHE STRING "The target build environment")
if (NOT BUILD_ENV STREQUAL "windows")
	message(FATAL_ERROR "Only \"windows\" build environment is currently supported (value: '${BUILD_ENV}')")
endif()

set(HR_ENABLE_DEVEL OFF CACHE BOOL "Enables generic developer / debugging features")
set(HR_ENABLE_LOGGING OFF CACHE BOOL "Enables the logger and logging messages")
set(HR_ENABLE_PROFILLING OFF CACHE BOOL "Enables the profiler in the engine, otherwise, all features related to profiling are discarded")

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "451f2fe2-a8a2-47c3-bc32-94786d8fc91b")
# set(CMAKE_CXX_MODULE_STD ON)
# set(CMAKE_CXX_SCAN_FOR_MODULES ON)

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

configure_file (
    "${CMAKE_CURRENT_LIST_DIR}/src/build.hpp.in"
    "${CMAKE_CURRENT_LIST_DIR}/src/build.hpp"
    @ONLY
    NEWLINE_STYLE UNIX
)

#######################
### the project

project(mustard VERSION 0.9 LANGUAGES C CXX)

include(CTest)

add_executable(horseradish WIN32 "")
target_compile_features(horseradish PUBLIC c_std_23)
target_compile_features(horseradish PUBLIC cxx_std_23)

if (BUILD_TESTING)
	add_executable(horseradish-utests "")
	target_compile_features(horseradish-utests PUBLIC c_std_23)
	target_compile_features(horseradish-utests PUBLIC cxx_std_23)

	add_test(NAME "horseradish unit tests" COMMAND horseradish-utests)
endif()

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

if (BUILD_ENV STREQUAL "windows")

	set(HR_COMPILER_DEFS

		WINVER=0x0A00 # Windows 10
		_WIN32_WINNT=0x0A00 # Windows 10
		NTDDI_VERSION=0x0A000007 # NTDDI_WIN10_19H1

		_UNICODE #default Windows APIs to Unicode
		UNICODE

		WIN32_LEAN_AND_MEAN
		VC_EXTRALEAN

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

		_MSVC_STL_HARDENING=1
		_MSVC_STL_DESTRUCTOR_TOMBSTONES=1

		_CRT_SECURE_NO_WARNINGS
		#_SCL_SECURE_NO_WARNINGS
		_WINSOCK_DEPRECATED_NO_WARNINGS
	)

	target_compile_definitions(horseradish PRIVATE ${HR_COMPILER_DEFS})

	if (BUILD_TESTING)
		target_compile_definitions(horseradish-utests PRIVATE ${HR_COMPILER_DEFS})
	endif()

endif()

if (CMAKE_GENERATOR MATCHES "^Visual Studio")

	set(HR_COMPILER_OPTIONS

		/GR- # disable RTTI
		/utf-8
		/permissive- # make sure we keep up with standards conformance
		/W4 # warning level
		#/WX # treat warnings as errors
		#/MP # enable parallel builds
		#/Bv # print toolchain info
		#/Bt # print time spent in the compiler

		/arch:AVX2
	)

	target_compile_options(horseradish PRIVATE ${HR_COMPILER_OPTIONS})
	target_link_options(horseradish PRIVATE "$<$<C_COMPILER_ID:MSVC>:-SAFESEH:NO>")

	if (BUILD_TESTING)
		target_compile_options(horseradish-utests PRIVATE ${HR_COMPILER_OPTIONS})
		target_link_options(horseradish-utests PRIVATE "$<$<C_COMPILER_ID:MSVC>:-SAFESEH:NO>")
	endif()

endif()

# libs definitions
add_definitions(-DLODEPNG_NO_COMPILE_DISK -DLODEPNG_NO_COMPILE_ERROR_TEXT) #lodepng
set(nlohmann-json_IMPLICIT_CONVERSIONS OFF) #nlohmann

#######################
### the sub-projects

set(CMAKE_SUPPRESS_REGENERATION ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_subdirectory(src)

target_include_directories(horseradish PUBLIC ${CMAKE_CURRENT_LIST_DIR}/src)
target_include_directories(horseradish PUBLIC ${CMAKE_CURRENT_LIST_DIR}/external_includes/gl)
target_include_directories(horseradish PUBLIC ${CMAKE_CURRENT_LIST_DIR}/external_includes/ffmpeg)

if (BUILD_TESTING)
	target_include_directories(horseradish-utests PUBLIC ${CMAKE_CURRENT_LIST_DIR}/src)
	target_include_directories(horseradish-utests PUBLIC ${CMAKE_CURRENT_LIST_DIR}/external_includes/gl)
	target_include_directories(horseradish-utests PUBLIC ${CMAKE_CURRENT_LIST_DIR}/external_includes/ffmpeg)
endif()

find_package(PkgConfig REQUIRED)
pkg_check_modules(hr_pkg_deps REQUIRED IMPORTED_TARGET libcoro)

find_package(lodepng CONFIG REQUIRED)
find_package(ZLIB REQUIRED)
find_package(unofficial-minizip CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(Stb REQUIRED)
find_path(TINYGLTF_INCLUDE_DIRS "tiny_gltf.h")
find_package(tinyexr CONFIG REQUIRED)
find_package(tinyobjloader CONFIG REQUIRED)
find_package(meshoptimizer CONFIG REQUIRED)
find_package(mikktspace CONFIG REQUIRED)
find_package(tsl-hopscotch-map CONFIG REQUIRED)
find_package(unofficial-libsquish CONFIG REQUIRED)
find_package(Catch2 CONFIG REQUIRED)
find_package(murmurhash CONFIG REQUIRED)

target_link_libraries(horseradish PRIVATE PkgConfig::hr_pkg_deps) #libcoro
target_link_libraries(horseradish PRIVATE lodepng)
target_link_libraries(horseradish PRIVATE ZLIB::ZLIB)
target_link_libraries(horseradish PRIVATE unofficial::minizip::minizip)
target_link_libraries(horseradish PRIVATE nlohmann_json::nlohmann_json)
target_include_directories(horseradish PRIVATE ${Stb_INCLUDE_DIR})
target_include_directories(horseradish PRIVATE ${TINYGLTF_INCLUDE_DIRS})
target_link_libraries(horseradish PRIVATE unofficial::tinyexr::tinyexr)
target_link_libraries(horseradish PRIVATE tinyobjloader::tinyobjloader)
target_link_libraries(horseradish PRIVATE meshoptimizer::meshoptimizer)
target_link_libraries(horseradish PRIVATE mikktspace::mikktspace)
target_link_libraries(horseradish PRIVATE tsl::hopscotch_map)
target_link_libraries(horseradish PRIVATE unofficial::libsquish::squish)
target_link_libraries(horseradish PRIVATE murmurhash::murmurhash)

if (BUILD_TESTING)
	target_link_libraries(horseradish-utests PRIVATE PkgConfig::hr_pkg_deps) #libcoro
	target_link_libraries(horseradish-utests PRIVATE lodepng)
	target_link_libraries(horseradish-utests PRIVATE ZLIB::ZLIB)
	target_link_libraries(horseradish-utests PRIVATE unofficial::minizip::minizip)
	target_link_libraries(horseradish-utests PRIVATE nlohmann_json::nlohmann_json)
	target_include_directories(horseradish-utests PRIVATE ${Stb_INCLUDE_DIR})
	target_include_directories(horseradish-utests PRIVATE ${TINYGLTF_INCLUDE_DIRS})
	target_link_libraries(horseradish-utests PRIVATE unofficial::tinyexr::tinyexr)
	target_link_libraries(horseradish-utests PRIVATE tinyobjloader::tinyobjloader)
	target_link_libraries(horseradish-utests PRIVATE meshoptimizer::meshoptimizer)
	target_link_libraries(horseradish-utests PRIVATE mikktspace::mikktspace)
	target_link_libraries(horseradish-utests PRIVATE tsl::hopscotch_map)
	target_link_libraries(horseradish-utests PRIVATE unofficial::libsquish::squish)
	target_link_libraries(horseradish-utests PRIVATE murmurhash::murmurhash)

	target_link_libraries(horseradish-utests PRIVATE Catch2::Catch2 Catch2::Catch2WithMain)
endif()

if (BUILD_ENV STREQUAL "windows")

	set_target_properties(horseradish PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/../sandbox/bin")

	target_link_libraries(horseradish PRIVATE ${CMAKE_CURRENT_LIST_DIR}/external_libs/ffmpeg_x64/avcodec.lib)
	target_link_libraries(horseradish PRIVATE ${CMAKE_CURRENT_LIST_DIR}/external_libs/ffmpeg_x64/avformat.lib)
	target_link_libraries(horseradish PRIVATE ${CMAKE_CURRENT_LIST_DIR}/external_libs/ffmpeg_x64/avutil.lib)
	target_link_libraries(horseradish PRIVATE ${CMAKE_CURRENT_LIST_DIR}/external_libs/ffmpeg_x64/swscale.lib)

	if (BUILD_TESTING)
		set_target_properties(horseradish-utests PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/../sandbox/bin")

		target_link_libraries(horseradish-utests PRIVATE ${CMAKE_CURRENT_LIST_DIR}/external_libs/ffmpeg_x64/avcodec.lib)
		target_link_libraries(horseradish-utests PRIVATE ${CMAKE_CURRENT_LIST_DIR}/external_libs/ffmpeg_x64/avformat.lib)
		target_link_libraries(horseradish-utests PRIVATE ${CMAKE_CURRENT_LIST_DIR}/external_libs/ffmpeg_x64/avutil.lib)
		target_link_libraries(horseradish-utests PRIVATE ${CMAKE_CURRENT_LIST_DIR}/external_libs/ffmpeg_x64/swscale.lib)
	endif()

endif()

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT horseradish)

#######################
### final configs

# just to make things pretty in IDEs

get_property(HR_SOURCES TARGET horseradish PROPERTY SOURCES)
get_property(HR_HEADERS TARGET horseradish PROPERTY HEADER_SET)
get_property(HR_MODULES TARGET horseradish PROPERTY CXX_MODULE_SET)
source_group(TREE "${CMAKE_CURRENT_LIST_DIR}/src" FILES ${HR_SOURCES} ${HR_HEADERS} ${HR_MODULES}) # just to make things pretty in IDEs

if (BUILD_TESTING)
	get_property(HR_SOURCES TARGET horseradish-utests PROPERTY SOURCES)
	get_property(HR_HEADERS TARGET horseradish-utests PROPERTY HEADER_SET)
	get_property(HR_MODULES TARGET horseradish-utests PROPERTY CXX_MODULE_SET)
	source_group(TREE "${CMAKE_CURRENT_LIST_DIR}/src" FILES ${HR_SOURCES} ${HR_HEADERS} ${HR_MODULES}) # just to make things pretty in IDEs
endif()
