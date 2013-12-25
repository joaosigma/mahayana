#pragma once
#ifndef __MESH_FACTORY__
#define __MESH_FACTORY__

#include "common\Mesh.hpp"
#include "common\MeshFactory.hpp"
#include "common\Stream.hpp"

using namespace HorseRadish;

Geometry::Model* mfReadCollada(HorseRadish::Streams::StreamReader * const streamReader);
Geometry::Model* mfRead3DS(HorseRadish::Streams::StreamReader * const streamReader);
Geometry::Model* mfReadOBJ(HorseRadish::Streams::StreamReader * const streamReader);

#endif