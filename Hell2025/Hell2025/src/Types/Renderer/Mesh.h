#pragma once
#include <glm/glm.hpp>
#include <string>

struct Mesh {
    int32_t baseVertex = 0;
    uint32_t baseIndex = 0;
    uint32_t vertexCount = 0;
    uint32_t indexCount = 0;
    glm::vec3 aabbMin = glm::vec3(0);
    glm::vec3 aabbMax = glm::vec3(0);
    glm::vec3 extents = glm::vec3(0);
    uint64_t triangleMeshBvhId = 0;
    float boundingSphereRadius = 0;

    void SetName(const std::string& name);
    int32_t GetIndexCount();
    const std::string& GetName();

private:
    std::string m_name = "undefined";
};
