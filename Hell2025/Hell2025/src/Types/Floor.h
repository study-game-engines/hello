#pragma once
#include "HellTypes.h"

struct Floor {
    void InitFromPoints(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3);
    void UpdateRenderItem(int globalBaseVertex, int globalBaseIndex);
    void SetMaterial(const std::string& materialName);

    Material* GetMaterial()                 { return m_material; };
    std::vector<Vertex>& GetVertices()      { return m_vertices; }
    std::vector<uint32_t>& GetIndices()     { return m_indices; }
    HouseRenderItem& GetRenderItem()        { return m_renderItem; };

private:
    Material* m_material = nullptr;
    glm::vec3 m_p0 = glm::vec3(0.0f);
    glm::vec3 m_p1 = glm::vec3(0.0f);
    glm::vec3 m_p2 = glm::vec3(0.0f);
    glm::vec3 m_p3 = glm::vec3(0.0f);
    std::vector<Vertex> m_vertices;
    std::vector<uint32_t> m_indices;
    HouseRenderItem m_renderItem;
};