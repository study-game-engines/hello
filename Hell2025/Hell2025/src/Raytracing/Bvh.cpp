#include "BVH.h"
#include "Renderer/Renderer.h"
#include "HellDefines.h"
#include "Timer.hpp"
#include "UniqueID.h"

#include <iostream>
#include <unordered_map>

#include "bvh/v2/bvh.h"
#include "bvh/v2/vec.h"
#include "bvh/v2/bbox.h"
#include "bvh/v2/ray.h"
#include "bvh/v2/node.h"
#include "bvh/v2/default_builder.h"
#include "bvh/v2/thread_pool.h"
#include "bvh/v2/executor.h"
#include "bvh/v2/stack.h"
#include "bvh/v2/tri.h"

using Scalar = float;
using BvhVec3 = bvh::v2::Vec<Scalar, 3>;
using BBox = bvh::v2::BBox<Scalar, 3>;
using Tri = bvh::v2::Tri<Scalar, 3>;
using BvhNode = bvh::v2::Node<Scalar, 3>;
using Bvh = bvh::v2::Bvh<BvhNode>;
using Ray = bvh::v2::Ray<Scalar, 3>;
using PreComputedTris = std::vector<bvh::v2::PrecomputedTri<float>>;

struct SceneBvh {
    Bvh m_bvh;
    std::vector<ObjectInstanceData> m_instances;
    std::vector<BBox> m_bboxes;
    std::vector<BvhVec3> m_centers;
};

struct TriangleMeshBvh {
    Bvh m_bvh;
    std::vector<Tri> m_tris;
    PreComputedTris m_precomputedTris;
};

struct RayData {
    float origin[3];
    float dir[3];
    float invDir[3];
    float paddedInvDir[3];
    float minDistance = 0;
    float maxDistance = 0;
    int octant[3];
};

namespace BVH {

    std::unordered_map<uint64_t, TriangleMeshBvh> g_triangleMeshBvhs;
    std::unordered_map<uint64_t, SceneBvh> g_sceneBvhs;
    bvh::v2::ThreadPool g_threadPool;
    bvh::v2::ParallelExecutor g_executor(g_threadPool);

    bool TriangleMeshBvhExists(uint64_t bvhId);
    bool AabbBvhExists(uint64_t bvhId);
    void RenderBvh(const Bvh& bvh, glm::vec4& color, glm::mat4& worldTransform, uint32_t nodeIndex = 0);
    void RenderBvhFlat(const Bvh& bvh, glm::vec4& color);
    RayTraversalResult ClosestHitTriangleMesh(TriangleMeshBvh& triangleMeshBvh, glm::vec3 rayOrigin, glm::vec3 rayDir, float maxDistance, glm::mat4 meshTransform);
    SceneRayTraversalResult ClosestHitScene(SceneBvh& sceneBvh, glm::vec3 rayOrigin, glm::vec3 rayDir, float maxDistance);
    glm::vec3 BvhVec3ToGlmVec3(BvhVec3 vec);
    BvhVec3 GlmVec3ToBvhVec3(glm::vec3 vec);
    uint32_t FloatToBits(float x);
    float BitsToFloat(uint32_t u);
    float AddUlpMagnitude(float x, unsigned ulps);
    RayData ComputeRayData(const glm::vec3& rayOrigin, const glm::vec3& rayDir, float minDistance, float maxDistance);
    float IntersectAxisMin(const RayData& rayData, int axis, float p);
    float IntersectAxisMax(const RayData& rayData, int axis, float p);
    float RobustMin(float x, float y);
    float RobustMax(float x, float y);
    bool IntersectNodeDistance(const RayData& rayData, const glm::vec3& aabbBoundsMin, const glm::vec3& aabbBoundsMax, float& t);

    RayTraversalResult ClosestHitTriangleMesh(uint64_t bvhId, glm::vec3 rayOrigin, glm::vec3 rayDir, float maxDistance, glm::mat4 meshTransform) {
        if (TriangleMeshBvhExists(bvhId)) {
            TriangleMeshBvh& triangleMeshBvh = g_triangleMeshBvhs[bvhId];
            return ClosestHitTriangleMesh(triangleMeshBvh, rayOrigin, rayDir, maxDistance, meshTransform);
        }
        else {
            return RayTraversalResult();
        }
    }

