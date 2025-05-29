#pragma once
#include "Types/Renderer/MeshBuffer.h"

namespace AStarMap {
    void Update();
    void UpdateDebugMeshesFromHeightField();

    int GetMapWidth();
    int GetMapHeight();
    MeshBuffer& GetDebugGridMeshBuffer(); 
    MeshBuffer& GetDebugSolidMeshBuffer();
}