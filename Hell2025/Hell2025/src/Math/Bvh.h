#pragma once
#include "HellTypes.h"
#include <vector>

#define MAX_BVH_STACK_SIZE 16

namespace BVH {
    uint64_t CreateMeshBvhFromVertices(std::vector<Vertex>& vertices);
    uint64_t CreateNewSceneBvh();

    void UpdateSceneBvh(uint64_t bvhId, std::vector<PrimitiveInstance>& instances);
    void DestroySceneBvh(uint64_t bvhId);
    bool SceneBvhExists(uint64_t bvhId); 
    bool MeshBvhExists(uint64_t bvhId);
    void DestroyMeshBvh(uint64_t bvhId);
    bool MeshBvhExists(uint64_t bvhId);
    void FlatternMeshBvhNodes();

    void RenderMesh(uint64_t bvhId, glm::vec4 color, glm::mat4 worldTransform);
    void RenderMeshBvh(uint64_t bvhId, glm::vec4 color, glm::mat4 worldTransform);
    void RenderRayResultTriangle(RayTraversalResult& rayResult, glm::vec4 color);
    void RenderRayResultNode(RayTraversalResult& rayResult, glm::vec4 color);
    void RenderSceneBvh(uint64_t bvhId, glm::vec4 color);

    bool IntersectNode(const RayData& rayData, const glm::vec3& aabbBoundsMin, const glm::vec3& aabbBoundsMax, float& t);
    bool IntersectNodeGpu(const RayData& rayData, const glm::vec3& aabbBoundsMin, const glm::vec3& aabbBoundsMax, float& t);
    bool IntersectTri(glm::vec3 rayOrigin, glm::vec3 rayDir, float minDistance, float maxDistance, glm::vec3 p0, glm::vec3 e1, glm::vec3 e2, glm::vec3 normal, float& t);

    RayData ComputeRayData(const glm::vec3& rayOrigin, const glm::vec3& rayDir, float minDistance, float maxDistance);

    RayTraversalResult AnyHit(uint64_t sceneBvhId, glm::vec3 rayOrigin, glm::vec3 rayDir, float maxDistance);
    RayTraversalResult ClosestHit(uint64_t sceneBvhId, glm::vec3 rayOrigin, glm::vec3 rayDir, float maxDistance);

    SceneBvh* GetSceneBvhById(uint64_t bvhId);
    MeshBvh* GetMeshBvhById(uint64_t bvhId);

    const std::vector<BvhNode>& GetSceneGpuBvhNodes();
    const std::vector<BvhNode>& GetMeshGpuBvhNodes();
    const std::vector<GpuPrimitiveInstance>& GetGpuEntityInstances(uint64_t sceneBvhId);
    const std::vector<float>& GetTriangleData();
}