    SceneRayTraversalResult ClosestHitScene(uint64_t bvhId, glm::vec3 rayOrigin, glm::vec3 rayDir, float maxDistance) {
        if (SceneBvhExists(bvhId)) {
            SceneBvh& sceneBvh = g_sceneBvhs[bvhId];
            return ClosestHitScene(sceneBvh, rayOrigin, rayDir, maxDistance);
        }
        else {
            return SceneRayTraversalResult();
        }
    }

    uint64_t CreateBvhFromVertices(std::vector<Vertex>& vertices) {
        //Timer timer("CreateBvhFromVertices() " + std::to_string(vertices.size()) + " verts");

        uint64_t uniqueId = UniqueID::GetNext();
        TriangleMeshBvh& triangleMeshBvh = g_triangleMeshBvhs[uniqueId];

        // Get references for clarity
        Bvh& bvh = triangleMeshBvh.m_bvh;
        std::vector<Tri>& tris = triangleMeshBvh.m_tris;
        PreComputedTris& precomputedTris = triangleMeshBvh.m_precomputedTris;

        // Clear any old data
        tris.clear();
        precomputedTris.clear();

        // Preallocate vector storage
        tris.reserve(vertices.size() / 3);
        precomputedTris.resize(vertices.size() / 3);

        // Create triangles from vertices
        for (int i = 0; i < vertices.size(); i += 3) {
            glm::vec3& v0 = vertices[i + 0].position;
            glm::vec3& v1 = vertices[i + 1].position;
            glm::vec3& v2 = vertices[i + 2].position;

            tris.emplace_back(
                BvhVec3(v0.x, v0.y, v0.z),
                BvhVec3(v1.x, v1.y, v1.z),
                BvhVec3(v2.x, v2.y, v2.z)
            );
        }

        // Get triangle centers and bounding boxes
        std::vector<BBox> bboxes(tris.size());
        std::vector<BvhVec3> centers(tris.size());
        g_executor.for_each(0, tris.size(), [&](size_t begin, size_t end) {
            for (size_t i = begin; i < end; ++i) {
                bboxes[i] = tris[i].get_bbox();
                centers[i] = tris[i].get_center();
            }
        });

   
        typename bvh::v2::DefaultBuilder<BvhNode>::Config config;
        config.quality = bvh::v2::DefaultBuilder<BvhNode>::Quality::Medium;
        {
            //Timer timer("Build() " + std::to_string(vertices.size()) + " verts");
            bvh = bvh::v2::DefaultBuilder<BvhNode>::build(g_threadPool, bboxes, centers, config);
        }
       
        // This precomputes data to speed up traversal
        g_executor.for_each(0, tris.size(), [&](size_t begin, size_t end) {
            for (size_t i = begin; i < end; ++i) {
                auto j = bvh.prim_ids[i];
                precomputedTris[i] = tris[j];
            }
        });

        return uniqueId;
    }

    uint64_t CreateSceneBvh(std::vector<ObjectInstanceData>& instances) {
        Timer timer("CreateSceneBvh()");

        if (instances.empty()) return 0;

        uint64_t uniqueId = UniqueID::GetNext();
        SceneBvh& sceneBvh = g_sceneBvhs[uniqueId];
        sceneBvh.m_instances = instances;
        sceneBvh.m_bboxes.resize(instances.size());
        sceneBvh.m_centers.resize(instances.size());

        for (int i = 0; i < instances.size(); i++) {
            ObjectInstanceData& instance = instances[i];
            BvhVec3 aabbMin = GlmVec3ToBvhVec3(instance.worldAabbBoundsMin);
            BvhVec3 aabbMax = GlmVec3ToBvhVec3(instance.worldAabbBoundsMax);
            BvhVec3 center = GlmVec3ToBvhVec3(instance.worldAabbCenter);
            sceneBvh.m_bboxes[i] = BBox(aabbMin, aabbMax);
            sceneBvh.m_centers[i] = center;
        }

        typename bvh::v2::DefaultBuilder<BvhNode>::Config config;
        config.quality = bvh::v2::DefaultBuilder<BvhNode>::Quality::High;
        config.max_leaf_size = 1;
        sceneBvh.m_bvh = bvh::v2::DefaultBuilder<BvhNode>::build(g_threadPool, sceneBvh.m_bboxes, sceneBvh.m_centers, config);

        return uniqueId;
    }

