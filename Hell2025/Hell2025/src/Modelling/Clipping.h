#pragma once
#include "Modelling/Types/ClippingCube.h";
#include "Modelling/Types/WallSegment.h";

namespace Clipping {
    void SubtractCubesFromWallSegment(WallSegment& wallSegment, std::vector<ClippingCube>& clippingCubes, std::vector<Vertex>& verticesOut, std::vector<uint32_t>& indicesOut);
}