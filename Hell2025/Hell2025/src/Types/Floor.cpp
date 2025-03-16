#include "Floor.h"
#include "AssetManagement/AssetManager.h"
#include "Input/Input.h"
#include "Renderer/Renderer.h"
#include "Util/Util.h"

void Floor::InitFromPoints(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3) {
    m_p0 = p0;
    m_p1 = p1;
    m_p2 = p2;
    m_p3 = p3;
}

void Floor::SetMaterial(const std::string& materialName) {
    m_material = AssetManager::GetMaterialByName(materialName);
}

void Floor::UpdateRenderItem(int globalBaseVertex, int globalBaseIndex) {
    m_vertices.clear();
    m_indices.clear();

    // Vertices
    m_vertices.push_back(m_p0);
    m_vertices.push_back(m_p1);
    m_vertices.push_back(m_p2);
    m_vertices.push_back(m_p3);

    // Indices
    m_indices = { 0, 1, 2, 2, 3, 0 };

    // Update UVs
    float textureScale = 0.5f;
    float textureOffsetX = 0.0f;
    float textureOffsetY = 0.0f;
    for (Vertex& vertex : m_vertices) {
        glm::vec3 origin = glm::vec3(0, 0, 0);
        origin = glm::vec3(0);
        vertex.uv = Util::CalculateUV(vertex.position, glm::vec3(0.0f, 1.0f, 0.0f));
        vertex.uv *= textureScale;
        vertex.uv.x += textureOffsetX;
        vertex.uv.y += textureOffsetY;
    }

    // Update normals and tangents
    for (int i = 0; i < m_indices.size(); i += 3) {
        Vertex& v0 = m_vertices[m_indices[i + 0]];
        Vertex& v1 = m_vertices[m_indices[i + 1]];
        Vertex& v2 = m_vertices[m_indices[i + 2]];
        Util::SetNormalsAndTangentsFromVertices(v0, v1, v2);
    }

    // Handle missing material
    if (!m_material) {
        m_material = AssetManager::GetDefaultMaterial();
    }

    // Render Item    
    m_renderItem.baseColorTextureIndex = m_material->m_basecolor;
    m_renderItem.normalMapTextureIndex = m_material->m_normal;
    m_renderItem.rmaTextureIndex = m_material->m_rma;
    m_renderItem.baseVertex = globalBaseVertex;
    m_renderItem.baseIndex = globalBaseIndex;
    m_renderItem.vertexCount = m_vertices.size();
    m_renderItem.indexCount = m_indices.size();
    m_renderItem.aabbMin = glm::vec4(0); // TODO
    m_renderItem.aabbMax = glm::vec4(0); // TODO
}