    SceneRayTraversalResult ClosestHitScene(SceneBvh& sceneBvh, glm::vec3 rayOrigin, glm::vec3 rayDir, float maxDistance) {
        //Timer timer("ClosestHitScene()");
        constexpr float minDistance = 0.0001f;

        // Create ray for library
        Ray ray = Ray{
            BvhVec3(rayOrigin.x, rayOrigin.y, rayOrigin.z),
            BvhVec3(rayDir.x, rayDir.y, rayDir.z),
            minDistance,
            maxDistance
        };

        // Compute ray data for our manual ray vs AABB test
        RayData rayData = ComputeRayData(rayOrigin, rayDir, minDistance, maxDistance);

        // Track the closest hit distance and associated IDs
        float bestDistance = maxDistance;
        size_t bestInstanceIndex = std::numeric_limits<size_t>::max();
        size_t bestPrimitiveId = std::numeric_limits<size_t>::max();
        glm::vec3 bestHitPos(0.0f);

        Bvh& bvh = sceneBvh.m_bvh;

        constexpr size_t stack_size = 64;

        // Traverse the BVH and get the closest hit
        bvh::v2::SmallStack<Bvh::Index, stack_size> stack;
        Bvh::Index rootNodeIndex = bvh.get_root().index;

        // Call back for leaf node ray hit
        auto leafCallback = [&](size_t begin, size_t end) -> bool {
            for (size_t i = begin; i < end; ++i) {
                //std::cout << "callback triggered\n";

                size_t instanceIndex = bvh.prim_ids[i];
                const ObjectInstanceData& instance = sceneBvh.m_instances[instanceIndex];

                // Perform a bounding-box intersection in world space
                float t;
                if (IntersectNodeDistance(rayData, instance.worldAabbBoundsMin, instance.worldAabbBoundsMax, t)) {

                    // If this bounding box is closer than what we've found so far, do a narrow-phase test
                    if (t < bestDistance) {
                        RayTraversalResult localResult = ClosestHitTriangleMesh(instance.triangleMeshBvhId, rayOrigin, rayDir, maxDistance, instance.worldTransform);

                        // If a triangle was actually hit and it's closer, update the best
                        if (localResult.hitFound) {
                            float dist = glm::distance(rayOrigin, localResult.hitPosition);
                            if (dist < bestDistance) {
                                bestDistance = dist;
                                bestInstanceIndex = instanceIndex;
                                bestPrimitiveId = localResult.primtiviveId;
                                bestHitPos = localResult.hitPosition;
                            }
                        }
                    }
                    //std::cout << "aabb hit\n";
                }
            }
            // Always return false so that the BVH traversal continues and processes all leaves to find the closest hit
            return false;
        };

        // Cast the ray
        bvh.intersect<false, false>(ray, rootNodeIndex, stack, leafCallback);

        SceneRayTraversalResult result;

        // On hit, store hit data
        if (bestInstanceIndex != std::numeric_limits<size_t>::max()) {
            result.hitFound = true;
            result.instanceIndex = bestInstanceIndex;
            result.primtiviveId = bestPrimitiveId;
            result.hitPosition = bestHitPos;
        }

        return result;
    }

