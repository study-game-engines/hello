#include "AStarMap.h"
#include "BackEnd/BackEnd.h"
#include "Input/Input.h"
#include "Physics/Physics.h"
#include "World/World.h"

namespace AStarMap {

    MeshBuffer g_debugGridMeshBuffer;
    MeshBuffer g_debuGSolidMeshBuffer;
    glm::vec3 g_mapWorldSpaceOffset = glm::vec3(10.0f, 30.5f, 10.0f);

    std::vector<int> g_map;
    int g_mapWidth = 200;
    int g_mapHeight = 200;
    float g_worldSpaceSpacing = 0.25f;    
    std::vector<glm::ivec2> g_wallCells; // Only used to debug draw

    void UpdateDebugGridMesh();
    void UpdateDebugSolidMesh();

    inline glm::ivec2 Index2D(int idx) { return glm::ivec2(idx % g_mapWidth, idx / g_mapWidth); }
    inline int Index1DDebugMesh(int x, int y, int mapWidth) { return y * mapWidth + x; }
    inline int Index1D(int x, int y) { return y * g_mapWidth + x; }


    void Init() {
        g_map.resize(GetCellCount());

        // Mark trees
        for (Tree& tree : World::GetTrees()) {

            glm::vec3 position = tree.GetPosition();
            
            int size = 2;
            glm::ivec2 treeCell = GetCellCoordsFromWorldSpacePosition(position);

            for (int x = -size; x <= size; x++) {
                for (int y = -size; y <= size; y++) {
                    MarkCellAsObstacle(treeCell.x + x, treeCell.y + y);
                }
            }


            std::cout << "Marking tree " << position << " at " << treeCell << "\n";
        }

        // Store wall cells once at init
        g_wallCells.clear();
        for (int i = 0; i < g_map.size(); i++) {
            if (g_map[i]) {                
                g_wallCells.push_back(Index2D(i));
            }
        }

        std::cout << "World::GetTrees().size(): " << World::GetTrees().size() << "\n";
        std::cout << "g_wallCells.size(): " << g_wallCells.size() << "\n";
    }

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
                indices.push_back(Index1DDebugMesh(x, y, g_mapWidth + 1));
                indices.push_back(Index1DDebugMesh(x, y + 1, g_mapWidth + 1));
            }
        }
        // Horizontal segments
        for (int y = 0; y <= g_mapHeight; ++y) {
            for (int x = 0; x < g_mapWidth; ++x) {
                indices.push_back(Index1DDebugMesh(x, y, g_mapWidth + 1));
                indices.push_back(Index1DDebugMesh(x + 1, y, g_mapWidth + 1));
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
                int i0 = Index1DDebugMesh(x, y, g_mapWidth + 1);
                int i1 = Index1DDebugMesh(x + 1, y, g_mapWidth + 1);
                int i2 = Index1DDebugMesh(x + 1, y + 1, g_mapWidth + 1);
                int i3 = Index1DDebugMesh(x, y + 1, g_mapWidth + 1);

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

    void MarkCellAsObstacle(int x, int y) {
        if (IsInBounds(x, y)) {
            int idx = Index1D(x, y);
            g_map[idx] = 1;
            //std::cout << "MarkCellAsObstacle() marked " << x << ", " << y << " as true, aka 1d idx " << idx << "\n";
        }
        else {
            std::cout << "MarkCellAsObstacle() failed coz " << x << ", " << y << " is out of bounds\n";
        }
    }

    bool IsInBounds(int x, int y) {
        return (x >= 0 && y >= 0 && x < g_mapWidth && y < g_mapHeight);
    }

    bool IsCellObstacle(int x, int y) {
        if (IsInBounds) {
            int idx = Index1D(x, y);
            return g_map[idx];
        }
        else {
            return false;
        }
    }

    glm::ivec2 GetCellCoordsFromWorldSpacePosition(glm::vec3 position) {
        int x = ((position.x - g_mapWorldSpaceOffset.x)) / g_worldSpaceSpacing;
        int y = ((position.z - g_mapWorldSpaceOffset.z)) / g_worldSpaceSpacing;
        return  glm::ivec2(x, y);
    }

    std::vector<glm::ivec2> GetWallCells()  { return g_wallCells; }
    int GetCellCount()                      { return g_mapWidth * g_mapHeight; }
    int GetMapWidth()                       { return g_mapWidth; }
    int GetMapHeight()                      { return g_mapHeight; }
    MeshBuffer& GetDebugGridMeshBuffer()    { return g_debugGridMeshBuffer; }
    MeshBuffer& GetDebugSolidMeshBuffer()   { return g_debuGSolidMeshBuffer; }
}