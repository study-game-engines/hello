#pragma once
#include "HellTypes.h"
#include <vector>

struct CloudPoint {
    glm::vec4 position = glm::vec4(0);
    glm::vec4 normal = glm::vec4(0);
    glm::vec4 directLighting = glm::vec4(0);
};

namespace GlobalIllumination {
    void CreatePointCloud();

    void Update();

    std::vector<CloudPoint>& GetPointClound();
}