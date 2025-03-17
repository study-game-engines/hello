#include "Wall.h"
#include "Renderer/Renderer.h"
#include "Modelling/Clipping.h"
#include "World/World.h"

void Wall::Init(WallCreateInfo createInfo) {
    CleanUp();

    m_points = createInfo.points;
    m_height = createInfo.height;
    m_textureOffsetX = createInfo.textureOffsetX;
    m_textureOffsetY = createInfo.textureOffsetY;
    m_textureScale = createInfo.textureScale;
    m_material = AssetManager::GetMaterialByName(createInfo.materialName);

    for (int i = 0; i < m_points.size() - 1; i++) {
        const glm::vec3& start = m_points[i];
        const glm::vec3& end = m_points[i + 1];
        WallSegment& wallSegment = m_wallSegments.emplace_back();
        wallSegment.Init(start, end, m_height);
    }
    CreateVertexData();
}

void Wall::CleanUp() {
    for (WallSegment& wallSegment : m_wallSegments) {
        wallSegment.CleanUp();
    }
    m_wallSegments.clear();
    m_points.clear();
    m_height = 0.0f;
    m_textureOffsetX = 0.0;
    m_textureOffsetY = 0.0f;
    m_textureScale = 1.0f;
    m_material = nullptr;
}

void Wall::CreateVertexData() {
    for (WallSegment& wallSegment : m_wallSegments) {
        wallSegment.CreateVertexData(World::GetClippingCubes(), m_textureOffsetX, m_textureOffsetY, m_textureScale);
    }
}