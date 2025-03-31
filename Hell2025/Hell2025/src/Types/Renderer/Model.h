#pragma once
#include "HellEnums.h"
#include "HellTypes.h"
#include "File/FileFormats.h"
#include <string>
#include <vector>
#include <limits>
#include <glm/glm.hpp>
#include "Mesh.h"

struct Model {
    Model() = default;

    void SetFileInfo(FileInfo fileInfo);
    void AddMeshIndex(uint32_t index);
    void SetLoadingState(LoadingState loadingState);
    void SetName(std::string modelName);
    void SetAABB(glm::vec3 aabbMin, glm::vec3 aabbMax);
    Mesh* GetMeshByIndex(uint32_t index);
    
    const LoadingState& GetLoadingState() const             { return m_loadingState; }
    const FileInfo& GetFileInfo() const                     { return m_fileInfo; }
    const size_t GetMeshCount()  const                      { return m_meshIndices.size(); }
    const glm::vec3& GetAABBMin() const                     { return m_aabbMin; }
    const glm::vec3& GetAABBMax() const                     { return m_aabbMax; }
    const glm::vec3& GetExtents() const                     { return m_aabbMax - m_aabbMin; }
    const std::string GetName() const                       { return m_name; }
    const std::vector<uint32_t>& GetMeshIndices() const     { return m_meshIndices; }

    bool m_awaitingLoadingFromDisk = true;
    std::string m_fullPath = "";
    ModelData m_modelData;


private:
    glm::vec3 m_aabbMin = glm::vec3(std::numeric_limits<float>::max());
    glm::vec3 m_aabbMax = glm::vec3(-std::numeric_limits<float>::max());
    std::string m_name = "undefined";
    std::vector<uint32_t> m_meshIndices;
    FileInfo m_fileInfo;
    LoadingState m_loadingState = LoadingState::AWAITING_LOADING_FROM_DISK;
};
