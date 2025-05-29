#include "AStarMap.h"
#include "BackEnd/BackEnd.h"
#include "Input/Input.h"
#include "Physics/Physics.h"

namespace AStarMap {

    MeshBuffer g_debugGridMeshBuffer;
    MeshBuffer g_debuGSolidMeshBuffer;
    glm::vec3 g_mapWorldSpaceOffset = glm::vec3(10.0f, 30.5f, 10.0f);

    int g_mapWidth = 100;
    int g_mapHeight = 100;
    float g_worldSpaceSpacing = 0.5f;

    void UpdateDebugGridMesh();
    void UpdateDebugSolidMesh();
    inline int Index1D(int x, int y, int mapWidth) { return y * mapWidth + x; }

    void Update() {
        //if (Input::KeyPressed(HELL_KEY_SPACE)) {
        //    UpdateDebugMeshesFromHeightField();
        //}
    }

    void UpdateDebugMeshesFromHeightField() {
        Physics::ActivateAllHeightFields();
        UpdateDebugGridMesh();
        UpdateDebugSolidMesh();
    }

    void UpdateDebugGridMesh() {
        std::vector<Vertex> vertices;
        vertices.reserve(g_mapWidth + 1 * g_mapHeight + 1);
        for (int y = 0; y < g_mapHeight + 1; ++y) {
            for (int x = 0; x < g_mapWidth + 1; ++x) {
                Vertex vertex;
                vertex.position = glm::vec3(x * g_worldSpaceSpacing, 0.0f, y * g_worldSpaceSpacing) + g_mapWorldSpaceOffset;
                vertex.uv = glm::vec2(x / float(g_mapWidth), y / float(g_mapHeight));
                
                // Raycast down to sample height
                glm::vec3 rayOrigin = vertex.position + glm::vec3(0, 100, 0);
                glm::vec3 rayDirection = glm::vec3(0, -1, 0);
                float rayLength = 1000;
                PhysXRayResult result = Physics::CastPhysXRayHeightMap(rayOrigin, rayDirection, rayLength);
                if (result.hitFound) {
                    vertex.position = result.hitPosition + glm::vec3(0.0f, 0.02f, 0.0f);
                }
                vertices.push_back(vertex);
            }
        }

        std::vector<uint32_t> indices;
        indices.reserve(((g_mapWidth + 1) * g_mapHeight + (g_mapHeight + 1) * g_mapWidth) * 2);
        // Vertical segments
        for (int x = 0; x <= g_mapWidth; ++x) {
            for (int y = 0; y < g_mapHeight; ++y) {
                indices.push_back(Index1D(x, y, g_mapWidth + 1));
                indices.push_back(Index1D(x, y + 1, g_mapWidth + 1));
            }
        }
        // Horizontal segments
        for (int y = 0; y <= g_mapHeight; ++y) {
            for (int x = 0; x < g_mapWidth; ++x) {
                indices.push_back(Index1D(x, y, g_mapWidth + 1));
                indices.push_back(Index1D(x + 1, y, g_mapWidth + 1));
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
        std::vector<Vertex> vertices;
        vertices.reserve(g_mapWidth + 1 * g_mapHeight + 1);
        for (int y = 0; y < g_mapHeight + 1; ++y) {
            for (int x = 0; x < g_mapWidth + 1; ++x) {
                Vertex vertex;
                vertex.position = glm::vec3(x * g_worldSpaceSpacing, 0.0f, y * g_worldSpaceSpacing) + g_mapWorldSpaceOffset;
                vertex.uv = glm::vec2(x / float(g_mapWidth), y / float(g_mapHeight));

                // Raycast down to sample height
                glm::vec3 rayOrigin = vertex.position + glm::vec3(0, 100, 0);
                glm::vec3 rayDirection = glm::vec3(0, -1, 0);
                PhysXRayResult result = Physics::CastPhysXRayHeightMap(rayOrigin, rayDirection, 1000.0f);
                if (result.hitFound) {
                    vertex.position = result.hitPosition + glm::vec3(0.0f, 0.02f, 0.0f);
                }

                vertices.push_back(vertex);
            }
        }

        std::vector<uint32_t> indices;
        indices.reserve(g_mapWidth * g_mapHeight * 6);
        for (int y = 0; y < g_mapHeight; ++y) {
            for (int x = 0; x < g_mapWidth; ++x) {
                int i0 = Index1D(x, y, g_mapWidth + 1);
                int i1 = Index1D(x + 1, y, g_mapWidth + 1);
                int i2 = Index1D(x + 1, y + 1, g_mapWidth + 1);
                int i3 = Index1D(x, y + 1, g_mapWidth + 1);

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

    int GetMapWidth()                       { return g_mapWidth; }
    int GetMapHeight()                      { return g_mapHeight; }
    MeshBuffer& GetDebugGridMeshBuffer()    { return g_debugGridMeshBuffer; }
    MeshBuffer& GetDebugSolidMeshBuffer()   { return g_debuGSolidMeshBuffer; }
}