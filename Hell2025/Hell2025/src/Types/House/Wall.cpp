#include "Wall.h"
#include "AssetManagement/AssetManager.h"
#include "Editor/Editor.h"
#include "Modelling/Clipping.h"
#include "Renderer/RenderDataManager.h"
#include "Renderer/Renderer.h"
#include "World/World.h"
#include "UniqueID.h"

void Wall::Init(WallCreateInfo createInfo) {
    m_objectId = UniqueID::GetNext(); // Only do this once
    m_createInfo = createInfo;

    UpdateSegmentsAndVertexData();

    std::cout << "Wall::Init(): " << Util::WallTypeToString(m_createInfo.wallType) << "\n";
}

void Wall::UpdateSegmentsAndVertexData() {
    CleanUp();

    for (WallSegment& wallSegment : m_wallSegments) {
        wallSegment.CleanUp();
    }
    m_wallSegments.clear();

    //m_points = m_createInfo.points;
    //m_height = m_createInfo.height;
    //m_textureOffsetU = m_createInfo.textureOffsetU;
    //m_textureOffsetV = m_createInfo.textureOffsetV;
    //m_textureScale = m_createInfo.textureScale;
    m_material = AssetManager::GetMaterialByName(m_createInfo.materialName);
    m_ceilingTrimType = m_createInfo.ceilingTrimType;
    m_floorTrimType = m_createInfo.floorTrimType;

    if (m_createInfo.useReversePointOrder) {
        std::reverse(m_createInfo.points.begin(), m_createInfo.points.end());
    }

    for (int i = 0; i < GetPointCount() - 1; i++) {
        const glm::vec3& start = m_createInfo.points[i];
        const glm::vec3& end = m_createInfo.points[i + 1];
        WallSegment& wallSegment = m_wallSegments.emplace_back();
        wallSegment.Init(start, end, m_createInfo.height, m_objectId);
    }

    // Calculate worldspace center
    m_worldSpaceCenter = glm::vec3(0.0f);
    if (!m_createInfo.points.empty()) {
        for (glm::vec3& point : m_createInfo.points) {
            m_worldSpaceCenter += point;
        }
        m_worldSpaceCenter /= m_createInfo.points.size();
    }

    // Create weather boards
    if (m_createInfo.wallType == WallType::WEATHER_BOARDS) {
        CreateWeatherBoards();
    }
    // Create CSG geometry and trims
    else {
        CreateCSGVertexData();
        CreateTrims();
    }
}

void Wall::FlipFaces() {
    m_createInfo.useReversePointOrder = !m_createInfo.useReversePointOrder;
    UpdateSegmentsAndVertexData();
}

void Wall::UpdateWorldSpaceCenter(glm::vec3 worldSpaceCenter) {
    glm::vec3 offset = worldSpaceCenter - m_worldSpaceCenter;
    for (glm::vec3& point : m_createInfo.points) {
        point += offset;
    }
    UpdateSegmentsAndVertexData();
}

bool Wall::AddPointToEnd(glm::vec3 point, bool supressWarning) {
    glm::vec3& previousPoint = m_createInfo.points.back();
    float threshold = 0.05f;
    if (glm::distance(point, previousPoint) < threshold) {
        std::cout << "Wall::AddPoint() failed: new point " << point << " is too close to previous point " << previousPoint << "\n";
        return false;
    }

    m_createInfo.points.push_back(point);
    UpdateSegmentsAndVertexData();
    return true;
}

bool Wall::UpdatePointPosition(int pointIndex, glm::vec3 position, bool supressWarning) {
    if (pointIndex < 0 || pointIndex >= m_createInfo.points.size()) {
        std::cout << "Wall::UpdatePointPosition() failed: point index " << pointIndex << " out of range of size " << m_createInfo.points.size() << "\n";
    }

    // Threshold check
    float threshold = 0.05f;
    if (pointIndex > 0) {
        glm::vec3& previousPoint = m_createInfo.points[pointIndex - 1];
        if (glm::distance(position, previousPoint) < threshold) {
            std::cout << "Wall::UpdatePointPosition() failed: new point " << position << " is too close to previous point " << previousPoint << "\n";
            return false;
        }
    }
    if (pointIndex < m_createInfo.points.size() - 1) {
        glm::vec3& nextPoint = m_createInfo.points[pointIndex + 1];
        if (glm::distance(position, nextPoint) < threshold) {
            std::cout << "Wall::UpdatePointPosition() failed: new point " << position << " is too close to next point " << nextPoint << "\n";
            return false;
        }
    }

    m_createInfo.points[pointIndex] = position;
    UpdateSegmentsAndVertexData();
    return true;
}

