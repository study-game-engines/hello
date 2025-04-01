#include "Bvh.h"
#include "Input/Input.h"
#include "Renderer/Renderer.h"
#include "HellDefines.h"
#include "UniqueID.h"
#include "Util.h"
#include "Timer.hpp"

#include <iostream>
#include <unordered_map>

#include "bvh/v2/bvh.h"
#include "bvh/v2/vec.h"
#include "bvh/v2/bbox.h"
#include "bvh/v2/ray.h"
#include "bvh/v2/node.h"
#include "bvh/v2/default_builder.h"
#include "bvh/v2/thread_pool.h"
#include "bvh/v2/stack.h"

using MadmannVec3 = bvh::v2::Vec<float, 3>;
using MadmannBBox = bvh::v2::BBox<float, 3>;
using MadmannBvhNode = bvh::v2::Node<float, 3>;
using MadmannBvh = bvh::v2::Bvh<MadmannBvhNode>;
using MadmannBvhBuilder = bvh::v2::DefaultBuilder<MadmannBvhNode>;

namespace BVH {
    std::unordered_map<uint64_t, MeshBvh> g_meshBvhs;
    std::unordered_map<uint64_t, SceneBvh> g_sceneBvhs;
    bvh::v2::ThreadPool g_threadPool;

    // Gpu data
    std::unordered_map<uint64_t, uint32_t> g_meshBvhRootNodeOffsetMapping; // Maps a flatterend MeshBvh's root node to its id
    std::vector<BvhNode> g_meshBvhsNodes;
    std::vector<BvhNode> g_sceneBvhsNodes;
    std::vector<float> g_triangleData;

    glm::vec3 BvhVec3ToGlmVec3(MadmannVec3 vec);
    MadmannVec3 GlmVec3ToBvhVec3(glm::vec3 vec);

    uint64_t CreateMeshBvhFromVertices(std::vector<Vertex>& vertices) {
        Timer timer("CreateBvhFromVertices() " + std::to_string(vertices.size()) + " verts");

        uint64_t uniqueId = UniqueID::GetNext();
        MeshBvh& meshBvh = g_meshBvhs[uniqueId];

        assert(vertices.size() % 3 == 0 && "Vertex count must be a multiple of 3");

        size_t triangleCount = vertices.size() / 3;
        std::vector<MadmannBBox> bboxes(triangleCount);
        std::vector<MadmannVec3> centers(triangleCount);

        for (int i = 0; i < triangleCount; ++i) {
            // Access the vertices for the current triangle
            int vertexIndex = i * 3;
            glm::vec3& v0 = vertices[vertexIndex + 0].position;
            glm::vec3& v1 = vertices[vertexIndex + 1].position;
            glm::vec3& v2 = vertices[vertexIndex + 2].position;

            // Calculate bounding box and center
            glm::vec3 min_p = glm::min(glm::min(v0, v1), v2);
            glm::vec3 max_p = glm::max(glm::max(v0, v1), v2);
            glm::vec3 center_p = (v0 + v1 + v2) / 3.0f;

            bboxes[i] = MadmannBBox(MadmannVec3(min_p.x, min_p.y, min_p.z), MadmannVec3(max_p.x, max_p.y, max_p.z));
            centers[i] = MadmannVec3(center_p.x, center_p.y, center_p.z);
        }

        MadmannBvhBuilder::Config config;
        config.quality = MadmannBvhBuilder::Quality::High;
        MadmannBvh bvh = MadmannBvhBuilder::build(g_threadPool, bboxes, centers, config);

        // Create our node array
        int nodeCount = bvh.nodes.size();
        meshBvh.m_nodes.resize(nodeCount);

        // .. by iterating the bvh created by the library and extracing the relevant data
        for (int i = 0; i < nodeCount; i++) {
            const MadmannBvhNode& mmNode = bvh.nodes[i];
            BvhNode& node = meshBvh.m_nodes[i];
            node.boundsMin = glm::vec3(mmNode.bounds[0], mmNode.bounds[2], mmNode.bounds[4]);
            node.boundsMax = glm::vec3(mmNode.bounds[1], mmNode.bounds[3], mmNode.bounds[5]);
            node.primitiveCount = mmNode.index.value & ((1u << MadmannBvhNode::prim_count_bits) - 1);
            node.firstChildOrPrimitive = mmNode.index.value >> MadmannBvhNode::prim_count_bits;
        }

        // Create the re-ordered triangle array
        int floatCount = vertices.size() / 3 * 3 * 4;
        meshBvh.m_triangleData.reserve(floatCount);

        // .. by walking the bvh and re-order triangles so that leaf node tris are adjacent, and updating the node indices
        uint32_t stack[MAX_BVH_STACK_SIZE];
        size_t stack_size = 0;
        stack[stack_size++] = 0;
        while (stack_size != 0) {
            --stack_size;
            BvhNode& node = meshBvh.m_nodes[stack[stack_size]];

            // Iterate primitives
            if (node.primitiveCount > 0) {
                int newPrimitiveIndex = meshBvh.m_triangleData.size();

                for (int i = 0; i < node.primitiveCount; i++) {
                    int primitiveId = node.firstChildOrPrimitive + i;
                    int triangleIndex = bvh.prim_ids[primitiveId];

                    int vertexIndex = triangleIndex * 3;
                    glm::vec3& p0 = vertices[vertexIndex + 0].position;
                    glm::vec3& p1 = vertices[vertexIndex + 1].position;
                    glm::vec3& p2 = vertices[vertexIndex + 2].position;

                    // Precompute edges
                    glm::vec3 e1 = p0 - p1;
                    glm::vec3 e2 = p2 - p0;

                    // Precompute normal
                    glm::vec3 normal = cross(e1, e2);

                    // Store p0
                    meshBvh.m_triangleData.push_back(p0.x);
                    meshBvh.m_triangleData.push_back(p0.y);
                    meshBvh.m_triangleData.push_back(p0.z);

                    // Store e1
                    meshBvh.m_triangleData.push_back(e1.x);
                    meshBvh.m_triangleData.push_back(e1.y);
                    meshBvh.m_triangleData.push_back(e1.z);

                    // Store e2
                    meshBvh.m_triangleData.push_back(e2.x);
                    meshBvh.m_triangleData.push_back(e2.y);
                    meshBvh.m_triangleData.push_back(e2.z);

                    // Store normal
                    meshBvh.m_triangleData.push_back(normal.x);
                    meshBvh.m_triangleData.push_back(normal.y);
                    meshBvh.m_triangleData.push_back(normal.z);
                }

                // Update node primitive with new index
                node.firstChildOrPrimitive = newPrimitiveIndex;
            }
            // The node is internal, so recurse
            else {
                stack[stack_size++] = node.firstChildOrPrimitive + 0;
                stack[stack_size++] = node.firstChildOrPrimitive + 1;
            }
        }
    
        return uniqueId;
    }

