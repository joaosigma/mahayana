cmake_minimum_required(VERSION 4.2.1)

set(HR_HEADERS
	types.hpp
)

set(HR_SOURCES
)

set(HR_MODULES
	core.cpp

	avltree.cppm
	bareas.cpp
	bareas.cppm
	bvolumes.cppm
	color.cppm
	ecs.cppm
	encoders.cpp
	encoders.cppm
	fileSystem.cpp
	fileSystem.cppm
	hashing.cpp
	hashing.cppm
	image.cppm
	imageFactory.cpp
	imageFactory.cppm
	imageFormats.cppm
	logger.cpp
	logger.cppm
	math.cpp
	math.cppm
	matrix.cpp
	matrix.cppm
	mesh.cpp
	mesh.cppm
	meshAnim.cpp
	meshAnim.cppm
	meshFactory.cpp
	meshFactory.cppm
	plane.cppm
	platform.cppm
	primitives2D.cppm
	quaternion.cpp
	quaternion.cppm
	random.cpp
	random.cppm
	ray.cppm
	scopedAction.cppm
	serialize.cppm
	serializeArchives.cppm
	serializeSupport.cppm
	sorting.cpp
	sorting.cppm
	sparseMap.cppm
	sparseSet.cppm
	stream.cpp
	stream.cppm
	stringUtils.cpp
	stringUtils.cppm
	taggedPointer.cppm
	tasks.cpp
	tasks.cppm
	timer.cpp
	timer.cppm
	triangle.cppm
	types.cppm
	vector.cppm
)

if (BUILD_ENV STREQUAL "windows")

	list(APPEND HR_MODULES
		platformWin64.cpp
	)

endif()

target_sources(horseradish
	PRIVATE
		FILE_SET HEADERS
		FILES ${HR_HEADERS}
	PRIVATE
		${HR_SOURCES}
	PRIVATE
		FILE_SET CXX_MODULES
		BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
		FILES ${HR_MODULES}
)

if (BUILD_TESTING)
	target_sources(horseradish-utests
		PRIVATE
			FILE_SET HEADERS
			FILES ${HR_HEADERS}
		PRIVATE
			${HR_SOURCES}
			math_t.cpp
			matrix_t.cpp
			quaternion_t.cpp
			triangle_t.cpp
			vector_t.cpp
		PRIVATE
			FILE_SET CXX_MODULES
			BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
			FILES
				${HR_MODULES}
				utils_t.cppm
	)
endif()
