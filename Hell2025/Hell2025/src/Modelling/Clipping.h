#pragma once
#include "API/OpenGL/Types/GL_detachedMesh.hpp"
#include "Modelling/Types/ClippingCube.h";
#include "Modelling/Types/WallSegment.h";

namespace Clipping {
    void Init();
    void Update();
    void SubtractCubesFromWallSegment(WallSegment& wallSegment, std::vector<ClippingCube>& clippingCubes, float texOffsetX, float texOffsetY, float texScale, std::vector<Vertex>& verticesOut, std::vector<uint32_t>& indicesOut);
    OpenGLDetachedMesh& GetMesh();
}