glm::vec3 Wall::GetPointByIndex(int pointIndex) {
    if (pointIndex < 0 || pointIndex >= m_createInfo.points.size()) {
        std::cout << "Wall::GetPointByIndex() failed: point index " << pointIndex << " out of range of size " << m_createInfo.points.size() << "\n";
        return glm::vec3(0.0f);
    }
    return m_createInfo.points[pointIndex];
}

void Wall::CleanUp() {
    for (WallSegment& wallSegment : m_wallSegments) {
        wallSegment.CleanUp();
    }

    //m_ceilingTrimType = TrimType::NONE;
    //m_floorTrimType = TrimType::NONE;
    //m_points.clear();
    //m_trims.clear();
    //m_wallSegments.clear();
    //m_height = 0.0f;
    //m_textureOffsetU = 0.0;
    //m_textureOffsetV = 0.0f;
    //m_textureScale = 1.0f;
    //m_material = nullptr;
    //m_createInfo = WallCreateInfo();
}

void Wall::CreateTrims() {
    m_trims.clear();

    World::UpdateDoorAndWindowCubeTransforms();

    
    // Ceiling
    if (m_ceilingTrimType != TrimType::NONE) {
        for (int i = 0; i < m_createInfo.points.size() - 1; i++) {
            const glm::vec3& start = m_createInfo.points[i];
            const glm::vec3& end = m_createInfo.points[i + 1];

            Transform transform;
            transform.position = start;
            transform.position.y += m_createInfo.height;
            transform.rotation.y = Util::EulerYRotationBetweenTwoPoints(start, end);
            transform.scale.x = glm::distance(start, end);

            Trim& trim = m_trims.emplace_back();
            trim.Init(transform, "TrimCeiling", "Trims");
        }
    }

    // Floor
    if (m_floorTrimType != TrimType::NONE) {
        for (int i = 0; i < m_createInfo.points.size() - 1; i++) {
            const glm::vec3& start = m_createInfo.points[i];
            const glm::vec3& end = m_createInfo.points[i + 1];
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

void Wall::CreateCSGVertexData() {
    for (WallSegment& wallSegment : m_wallSegments) {
        wallSegment.CreateVertexData(World::GetClippingCubes(), m_createInfo.textureOffsetU, m_createInfo.textureOffsetV, m_createInfo.textureScale);
    }
}

void Wall::SubmitRenderItems() {
    for (WallSegment& wallSegment : m_wallSegments) {
        Mesh* mesh = World::GetHouseMeshByIndex(wallSegment.GetMeshIndex());
        if (!mesh) continue;

        HouseRenderItem renderItem;
        renderItem.baseColorTextureIndex = m_material->m_basecolor;
        renderItem.normalMapTextureIndex = m_material->m_normal;
        renderItem.rmaTextureIndex = m_material->m_rma;
        renderItem.baseVertex = mesh->baseVertex;
        renderItem.baseIndex = mesh->baseIndex;
        renderItem.vertexCount = mesh->vertexCount;
        renderItem.indexCount = mesh->indexCount;
        renderItem.aabbMin = glm::vec4(mesh->aabbMin, 0.0f);
        renderItem.aabbMax = glm::vec4(mesh->aabbMax, 0.0f);
        renderItem.meshIndex = wallSegment.GetMeshIndex();
        RenderDataManager::SubmitRenderItem(renderItem);

        // Outline?
        //if (Editor::GetHoveredObjectId() == m_objectId) {
        //    RenderDataManager::SubmitOutlineRenderItem(renderItem);
        //    //std::cout << "hover found id: " << m_objectId << "\n";
        //}
    }
    //std::cout << "wall::submit() id: " << m_objectId << "\n";

    for (Trim& trim : m_trims) {
        trim.SubmitRenderItem();
    }
}

void Wall::DrawSegmentVertices(glm::vec4 color) {
    for (WallSegment& wallSegment : m_wallSegments) {
        const glm::vec3& p1 = wallSegment.GetStart();
        const glm::vec3& p2 = wallSegment.GetEnd();
        glm::vec3 p3 = wallSegment.GetStart() + glm::vec3(0.0f, wallSegment.GetHeight(), 0.0f);
        glm::vec3 p4 = wallSegment.GetEnd() + glm::vec3(0.0f, wallSegment.GetHeight(), 0.0f);
        Renderer::DrawPoint(p1, color);
        Renderer::DrawPoint(p2, color);
        Renderer::DrawPoint(p3, color);
        Renderer::DrawPoint(p4, color);
    }
}



void Wall::DrawSegmentLines(glm::vec4 color) {
    for (WallSegment& wallSegment : m_wallSegments) {
        const glm::vec3& p1 = wallSegment.GetStart();
        const glm::vec3& p2 = wallSegment.GetEnd();
        glm::vec3 p3 = wallSegment.GetStart() + glm::vec3(0.0f, wallSegment.GetHeight(), 0.0f);
        glm::vec3 p4 = wallSegment.GetEnd() + glm::vec3(0.0f, wallSegment.GetHeight(), 0.0f);
        Renderer::DrawLine(p1, p2, color);
        Renderer::DrawLine(p3, p4, color);
        Renderer::DrawLine(p1, p3, color);
        Renderer::DrawLine(p2, p4, color);

        glm::vec3 midPoint = Util::GetMidPoint(wallSegment.GetStart(), wallSegment.GetEnd()); 
        glm::vec3 normal = wallSegment.GetNormal();
        glm::vec3 projectedMidPoint = midPoint + (normal * 0.2f);
        Renderer::DrawLine(midPoint, projectedMidPoint, color);
    }
}





BoardVertexData Wall::CreateBoardVertexData(glm::vec3 begin, glm::vec3 end, glm::vec3 boardDirection, int yUVOffsetIndex, float xUVOffset) {

    BoardVertexData weatherBoardVertexData;

    Model* model = AssetManager::GetModelByName("WeatherBoard");
    uint32_t meshIndex = model->GetMeshIndices()[0];
    Mesh* mesh = AssetManager::GetMeshByIndex(meshIndex);

    std::span<Vertex> verticesSpan = AssetManager::GetMeshVerticesSpan(mesh);
    std::span<uint32_t> indicesSpan = AssetManager::GetMeshIndicesSpan(mesh);

    float boardWidth = glm::distance(begin, end);
    const float meshBoardWidth = 4.0f; // FULL WIDTH (MATHCES TEXUTRE STUFF)

    glm::mat4 rotationMatrix = Util::GetRotationMat4FromForwardVector(boardDirection);

    for (Vertex vertex : verticesSpan) {

        // Shift right most vertices
        if (vertex.position.z > 0.5f) {
            vertex.position.x = boardWidth * boardDirection.x;
            vertex.position.z = boardWidth * boardDirection.z;
            vertex.uv.x = boardWidth / meshBoardWidth;
        }

        // Rotate the normal
        vertex.normal = rotationMatrix * glm::vec4(vertex.normal, 0.0f);

        // Finally shift to the origin
        vertex.position += begin;

        // Test next board up 
        float uvVerticalOffset = 1.0f / 16.0f;;
        vertex.uv.y -= uvVerticalOffset * yUVOffsetIndex;

        vertex.uv.x += xUVOffset;
        weatherBoardVertexData.vertices.push_back(vertex);
    }

    // Indices
    for (uint32_t& index : indicesSpan) {
        weatherBoardVertexData.indices.push_back(index);
    }

    return weatherBoardVertexData;
}

#define WEATHERBOARD_STOP_MESH_HEGIHT 2.6f

void Wall::CreateWeatherBoards() {

    float individialBoardHeight = 0.13f;
    float desiredTotalWallHeight = 2.8;
    int weatherBoardCount = desiredTotalWallHeight / individialBoardHeight;
    float actualFinalWallHeight = weatherBoardCount * individialBoardHeight;
    float weathboardStopScale = actualFinalWallHeight / WEATHERBOARD_STOP_MESH_HEGIHT;
    
    // Make the stops
    m_weatherBoardstopRenderItems.clear();

    for (WallSegment& wallSegemet : m_wallSegments) {
        glm::vec3 start = wallSegemet.GetStart();
        glm::vec3 end = wallSegemet.GetEnd();
        glm::vec3 segmentDir = glm::normalize(end - start);
        glm::mat4 rotationMatrix = Util::GetRotationMat4FromForwardVector(segmentDir);
        Transform transform;
        transform.position = start;
        transform.scale.y = weathboardStopScale;

        Material* material = AssetManager::GetMaterialByName("WeatherBoards0");
        Model* model = AssetManager::GetModelByName("WeatherboardStop");

        RenderItem& renderItem = m_weatherBoardstopRenderItems.emplace_back();
        renderItem.modelMatrix = transform.to_mat4() * rotationMatrix;
        renderItem.inverseModelMatrix = glm::inverse(renderItem.modelMatrix);
        renderItem.meshIndex = model->GetMeshIndices()[0];
        renderItem.baseColorTextureIndex = material->m_basecolor;
        renderItem.rmaTextureIndex = material->m_rma;
        renderItem.normalMapTextureIndex = material->m_normal;
        Util::UpdateRenderItemAABB(renderItem);
        Util::PackUint64(m_objectId, renderItem.objectIdLowerBit, renderItem.objectIdUpperBit);
    }

    std::cout << "Wall::CreateWeatherBoards()\n";

    // Ensure all the door and window cube transforms exist at the time this runs
    World::UpdateDoorAndWindowCubeTransforms();




    m_boardVertexDataSet.clear();

    for (WallSegment& wallSegemet : m_wallSegments) {

       // glm::vec3 origin = glm::vec3(-0.0225f, 0.0f, -3.0f);
        //glm::vec3 origin = wallSegemet.GetStart();
//
//
// glm::vec3 start = wallSegemet.GetStart();
// glm::vec3 end = wallSegemet.GetEnd();




        //std::vector<BoardVertexData> boardVertexDataSet;

        int yUVOffsetIndex = -1;

        for (int i = 0; i < weatherBoardCount; i++) {
            yUVOffsetIndex++;
            
            float xUVOffset = 0.0f;


            if (i > 15) {
                yUVOffsetIndex = 12;
            }
            if (i > 12) {
                xUVOffset = Util::RandomFloat(0.0f, 100.0f);
            }

            glm::vec3 start = wallSegemet.GetStart();
            glm::vec3 end = wallSegemet.GetEnd();

            start.y += individialBoardHeight * i;
            end.y += individialBoardHeight * i;

            glm::vec3 rayOrigin = start;
            glm::vec3 rayDir = glm::normalize(end - start);
            float desiredBoardLength = glm::distance(start, end);

            CubeRayResult rayResult;
            do {
                rayResult = Util::CastCubeRay(rayOrigin, rayDir, World::GetDoorAndWindowCubeTransforms(), desiredBoardLength);
                if (rayResult.hitFound) {

                    glm::vec3 hitPos = rayOrigin + (rayDir * rayResult.distanceToHit);
                   // DrawPoint(hitPos, YELLOW);

                    Transform transform;
                    transform.position = rayOrigin;
                    transform.rotation.y = Util::EulerYRotationBetweenTwoPoints(start, end);
                    transform.scale.x = rayResult.distanceToHit;

                    float dot = glm::dot(rayResult.hitNormal, rayDir);
                    if (dot <= 0.99f) {
                        glm::vec3 localStart = rayOrigin;
                        glm::vec3 localEnd = rayOrigin + (rayDir * rayResult.distanceToHit);

                        BoardVertexData boardVertexData = CreateBoardVertexData(localStart, localEnd, rayDir, yUVOffsetIndex, xUVOffset);
                        m_boardVertexDataSet.emplace_back(boardVertexData);
                    }

                    // Start next ray just past the opposite face of the hit cube
                    rayOrigin = rayResult.hitPosition + (rayDir * 0.01f);
                }

            } while (rayResult.hitFound);

            BoardVertexData boardVertexData = CreateBoardVertexData(rayOrigin, end, rayDir, yUVOffsetIndex, xUVOffset);
            m_boardVertexDataSet.emplace_back(boardVertexData);

            }
        //break;
    }


    

}

