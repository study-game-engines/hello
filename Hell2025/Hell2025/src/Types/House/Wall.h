#pragma once
#include "HellTypes.h"
#include "CreateInfo.h"
#include "Trim.h"
#include "WallSegment.h"
#include <glm/glm.hpp>
#include <vector>
struct Wall {

    void Init(WallCreateInfo createInfo);
    void CleanUp();
    void UpdateSegmentsAndVertexData();
    void UpdateWorldSpaceCenter(glm::vec3 worldSpaceCenter);
    void SubmitRenderItems();
    void CreateTrims();
    void DrawSegmentVertices(glm::vec4 color);
    void DrawSegmentLines(glm::vec4 color);
    void FlipFaces();
    bool AddPointToEnd(glm::vec3 point, bool supressWarning = true);
    bool UpdatePointPosition(int pointIndex, glm::vec3 position, bool supressWarning = true);

    glm::vec3 GetPointByIndex(int pointIndex); 

    const size_t GetPointCount() const                      { return m_createInfo.points.size(); }
    const glm::vec3& GetWorldSpaceCenter() const            { return m_worldSpaceCenter; }
    Material* GetMaterial()                                 { return m_material; };
    std::vector<WallSegment>& GetWallSegments()             { return m_wallSegments; }
    const uint64_t GetObjectId() const                      { return m_objectId; }
    const WallCreateInfo& GetCreateInfo() const             { return m_createInfo; }

private:
    uint64_t m_objectId = 0;
    //float m_height = 2.4f;
    //float m_textureScale = 1.0f;
    //float m_textureOffsetU = 0.0f;
    //float m_textureOffsetV = 0.0f;
    Material* m_material = nullptr;
    TrimType m_ceilingTrimType = TrimType::NONE;
    TrimType m_floorTrimType = TrimType::NONE;
    glm::vec3 m_worldSpaceCenter = glm::vec3(0.0f);
    //std::vector<glm::vec3> m_points;
    std::vector<WallSegment> m_wallSegments;
    std::vector<Trim> m_trims;
    WallCreateInfo m_createInfo;

    void CreateVertexData();
};