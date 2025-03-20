#pragma once
#include "HellTypes.h"

struct HousePlane {
    void InitFromPoints(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, float textureScale);
    void SetMaterial(const std::string& materialName);
    void CleanUp();

    Material* GetMaterial()                 { return m_material; };
    std::vector<Vertex>& GetVertices()      { return m_vertices; }
    std::vector<uint32_t>& GetIndices()     { return m_indices; }

private:
    uint64_t m_objectId = 0;
    uint64_t m_physicsId = 0;
    Material* m_material = nullptr;
    glm::vec3 m_p0 = glm::vec3(0.0f);
    glm::vec3 m_p1 = glm::vec3(0.0f);
    glm::vec3 m_p2 = glm::vec3(0.0f);
    glm::vec3 m_p3 = glm::vec3(0.0f);
    std::vector<Vertex> m_vertices;
    std::vector<uint32_t> m_indices;
    
    void CreatePhysicsObject();
};