#pragma once
#include "HellTypes.h"
#include "CreateInfo.h"
#include "glm/glm.hpp"
#include "Types/WallSegment.h"
#include <vector>

struct Wall {
    void Init(WallCreateInfo createInfo);
    void CleanUp();
    void CreateVertexData();

    Material* GetMaterial()                                 { return m_material; };
    const std::vector<WallSegment>& GetWallSegments() const { return m_wallSegments; }

private:
    float m_height = 2.4f;
    float m_textureScale = 1.0f;
    float m_textureOffsetX = 0.0f;
    float m_textureOffsetY = 0.0f;
    Material* m_material = nullptr;
    std::vector<glm::vec3> m_points;
    std::vector<WallSegment> m_wallSegments;
};