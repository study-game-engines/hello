#include "Wall.h"
#include "Renderer/Renderer.h"
#include "Modelling/Clipping.h"
#include "World/World.h"
#include "AssetManagement/AssetManager.h"

void Wall::Init(WallCreateInfo createInfo) {
    CleanUp();

    m_points = createInfo.points;
    m_height = createInfo.height;
    m_textureOffsetX = createInfo.textureOffsetX;
    m_textureOffsetY = createInfo.textureOffsetY;
    m_textureScale = createInfo.textureScale;
    m_material = AssetManager::GetMaterialByName(createInfo.materialName);
    m_ceilingTrimType = createInfo.ceilingTrimType;
    m_floorTrimType = createInfo.floorTrimType;

    for (int i = 0; i < m_points.size() - 1; i++) {
        const glm::vec3& start = m_points[i];
        const glm::vec3& end = m_points[i + 1];
        WallSegment& wallSegment = m_wallSegments.emplace_back();
        wallSegment.Init(start, end, m_height);
    }
    CreateVertexData();
    CreateTrims();
}

void Wall::CleanUp() {
    for (WallSegment& wallSegment : m_wallSegments) {
        wallSegment.CleanUp();
    }
    m_ceilingTrimType = TrimType::NONE;
    m_floorTrimType = TrimType::NONE;
    m_points.clear();
    m_trims.clear();
    m_wallSegments.clear();
    m_height = 0.0f;
    m_textureOffsetX = 0.0;
    m_textureOffsetY = 0.0f;
    m_textureScale = 1.0f;
    m_material = nullptr;
}

void Wall::CreateTrims() {
    m_trims.clear();
    
    // Ceiling
    if (m_ceilingTrimType != TrimType::NONE) {
        for (int i = 0; i < m_points.size() - 1; i++) {
            const glm::vec3& start = m_points[i];
            const glm::vec3& end = m_points[i + 1];

            Transform transform;
            transform.position = start;
            transform.position.y += m_height;
            transform.rotation.y = Util::EulerYRotationBetweenTwoPoints(start, end);
            transform.scale.x = glm::distance(start, end);

            Trim& trim = m_trims.emplace_back();
            trim.Init(transform, "TrimCeiling", "Trims");
        }
    }

    // Floor
    if (m_floorTrimType != TrimType::NONE) {
        for (int i = 0; i < m_points.size() - 1; i++) {
            const glm::vec3& start = m_points[i];
            const glm::vec3& end = m_points[i + 1];
            glm::vec3 rayOrigin = start;
            glm::vec3 rayDir = glm::normalize(end - start);
            float segmentLength = glm::distance(start, end);

            CubeRayResult rayResult;
            do {
                rayResult = Util::CastCubeRay(rayOrigin, rayDir, World::GetDoorAndWindowCubeTransforms(), segmentLength);
                if (rayResult.hitFound) {

                    Transform transform;
                    transform.position = rayOrigin;
                    transform.rotation.y = Util::EulerYRotationBetweenTwoPoints(start, end);
                    transform.scale.x = rayResult.distanceToHit;

                    float dot = glm::dot(rayResult.hitNormal, rayDir);
                    if (dot <= 0.99f) {
                        Trim& trim = m_trims.emplace_back();
                        trim.Init(transform, "TrimFloor", "Trims");
                    }

                    // Start next ray just past the opposite face of the hit cube
                    rayOrigin = rayResult.hitPosition + (rayDir * 0.01f);
                }

            } while (rayResult.hitFound);

            Transform transform;
            transform.position = rayOrigin;
            transform.rotation.y = Util::EulerYRotationBetweenTwoPoints(rayOrigin, end);
            transform.scale.x = glm::distance(rayOrigin, end);

            Trim& trim = m_trims.emplace_back();
            trim.Init(transform, "TrimFloor", "Trims");
        }
    }
}

void Wall::CreateVertexData() {
    for (WallSegment& wallSegment : m_wallSegments) {
        wallSegment.CreateVertexData(World::GetClippingCubes(), m_textureOffsetX, m_textureOffsetY, m_textureScale);
    }
}

void Wall::SubmitTrimRenderItems() {
    for (Trim& trim : m_trims) {
        trim.SubmitRenderItem();
    }
};