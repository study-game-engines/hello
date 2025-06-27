#pragma once
#include "HellTypes.h"
#include <vector>

struct CloudPoint {
    glm::vec4 position = glm::vec4(0);
    glm::vec4 normal = glm::vec4(0);
    glm::vec4 directLighting = glm::vec4(0);
};

namespace GlobalIllumination {
    void Update();
    void SetGlobalIlluminationStructuresDirtyState(bool state);
    void SetPointCloudNeedsGpuUpdateState(bool state);

    bool GlobalIlluminationStructuresAreDirty();
    bool PointCloudNeedsGpuUpdate();

    uint64_t GetSceneBvhId();
    const std::vector<BvhNode>& GetSceneNodes();
    std::vector<CloudPoint>& GetPointClound();
}