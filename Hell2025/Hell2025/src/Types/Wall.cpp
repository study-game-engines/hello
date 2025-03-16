#include "Wall.h"
#include "Renderer/Renderer.h"
#include "Modelling/Clipping.h"
#include "Input/Input.h"
#include "World/World.h"

void Wall::Init(WallCreateInfo createInfo) {
    m_points = createInfo.points;
    m_height = createInfo.height;
    m_textureOffsetX = createInfo.textureOffsetX;
    m_textureOffsetY = createInfo.textureOffsetY;
    m_textureScale = createInfo.textureScale;
    m_material = AssetManager::GetMaterialByName(createInfo.materialName);
}

void Wall::UpdateRenderItems(int globalBaseVertex, int globalBaseIndex) {
    m_vertices.clear();
    m_indices.clear();
    m_wallSegments.clear();
    m_renderItems.clear();

    std::vector<ClippingCube>& clippingCubes = World::GetClippingCubes();

    for (int i = 0; i < m_points.size() -1; i++) {
        const glm::vec3& start = m_points[i];
        const glm::vec3& end = m_points[i + 1];
        WallSegment& wallSegment = m_wallSegments.emplace_back();
        wallSegment.Update(start, end, m_height);

        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        int baseVertex = m_vertices.size();
        int baseIndex = m_indices.size();

        Clipping::SubtractCubesFromWallSegment(wallSegment, clippingCubes, m_textureOffsetX, m_textureOffsetY, m_textureScale, vertices, indices);

        // Fallback for missing material
        if (!m_material) {
            m_material = AssetManager::GetDefaultMaterial();
        }

        HouseRenderItem& renderItem = m_renderItems.emplace_back();
        renderItem.baseColorTextureIndex = m_material->m_basecolor;
        renderItem.normalMapTextureIndex = m_material->m_normal;
        renderItem.rmaTextureIndex = m_material->m_rma;
        renderItem.baseVertex = m_vertices.size() + globalBaseVertex;
        renderItem.baseIndex = m_indices.size() + globalBaseIndex;
        renderItem.vertexCount = vertices.size();
        renderItem.indexCount = indices.size();
        renderItem.aabbMin = glm::vec4(0); // TODO
        renderItem.aabbMax = glm::vec4(0); // TODO

        m_vertices.insert(m_vertices.end(), vertices.begin(), vertices.end());
        m_indices.insert(m_indices.end(), indices.begin(), indices.end());
    }

    for (Vertex& v : m_vertices) {
       // Renderer::DrawPoint(v.position, RED);
    }

    for (int i = 0; i < m_indices.size(); i += 3) {
        Vertex& v0 = m_vertices[m_indices[i + 0]];
        Vertex& v1 = m_vertices[m_indices[i + 1]];
        Vertex& v2 = m_vertices[m_indices[i + 2]];

        //Renderer::DrawLine(v0.position, v1.position, RED);
        //Renderer::DrawLine(v1.position, v2.position, RED);
        //Renderer::DrawLine(v2.position, v0.position, RED);

     //if (Input::KeyPressed(HELL_KEY_DELETE)) {
     //    std::cout << i << " ";
     //    std::cout << m_indices[i + 0] << " ";
     //    std::cout << m_indices[i + 1] << " ";
     //    std::cout << m_indices[i + 2] << " ";
     //    std::cout << "\n";
     //}
    }



   ///std::cout << "Total: " << m_vertices.size() << " verts " << m_indices.size() << "\n";
}

void Wall::DebugDraw() {
    //for (const glm::vec3& point : m_points) {
    //    Renderer::DrawPoint(point, OUTLINE_COLOR);
    //}

    for (WallSegment& wallSegment : m_wallSegments) {
        for (const glm::vec3& v : wallSegment.GetCorners()) {
            //Renderer::DrawPoint(v, OUTLINE_COLOR);
        }
    }
}