    uint64_t CreateNewSceneBvh() {
        uint64_t uniqueId = UniqueID::GetNext();
        SceneBvh& sceneBvh = g_sceneBvhs[uniqueId];
        return uniqueId;
    }

    void UpdateSceneBvh(uint64_t bvhId, std::vector<PrimitiveInstance>& instances) {
        if (instances.empty()) return;
        //Timer timer("UpdateSceneBvh() " + std::to_string(instances.size()) + " instances");

        SceneBvh& sceneBvh = g_sceneBvhs[bvhId];

        std::vector<MadmannBBox> bboxes(instances.size());
        std::vector<MadmannVec3> centers(instances.size());

        for (int i = 0; i < instances.size(); i++) {
            PrimitiveInstance& instance = instances[i];
            MadmannVec3 aabbMin = GlmVec3ToBvhVec3(instance.worldAabbBoundsMin);
            MadmannVec3 aabbMax = GlmVec3ToBvhVec3(instance.worldAabbBoundsMax);
            MadmannVec3 center = GlmVec3ToBvhVec3(instance.worldAabbCenter);
            bboxes[i] = MadmannBBox(aabbMin, aabbMax);
            centers[i] = center;
        }

        typename bvh::v2::DefaultBuilder<MadmannBvhNode>::Config config;
        config.quality = bvh::v2::DefaultBuilder<MadmannBvhNode>::Quality::High;
        MadmannBvh bvh = bvh::v2::DefaultBuilder<MadmannBvhNode>::build(g_threadPool, bboxes, centers, config);

        int nodeCount = bvh.nodes.size();
        sceneBvh.m_nodes.resize(nodeCount);

        for (int i = 0; i < nodeCount; i++) {
            const MadmannBvhNode& mmNode = bvh.nodes[i];
            BvhNode& node = sceneBvh.m_nodes[i];
            node.boundsMin = glm::vec3(mmNode.bounds[0], mmNode.bounds[2], mmNode.bounds[4]);
            node.boundsMax = glm::vec3(mmNode.bounds[1], mmNode.bounds[3], mmNode.bounds[5]);
            node.primitiveCount = mmNode.index.value & ((1u << MadmannBvhNode::prim_count_bits) - 1);
            node.firstChildOrPrimitive = mmNode.index.value >> MadmannBvhNode::prim_count_bits;
        }

        sceneBvh.m_instances.clear();
        sceneBvh.m_instances.reserve(instances.size());

        // Walk the scene BVH and create the entity instance array, specifically ordering them such that leaf node instances are adjacent
        uint32_t stack[MAX_BVH_STACK_SIZE];
        size_t stack_size = 0;
        uint32_t rootNodeIndex = 0;
        stack[stack_size++] = rootNodeIndex;

        while (stack_size != 0) {
            uint32_t currentIndex = stack[--stack_size];
            BvhNode& node = sceneBvh.m_nodes[currentIndex];

            // If this node is a leaf...
            if (node.primitiveCount > 0) {
                int newPrimitiveIndex = static_cast<int>(sceneBvh.m_instances.size());

                // Loop over each primitive instance in the leaf
                for (int i = 0; i < node.primitiveCount; i++) {
                    int primitiveId = node.firstChildOrPrimitive + i;
                    int instanceIndex = bvh.prim_ids[primitiveId];

                    const PrimitiveInstance& instance = instances[instanceIndex];

                    // Create the GPU primitive instance
                    GpuPrimitiveInstance& gpuInstance = sceneBvh.m_instances.emplace_back();
                    gpuInstance.bvhRootNodeIndex = g_meshBvhRootNodeOffsetMapping[instance.meshBvhId];
                    gpuInstance.worldTransform = instance.worldTransform;
                    gpuInstance.inverseWorldTransform = glm::inverse(gpuInstance.worldTransform);
                }
                // Update the leaf node's pointer so it now points into the new, contiguous instance array
                node.firstChildOrPrimitive = newPrimitiveIndex;
            }
            else {
                stack[stack_size++] = node.firstChildOrPrimitive;
                stack[stack_size++] = node.firstChildOrPrimitive + 1;
            }
        }

        // For now we only have one scene, so always update the global container with it for gpu retrieval
        g_sceneBvhsNodes = sceneBvh.m_nodes;
    }