    RayTraversalResult ClosestHitTriangleMesh(TriangleMeshBvh& triangleMeshBvh, glm::vec3 rayOrigin, glm::vec3 rayDir, float maxDistance, glm::mat4 meshTransform) {
        RayTraversalResult result;

        glm::mat4 inverseTransform = glm::inverse(meshTransform);
        const float globalMinDistance = 0.001f;

        glm::vec3 localOrigin = glm::vec3(inverseTransform * glm::vec4(rayOrigin, 1.0f));
        glm::vec3 localEnd = glm::vec3(inverseTransform * glm::vec4(rayOrigin + rayDir * maxDistance, 1.0f));
        glm::vec3 localDir = glm::normalize(localEnd - localOrigin);
        float localMaxDistance = glm::length(localEnd - localOrigin);
        float localMinDistance = globalMinDistance * localMaxDistance / maxDistance;

        Ray ray = Ray{
            BvhVec3(localOrigin.x, localOrigin.y, localOrigin.z),
            BvhVec3(localDir.x, localDir.y, localDir.z),
            localMinDistance,
            localMaxDistance
        };

        Bvh& bvh = triangleMeshBvh.m_bvh;
        PreComputedTris& precomputedTris = triangleMeshBvh.m_precomputedTris;

        constexpr size_t stack_size = 64;
        constexpr size_t invalidID = std::numeric_limits<size_t>::max();
        size_t primitiveID = std::numeric_limits<size_t>::max();
        Scalar u = 0;
        Scalar v = 0;

        // Traverse the BVH and get the u, v coordinates of the closest intersection.
        bvh::v2::SmallStack<Bvh::Index, stack_size> stack;
        Bvh::Index rootNodeIndex = bvh.get_root().index;

        // Call back for leaf node ray hit
        auto leafCallback = [&](size_t begin, size_t end) -> bool {
            for (size_t i = begin; i < end; ++i) {
                size_t j = i;
                if (auto hit = precomputedTris[j].intersect(ray)) {
                    primitiveID = bvh.prim_ids[i];
                    std::tie(ray.tmax, u, v) = *hit;
                }
            }
            return primitiveID != invalidID;
        };

        // Cast the ray
        bvh.intersect<false, false>(ray, rootNodeIndex, stack, leafCallback);

        // On hit, store hit data
        if (primitiveID != invalidID) {
            result.hitFound = true;
            result.primtiviveId = primitiveID;
            result.hitPosition = rayOrigin + (rayDir * ray.tmax);
        }

        return result;
    }

    void RenderBvhFlat(const Bvh& bvh, glm::vec4& color) {
        glm::mat4 identity = glm::mat4(1.0f);
        for (const auto& node : bvh.nodes) {
            BBox bbox = node.get_bbox();
            glm::vec3 aabbMin(bbox.min.values[0], bbox.min.values[1], bbox.min.values[2]);
            glm::vec3 aabbMax(bbox.max.values[0], bbox.max.values[1], bbox.max.values[2]);
            AABB aabb(aabbMin, aabbMax);
            Renderer::DrawAABB(aabb, color, identity);
        }
    }

    void RenderBvh(const Bvh& bvh, glm::vec4& color, glm::mat4& worldTransform, uint32_t nodeIndex) {
        if (nodeIndex >= bvh.nodes.size()) {
            std::cout << nodeIndex << " out of range of size " << bvh.nodes.size() << "\n";
            return;
        }
        const BvhNode& node = bvh.nodes[nodeIndex];

        BBox bbox = node.get_bbox();
        glm::vec3 aabbMin(bbox.min.values[0], bbox.min.values[1], bbox.min.values[2]);
        glm::vec3 aabbMax(bbox.max.values[0], bbox.max.values[1], bbox.max.values[2]);
        AABB aabb(aabbMin, aabbMax);
        Renderer::DrawAABB(aabb, color, worldTransform);

        if (!node.is_leaf()) {
            size_t leftChild = node.index.first_id();
            size_t rightChild = leftChild + 1;
            RenderBvh(bvh, color, worldTransform, leftChild);
            RenderBvh(bvh, color, worldTransform, rightChild);
        }
    }

