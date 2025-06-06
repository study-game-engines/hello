#pragma once
#include "HellTypes.h"
#include "CreateInfo.h"

struct ScreenSpaceBloodDecal {
    void Init(ScreenSpaceBloodDecalCreateInfo createInfo);

    glm::mat4 GetModelMatrix()          { return m_modelMatrix; }
    glm::mat4 GetInverseModelMatrix()   { return m_inverseModelMatrix; }

private:
    glm::mat4 m_modelMatrix = glm::mat4(1.0f);
    glm::mat4 m_inverseModelMatrix = glm::mat4(1.0f);
    int m_type = 0;
};