    void FlatternMeshBvhNodes() {
        g_meshBvhRootNodeOffsetMapping.clear();
        g_meshBvhsNodes.clear();
        g_triangleData.clear();

        // Preallocate memory
        int totalNodeCount = 0;
        int totalTriangleDataSize = 0;
        for (auto it = g_meshBvhs.begin(); it != g_meshBvhs.end(); ++it) {
            MeshBvh& meshBvh = it->second;
            totalNodeCount += meshBvh.m_nodes.size();
            totalTriangleDataSize += meshBvh.m_triangleData.size();
        }
        g_meshBvhsNodes.reserve(totalNodeCount);
        g_triangleData.reserve(totalTriangleDataSize);

        uint32_t rootNodeOffset = 0;
        uint32_t baseTriangleOffset = 0;

        // Itereate each mesh bvh, and store its nodes and triangle data in the global ararys
        for (auto it = g_meshBvhs.begin(); it != g_meshBvhs.end(); ++it) {
            uint64_t bvhId = it->first;
            MeshBvh& meshBvh = it->second;

            // Store the root node and triangle offsets
            g_meshBvhRootNodeOffsetMapping[bvhId] = rootNodeOffset;

            // Append this mesh's nodes to the global vector
            for (BvhNode& node : meshBvh.m_nodes) {

                // Copy the node
                BvhNode& appendedNode = g_meshBvhsNodes.emplace_back(node);

                // If the node is a leaf, add the base triangle offset
                if (appendedNode.primitiveCount > 0) {
                    appendedNode.firstChildOrPrimitive += baseTriangleOffset;
                }
                // If it's not a leaf, then add the root node offset
                else {
                    appendedNode.firstChildOrPrimitive += rootNodeOffset;
                }
            }

            // Copy the triangle data from this mesh into the global vector
            g_triangleData.insert(g_triangleData.end(), meshBvh.m_triangleData.begin(), meshBvh.m_triangleData.end());

            // Increment offsets
            rootNodeOffset += meshBvh.m_nodes.size();
            baseTriangleOffset += meshBvh.m_triangleData.size();
        }
    }

    glm::vec3 BvhVec3ToGlmVec3(MadmannVec3 vec) {
        return { vec.values[0], vec.values[2], vec.values[2] };
    }

    MadmannVec3 GlmVec3ToBvhVec3(glm::vec3 vec) {
        return { vec.x, vec.y, vec.z };
    }

    bool MeshBvhExists(uint64_t bvhId) {
        return (g_meshBvhs.find(bvhId) != g_meshBvhs.end());
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

    void DestroyMeshBvh(uint64_t bvhId) {
        auto it = g_meshBvhs.find(bvhId);
        if (it != g_meshBvhs.end()) {
            g_meshBvhs.erase(it);
        }
    }

    SceneBvh* GetSceneBvhById(uint64_t bvhId) {
        if (!SceneBvhExists(bvhId)) return nullptr;
        return &g_sceneBvhs[bvhId];
    }
    
    MeshBvh* GetMeshBvhById(uint64_t bvhId) {
        if (!MeshBvhExists(bvhId)) return nullptr;
        return &g_meshBvhs[bvhId];
    }

    const std::vector<BvhNode>& GetMeshGpuBvhNodes() {
        return g_meshBvhsNodes;
    }

    const std::vector<BvhNode>& GetSceneGpuBvhNodes() {
        return g_sceneBvhsNodes;
    }

    const std::vector<float>& GetTriangleData() {
        return g_triangleData;
    }

    const std::vector<GpuPrimitiveInstance>& GetGpuEntityInstances(uint64_t sceneBvhId) {
        static std::vector<GpuPrimitiveInstance> empty;

        if (!SceneBvhExists(sceneBvhId)) return empty;
        return g_sceneBvhs[sceneBvhId].m_instances;
    }
}