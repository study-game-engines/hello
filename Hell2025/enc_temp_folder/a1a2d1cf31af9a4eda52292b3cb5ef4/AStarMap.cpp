#include "AStarMap.h"
#include "BackEnd/BackEnd.h"
#include "Input/Input.h"
#include "Physics/Physics.h"

namespace AStarMap {

    MeshBuffer g_debugGridMeshBuffer;
    MeshBuffer g_debuGSolidMeshBuffer;
    glm::vec3 g_gridOffset = glm::vec3(10.0f, 35.0f, 10.0f);
    int quadCountX = 50;
    int quadCountY = 50;
    float spacing = 1.0f;

    void UpdateDebugGridMesh();
    void UpdateDebugSolidMesh();
    inline int Index1D(int x, int y, int mapWidth) { return y * mapWidth + x; }

    void Update() {
        if (Input::KeyPressed(HELL_KEY_SPACE)) {
            UpdateDebugMeshesFromHeightField();
        }
    }

    void UpdateDebugMeshesFromHeightField() {
        Physics::ActivateAllHeightFields();
        UpdateDebugGridMesh();
        UpdateDebugSolidMesh();
    }

    void UpdateDebugGridMesh() {
        int m_mapWidth = quadCountX + 1;
        int m_mapHeight = quadCountY + 1;

        std::vector<Vertex> vertices;
        vertices.reserve(m_mapWidth * m_mapHeight);
        for (int y = 0; y < m_mapHeight; ++y) {
            for (int x = 0; x < m_mapWidth; ++x) {
                Vertex vertex;
                vertex.position = glm::vec3(x * spacing, 0.0f, y * spacing) + g_gridOffset;
                vertex.uv = glm::vec2(x / float(quadCountX), y / float(quadCountY));
                glm::vec3 rayOrigin = vertex.position + glm::vec3(0, 100, 0);
                glm::vec3 rayDirection = glm::vec3(0, -1, 0);
                float rayLength = 1000;
                PhysXRayResult result = Physics::CastPhysXRayHeightMap(rayOrigin, rayDirection, rayLength);
                if (result.hitFound) {
                    vertex.position = result.hitPosition;
                }
                vertices.push_back(vertex);
            }
        }

        std::vector<uint32_t> indices;
        indices.reserve(((quadCountX + 1) * quadCountY + (quadCountY + 1) * quadCountX) * 2);
        // Vertical segments
        for (int x = 0; x <= quadCountX; ++x) {
            for (int y = 0; y < quadCountY; ++y) {
                indices.push_back(Index1D(x, y, m_mapWidth));
                indices.push_back(Index1D(x, y + 1, m_mapWidth));
            }
        }
        // Horizontal segments
        for (int y = 0; y <= quadCountY; ++y) {
            for (int x = 0; x < quadCountX; ++x) {
                indices.push_back(Index1D(x, y, m_mapWidth));
                indices.push_back(Index1D(x + 1, y, m_mapWidth));
            }
        }

        if (BackEnd::GetAPI() == API::OPENGL) {
            g_debugGridMeshBuffer.GetGLMeshBuffer().UpdateBuffers(vertices, indices);
        }
        else if (BackEnd::GetAPI() == API::VULKAN) {
            // TODO
        }
    }

    void UpdateDebugSolidMesh() {

        int m_mapWidth = quadCountX + 1;
        int m_mapHeight = quadCountY + 1;

        std::vector<Vertex> vertices;
        vertices.reserve(m_mapWidth * m_mapHeight);
        for (int y = 0; y < m_mapHeight; ++y) {
            for (int x = 0; x < m_mapWidth; ++x) {
                Vertex v;
                // world position before raycast
                v.position = glm::vec3(x * spacing, 0.0f, y * spacing) + g_gridOffset;
                v.uv = glm::vec2(x / float(quadCountX), y / float(quadCountY));

                // raycast down to sample height
                glm::vec3 rayOrigin = v.position + glm::vec3(0, 100, 0);
                glm::vec3 rayDirection = glm::vec3(0, -1, 0);
                auto     result = Physics::CastPhysXRayHeightMap(rayOrigin, rayDirection, 1000.0f);
                if (result.hitFound)
                    v.position = result.hitPosition;

                vertices.push_back(v);
            }
        }

        std::vector<uint32_t> indices;
        indices.reserve(quadCountX * quadCountY * 6);
        for (int y = 0; y < quadCountY; ++y) {
            for (int x = 0; x < quadCountX; ++x) {
                int i0 = Index1D(x, y, m_mapWidth);
                int i1 = Index1D(x + 1, y, m_mapWidth);
                int i2 = Index1D(x + 1, y + 1, m_mapWidth);
                int i3 = Index1D(x, y + 1, m_mapWidth);

                // triangle #1
                indices.push_back(i0);
                indices.push_back(i1);
                indices.push_back(i2);
                // triangle #2
                indices.push_back(i2);
                indices.push_back(i3);
                indices.push_back(i0);
            }
        }


        if (BackEnd::GetAPI() == API::OPENGL) {
            g_debuGSolidMeshBuffer.GetGLMeshBuffer().UpdateBuffers(vertices, indices);
        }
        else if (BackEnd::GetAPI() == API::VULKAN) {
            // TODO
        }
    }

    MeshBuffer& GetDebugGridMeshBuffer()    { return g_debugGridMeshBuffer; }
    MeshBuffer& GetDebugSolidMeshBuffer()   { return g_debuGSolidMeshBuffer; }
}