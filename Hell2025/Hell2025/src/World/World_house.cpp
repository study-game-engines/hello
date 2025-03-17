#include "World.h"
#include "Renderer/RenderDataManager.h"

namespace World {

    OpenGLDetachedMesh g_houseMesh;
    std::vector<Vertex> g_houseMeshVertices;
    std::vector<uint32_t> g_houseMeshIndices;
    std::vector<HouseRenderItem> g_renderItems;

    void UpdateHouseMeshVertexDataAndRenderItems() {
        std::vector<Wall>& walls = GetWalls();
        std::vector<HousePlane>& housePlanes = GetHousePlanes();

        g_houseMeshVertices.clear();
        g_houseMeshIndices.clear();

        int baseVertex = 0;
        int baseIndex = 0;

        // House walls
        for (Wall& wall : walls) {
            const Material* material = wall.GetMaterial() ? wall.GetMaterial() : AssetManager::GetDefaultMaterial();
            const std::vector<WallSegment>& wallSegments = wall.GetWallSegments();

            for (const WallSegment& wallSegment : wallSegments) {

                const std::vector<Vertex>& vertices = wallSegment.GetVertices();
                const std::vector<uint32_t>& indices = wallSegment.GetIndices();

                HouseRenderItem renderItem;
                renderItem.baseColorTextureIndex = material->m_basecolor;
                renderItem.normalMapTextureIndex = material->m_normal;
                renderItem.rmaTextureIndex = material->m_rma;
                renderItem.baseVertex = baseVertex;
                renderItem.baseIndex = baseIndex;
                renderItem.vertexCount = vertices.size();
                renderItem.indexCount = indices.size();
                renderItem.aabbMin = glm::vec4(0); // TODO
                renderItem.aabbMax = glm::vec4(0); // TODO

                RenderDataManager::SubmitHouseRenderItem(renderItem);

                g_houseMeshVertices.insert(g_houseMeshVertices.end(), vertices.begin(), vertices.end());
                g_houseMeshIndices.insert(g_houseMeshIndices.end(), indices.begin(), indices.end());

                baseVertex = g_houseMeshVertices.size();
                baseIndex = g_houseMeshIndices.size();
            }
        }

        // House planes
        for (HousePlane& housePlane : housePlanes) {
            const Material* material = housePlane.GetMaterial() ? housePlane.GetMaterial() : AssetManager::GetDefaultMaterial();

            std::vector<Vertex>& vertices = housePlane.GetVertices();
            std::vector<uint32_t>& indices = housePlane.GetIndices();

            HouseRenderItem renderItem;
            renderItem.baseColorTextureIndex = material->m_basecolor;
            renderItem.normalMapTextureIndex = material->m_normal;
            renderItem.rmaTextureIndex = material->m_rma;
            renderItem.baseVertex = baseVertex;
            renderItem.baseIndex = baseIndex;
            renderItem.vertexCount = vertices.size();
            renderItem.indexCount = indices.size();
            renderItem.aabbMin = glm::vec4(0); // TODO
            renderItem.aabbMax = glm::vec4(0); // TODO

            RenderDataManager::SubmitHouseRenderItem(renderItem);

            g_houseMeshVertices.insert(g_houseMeshVertices.end(), vertices.begin(), vertices.end());
            g_houseMeshIndices.insert(g_houseMeshIndices.end(), indices.begin(), indices.end());

            baseVertex = g_houseMeshVertices.size();
            baseIndex = g_houseMeshIndices.size();
        }

        g_houseMesh.UpdateBuffers(g_houseMeshVertices, g_houseMeshIndices);
    }

    OpenGLDetachedMesh& GetHouseMesh() {
        return g_houseMesh;
    }
}