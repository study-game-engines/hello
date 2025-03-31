#pragma once
#include "HellTypes.h"
#include <vector>

struct RayTraversalResult {
    bool hitFound = false;
    glm::vec3 hitPosition = glm::vec3(0.0f);
    size_t primtiviveId = 0;
    float t = 0;
};

struct SceneRayTraversalResult {
    bool hitFound = false;
    glm::vec3 hitPosition = glm::vec3(0.0f);
    size_t instanceIndex = 0;
    size_t primtiviveId = 0;
    float t = 0;
};

namespace BVH {
    uint64_t CreateBvhFromVertices(std::vector<Vertex>& vertices);
    uint64_t CreateSceneBvh(std::vector<ObjectInstanceData>& instances);

    RayTraversalResult ClosestHitTriangleMesh(uint64_t bvhId, glm::vec3 rayOrigin, glm::vec3 rayDir, float maxDistance, glm::mat4 meshTransform);
    SceneRayTraversalResult ClosestHitScene(uint64_t bvhId, glm::vec3 rayOrigin, glm::vec3 rayDir, float maxDistance);

    void RenderSceneBvh(uint64_t bvhId, glm::vec4 color);
    void RenderSceneBvhFlat(uint64_t bvhId, glm::vec4 color);
    void DestroySceneBvh(uint64_t bvhId);
    bool SceneBvhExists(uint64_t bvhId);

    void RenderTriangleMesh(uint64_t bvhId, glm::vec4 color, glm::mat4 worldTransform);
    void RenderTriangleMeshBvh(uint64_t bvhId, glm::vec4 color, glm::mat4 worldTransform);
    void RenderTriangleMeshTriangle(uint64_t bvhId, uint64_t primtiviveId, glm::vec4 color, glm::mat4 worldTransform);
    void DestroyTriangleMeshBvh(uint64_t bvhId);
    bool TriangleMeshBvhExists(uint64_t bvhId);
}