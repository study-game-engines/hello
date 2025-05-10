#pragma once
#include "HellTypes.h"
#include <vector>
#include <map>

inline const char HEIGHT_MAP_SIGNATURE[] = "HELL_HEIGHT_MAP";
#define HELL_NAME_BUFFER_SIZE 256
#define HELL_SIGNATURE_BUFFER_SIZE 32
#define HELL_MODEL_BVH_SIGNATURE "HELL_MODEL_BVH"
#define HELL_MODEL_SIGNATURE "HELL_MODEL"
#define HELL_MESH_SIGNATURE "HELL_MESH"
#define HELL_MESH_BVH_SIGNATURE  "HELL_MESH_BVH"

#pragma pack(push, 1)
struct ModelHeader {
    char signature[10];     // "HELL_MODEL" 10 bytes
    uint32_t version;
    uint32_t meshCount;
    uint32_t nameLength;
    uint64_t timestamp;
    glm::vec3 aabbMin;
    glm::vec3 aabbMax;
};

struct ModelHeaderV2 {
    char signature[32];
    uint32_t version;
    uint32_t meshCount;
    uint64_t timestamp;
    glm::vec3 aabbMin;
    glm::vec3 aabbMax;
};

struct MeshHeaderV2 {
    char signature[32];
    char name[256];
    uint32_t vertexCount;
    uint32_t indexCount;
    int32_t parentIndex;
    glm::vec3 aabbMin;
    glm::vec3 aabbMax;
    glm::mat4 localTransform;
    glm::mat4 inverseBindTransform;
};

struct MeshHeader {
    uint32_t nameLength;
    uint32_t vertexCount;
    uint32_t indexCount;
    glm::vec3 aabbMin;
    glm::vec3 aabbMax;
};

struct SkinnedModelHeader {
    char signature[18];     // "HELL_SKINNED_MODEL" 18 bytes
    uint32_t version;
    uint32_t nameLength;
    uint32_t vertexCount;
    uint32_t indexCount;
    uint32_t meshCount;
    uint32_t nodeCount;
    uint32_t boneCount;
    uint64_t timestamp;
};


struct SkinnedMeshHeader {
    uint32_t nameLength;
    uint32_t vertexCount;
    uint32_t indexCount;
    uint32_t localBaseVertex;
    glm::vec3 aabbMin;
    glm::vec3 aabbMax;
};

struct HeightMapHeader {
    char signature[32];
    char name[256];
    uint32_t width;
    uint32_t height;
};
struct ModelBvhHeader {
    char signature[32]; // "HELL_MODEL_BVH"
    uint64_t version;
    uint64_t meshCount;
    uint64_t timestamp;
};

struct MeshBvhHeader {
    char signature[32]; // "HELL_MESH_BVH"
    uint64_t floatCount;
    uint64_t nodeCount;
};
#pragma pack(pop)

struct MeshData {
    std::string name;
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    glm::vec3 aabbMin = glm::vec3(std::numeric_limits<float>::max());
    glm::vec3 aabbMax = glm::vec3(-std::numeric_limits<float>::max());
    uint32_t vertexCount = 0;
    uint32_t indexCount = 0;
    uint32_t parentIndex = -1;
    glm::mat4 localTransform = glm::mat4(1.0f);
    glm::mat4 inverseBindTransform = glm::mat4(1.0f);
};

struct SkinnedMeshData {
    std::string name;
    std::vector<WeightedVertex> vertices;
    std::vector<uint32_t> indices;
    glm::vec3 aabbMin = glm::vec3(std::numeric_limits<float>::max());
    glm::vec3 aabbMax = glm::vec3(-std::numeric_limits<float>::max());
    uint32_t vertexCount;
    uint32_t indexCount;
    uint32_t localBaseVertex;
};

struct ModelData {
    std::string name;
    uint32_t meshCount;
    uint64_t timestamp;
    std::vector<MeshData> meshes;
    glm::vec3 aabbMin = glm::vec3(std::numeric_limits<float>::max());
    glm::vec3 aabbMax = glm::vec3(-std::numeric_limits<float>::max());
};

struct SkinnedModelData {
    std::string name;
    std::vector<SkinnedMeshData> meshes;
    std::vector<glm::mat4> boneOffsets;
    std::vector<Node> nodes;
    std::map<std::string, unsigned int> boneMapping;
    uint32_t vertexCount;
    uint32_t indexCount;
    uint64_t timestamp;

    const uint32_t GetMeshCount() const { return meshes.size(); };
    const uint32_t GetNodeCount() const { return nodes.size(); };
    const uint32_t GetBoneCount() const { return boneOffsets.size(); };
};

struct HeightMapData {
    std::string name;
    uint32_t width;
    uint32_t height;
    std::vector<float> data;
};

struct ModelBvhData {
    std::vector<MeshBvh> bvhs;
};