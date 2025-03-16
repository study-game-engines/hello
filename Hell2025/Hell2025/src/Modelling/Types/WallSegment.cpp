#include "WallSegment.h"
#include "Util.h"

void WallSegment::Update(glm::vec3 start, glm::vec3 end, float height) {
    m_start = start;
    m_end = end;
    m_height = height;

    // Normal
    glm::vec3 dir = glm::normalize(m_end - m_start);
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    m_normal = glm::normalize(glm::cross(dir, up));

    // Corners
    m_corners = {
        m_start,                                    // Bottom-left
        m_start + glm::vec3 (0.0f, m_height, 0.0f), // Top-left
        m_end + glm::vec3 (0.0f, m_height, 0.0f),   // Top-right
        m_end                                       // Bottom-right
    };

    // AABB
    m_aabb = Util::GetAABBFromPoints(m_corners);
}