#pragma once
#include "Types/Renderer/MeshBuffer.h"

namespace AStarMap {
    void Update();
    void UpdateDebugMeshesFromHeightField();

    MeshBuffer& GetDebugGridMeshBuffer(); 
    MeshBuffer& GetDebugSolidMeshBuffer();
}