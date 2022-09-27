cmake_minimum_required(VERSION 3.17.3)

set(HR_SOURCES
	avl-tree.hpp
	bareas.cpp
	bareas.hpp
	bvolumes.hpp
	color.hpp
	ecs.hpp
	encoders.cpp
	encoders.hpp
	fileSystem.cpp
	fileSystem.hpp
	hashing.cpp
	hashing.hpp
	image.hpp
	imageFactory.cpp
	imageFactory.hpp
	imageFormats.hpp
	math.cpp
	math.hpp
	matrix.cpp
	matrix.hpp
	mesh.cpp
	mesh.hpp
	meshFactory.cpp
	meshFactory.hpp
	meshAnim.cpp
	meshAnim.hpp
	plane.hpp
	primitives2D.hpp
	quaternion.cpp
	quaternion.hpp
	random.cpp
	random.hpp
	ray.hpp
	scopedAction.hpp
	sorting.cpp
	sorting.hpp
	sparseMap.hpp
	sparseSet.hpp
	stream.cpp
	stream.hpp
	stringUtils.cpp
	stringUtils.hpp
	triangle.hpp
	taggedPointer.hpp
	timer.cpp
	timer.hpp
	types.cpp
	types.hpp
	vector.hpp
)

target_sources(horseradish PRIVATE ${HR_SOURCES})

target_sources(horseradish-utests PRIVATE ${HR_SOURCES})
target_sources(horseradish-utests
	PRIVATE
		vector_t.cpp
)

add_subdirectory(opengl)