    void RenderTriangleMesh(uint64_t bvhId, glm::vec4 color, glm::mat4 worldTransform) {
        if (TriangleMeshBvhExists(bvhId)) {
            TriangleMeshBvh& triangleMeshBvh = g_triangleMeshBvhs[bvhId];
            for (Tri& tri : triangleMeshBvh.m_tris) {
                glm::vec3 v0 = { tri.p0.values[0], tri.p0.values[1],tri.p0.values[2] };
                glm::vec3 v1 = { tri.p1.values[0], tri.p1.values[1],tri.p1.values[2] };
                glm::vec3 v2 = { tri.p2.values[0], tri.p2.values[1],tri.p2.values[2] };
                v0 = worldTransform * glm::vec4(v0, 1.0f);
                v1 = worldTransform * glm::vec4(v1, 1.0f);
                v2 = worldTransform * glm::vec4(v2, 1.0f);
                Renderer::DrawLine(v0, v1, color);
                Renderer::DrawLine(v1, v2, color);
                Renderer::DrawLine(v2, v0, color);
            }
        }
    }

    void RenderSceneBvh(uint64_t bvhId, glm::vec4 color) {
        glm::mat4 worldTransform = glm::mat4(1.0f);
        if (SceneBvhExists(bvhId)) {
            SceneBvh& sceneMeshBvh = g_sceneBvhs[bvhId];
            RenderBvh(sceneMeshBvh.m_bvh, color, worldTransform);
        }
    }

    void RenderSceneBvhFlat(uint64_t bvhId, glm::vec4 color) {
        glm::mat4 worldTransform = glm::mat4(1.0f);
        if (SceneBvhExists(bvhId)) {
            SceneBvh& sceneMeshBvh = g_sceneBvhs[bvhId];
            RenderBvhFlat(sceneMeshBvh.m_bvh, color);
        }
    }

    void RenderTriangleMeshBvh(uint64_t bvhId, glm::vec4 color, glm::mat4 worldTransform) {
        if (TriangleMeshBvhExists(bvhId)) {
            TriangleMeshBvh& triangleMeshBvh = g_triangleMeshBvhs[bvhId];
            RenderBvh(triangleMeshBvh.m_bvh, color, worldTransform);
        }
    }

    void RenderTriangleMeshTriangle(uint64_t bvhId, uint64_t primtiviveId, glm::vec4 color, glm::mat4 worldTransform) {
        if (TriangleMeshBvhExists(bvhId)) {
            TriangleMeshBvh& triangleMeshBvh = g_triangleMeshBvhs[bvhId];
            Tri& tri = triangleMeshBvh.m_tris[primtiviveId];
            glm::vec3 v0 = { tri.p0.values[0], tri.p0.values[1],tri.p0.values[2] };
            glm::vec3 v1 = { tri.p1.values[0], tri.p1.values[1],tri.p1.values[2] };
            glm::vec3 v2 = { tri.p2.values[0], tri.p2.values[1],tri.p2.values[2] };
            v0 = worldTransform * glm::vec4(v0, 1.0f);
            v1 = worldTransform * glm::vec4(v1, 1.0f);
            v2 = worldTransform * glm::vec4(v2, 1.0f);
            Renderer::DrawLine(v0, v1, color);
            Renderer::DrawLine(v1, v2, color);
            Renderer::DrawLine(v2, v0, color);
        }
    }

    glm::vec3 BvhVec3ToGlmVec3(BvhVec3 vec) {
        return { vec.values[0], vec.values[2], vec.values[2] };
    }

    BvhVec3 GlmVec3ToBvhVec3(glm::vec3 vec) {
        return { vec.x, vec.y, vec.z };
    }

    bool TriangleMeshBvhExists(uint64_t bvhId) {
        return (g_triangleMeshBvhs.find(bvhId) != g_triangleMeshBvhs.end());
    }

    bool SceneBvhExists(uint64_t bvhId) {
        return (g_sceneBvhs.find(bvhId) != g_sceneBvhs.end());
    }

    void DestroySceneBvh(uint64_t bvhId) {
        auto it = g_sceneBvhs.find(bvhId);
        if (it != g_sceneBvhs.end()) {
            g_sceneBvhs.erase(it);
        }
    }

