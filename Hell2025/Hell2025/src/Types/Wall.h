#pragma once
#include "HellTypes.h"
#include "CreateInfo.h"
#include "glm/glm.hpp"
#include "Modelling/Types/WallSegment.h"
#include <vector>

struct Wall {
    void Init(WallCreateInfo createInfo);
    void DebugDraw();
    void UpdateRenderItems(int globalBaseVertex, int globalBaseIndex);
    //void SetMaterial(const std::string& materialName);
    //void SetTextureOffset(float x, float y);
    //void SetTextureScale(float scale);

    Material* GetMaterial()                                 { return m_material; };
    std::vector<Vertex>& GetVertices()                      { return m_vertices; }
    std::vector<uint32_t>& GetIndices()                     { return m_indices; }
    std::vector<HouseRenderItem>& GetRenderItems()          { return m_renderItems; }

private:
    float m_height = 2.4f;
    float m_textureScale = 1.0f;
    float m_textureOffsetX = 0.0f;
    float m_textureOffsetY = 0.0f;
    Material* m_material = nullptr;
    std::vector<Vertex> m_vertices;
    std::vector<uint32_t> m_indices;
    std::vector<glm::vec3> m_points;
    std::vector<WallSegment> m_wallSegments;
    std::vector<HouseRenderItem> m_renderItems;
};