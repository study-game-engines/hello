#pragma once
#include "glm/glm.hpp"
#include "Math/AABB.h"
#include <vector>

struct WallSegment {
    void Update(glm::vec3 start, glm::vec3 end, float height);

    const glm::vec3& GetStart()                 const { return m_start; }
    const glm::vec3& GetEnd()                   const { return m_end; }
    const glm::vec3& GetNormal()                const { return m_normal; }
    const float GetHeight()                     const { return m_height; }
    const AABB& GetAABB()                       const { return m_aabb; }
    const std::vector<glm::vec3>& GetCorners()  const { return m_corners; }

private:
    glm::vec3 m_start;
    glm::vec3 m_end;
    glm::vec3 m_normal;
    float m_height = 2.4f;
    AABB m_aabb;
    std::vector<glm::vec3> m_corners;
};