#pragma once

#include "common\Mesh.hpp"
#include "common\MeshFactory.hpp"
#include "common\Stream.hpp"

using namespace HorseRadish;

Geometry::Model* mfRead3DS(HorseRadish::Streams::StreamReader &streamReader);
Geometry::Model* mfReadOBJ(HorseRadish::Streams::StreamReader &streamReader);