    void DestroyTriangleMeshBvh(uint64_t bvhId) {
        auto it = g_triangleMeshBvhs.find(bvhId);
        if (it != g_triangleMeshBvhs.end()) {
            g_triangleMeshBvhs.erase(it);
        }
    }

    uint32_t FloatToBits(float x) {
        uint32_t u;
        memcpy(&u, &x, sizeof(float));
        return u;
    }

    float BitsToFloat(uint32_t u) {
        float x;
        memcpy(&x, &u, sizeof(float));
        return x;
    }

    // ULP, or "Unit in the Last Place", is the smallest difference between two consecutive representable float values at the magnitude of x
    float AddUlpMagnitude(float x, unsigned ulps) {
        return isfinite(x) ? BitsToFloat(FloatToBits(x) + ulps) : x;
    }

    RayData ComputeRayData(const glm::vec3& rayOrigin, const glm::vec3& rayDir, float minDistance, float maxDistance) {
        RayData rayData;
        rayData.origin[0] = rayOrigin.x;
        rayData.origin[1] = rayOrigin.y;
        rayData.origin[2] = rayOrigin.z;
        rayData.dir[0] = rayDir.x;
        rayData.dir[1] = rayDir.y;
        rayData.dir[2] = rayDir.z;
        rayData.minDistance = minDistance;
        rayData.maxDistance = maxDistance;

        // Compute octant
        rayData.octant[0] = signbit(rayDir.x) ? 1 : 0;
        rayData.octant[1] = signbit(rayDir.y) ? 1 : 0;
        rayData.octant[2] = signbit(rayDir.z) ? 1 : 0;

        // Compute inverse direction
        rayData.invDir[0] = 1.0f / rayDir.x;
        rayData.invDir[1] = 1.0f / rayDir.y;
        rayData.invDir[2] = 1.0f / rayDir.z;

        // Compute padding inverse direction
        rayData.paddedInvDir[0] = AddUlpMagnitude(rayData.invDir[0], 2);
        rayData.paddedInvDir[1] = AddUlpMagnitude(rayData.invDir[1], 2);
        rayData.paddedInvDir[2] = AddUlpMagnitude(rayData.invDir[2], 2);

        return rayData;
    }

    float IntersectAxisMin(const RayData& rayData, int axis, float p) {
        return (p - rayData.origin[axis]) * rayData.invDir[axis];
    }

    float IntersectAxisMax(const RayData& rayData, int axis, float p) {
        return (p - rayData.origin[axis]) * rayData.paddedInvDir[axis];
    }

    float RobustMin(float x, float y) {
        return x < y ? x : y;
    }

    float RobustMax(float x, float y) {
        return x > y ? x : y;
    }

    bool IntersectNodeDistance(const RayData& rayData, const glm::vec3& aabbBoundsMin, const glm::vec3& aabbBoundsMax, float& t) {
        float bounds[6] = {
            aabbBoundsMin.x,
            aabbBoundsMax.x,
            aabbBoundsMin.y,
            aabbBoundsMax.y,
            aabbBoundsMin.z,
            aabbBoundsMax.z
        };

        float tmin_x = IntersectAxisMin(rayData, 0, bounds[0 + rayData.octant[0]]);
        float tmin_y = IntersectAxisMin(rayData, 1, bounds[2 + rayData.octant[1]]);
        float tmin_z = IntersectAxisMin(rayData, 2, bounds[4 + rayData.octant[2]]);
        float tmax_x = IntersectAxisMax(rayData, 0, bounds[0 + 1 - rayData.octant[0]]);
        float tmax_y = IntersectAxisMax(rayData, 1, bounds[2 + 1 - rayData.octant[1]]);
        float tmax_z = IntersectAxisMax(rayData, 2, bounds[4 + 1 - rayData.octant[2]]);

        float tmin = RobustMax(RobustMax(tmin_x, tmin_y), RobustMax(tmin_z, rayData.minDistance));
        float tmax = RobustMin(RobustMin(tmax_x, tmax_y), RobustMin(tmax_z, rayData.maxDistance));

        t = tmin;
        return tmin <= tmax;
    }
}