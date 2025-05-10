#pragma once
#include "HellTypes.h"
#include "Model.h"
#include <vector>
#include <unordered_map>

struct MeshNodes {
    std::vector<BlendingMode> m_blendingModes;
    std::vector<uint32_t> m_globalMeshIndices;
    std::vector<uint32_t> m_materialIndices;
    std::vector<int32_t> m_localParentIndices;
    std::vector<Transform> m_transforms;
    std::vector<glm::mat4> m_modelMatrices;
    std::vector<glm::mat4> m_localTransforms;
    std::vector<glm::mat4> m_inverseBindTransforms;
    std::vector<ObjectType> m_objectTypes;
    std::vector<uint64_t> m_objectIds;
    std::unordered_map<std::string, uint32_t> m_localIndexMap; // maps mesh name to its local index

    void InitFromModel(Model* model);
    void CleanUp();
    void UpdateHierachy();
    void UpdateRenderItems(const glm::mat4& worldMatrix);
    void SubmitRenderItems();
    void SetTransformByMeshName(const std::string& meshName, Transform transform);

    bool HasNodeWithObjectId(uint64_t objectId) const;
        
    RenderItem* GetRenderItemByNodeIndex(int nodeIndex);
    int32_t GetGlobalMeshIndex(int nodeIndex);
    Material* GetMaterial(int nodeIndex);
    glm::mat4 GetLocalTransform(int nodeIndex);
    glm::mat4 GetInverseBindTransform(int nodeIndex);
    glm::mat4 GetMeshModelMatrix(int nodeIndex);

    const size_t GetNodeCount() const                       { return m_globalMeshIndices.size(); }
    const std::vector<RenderItem>& GetRenderItems() const   { return m_renderItems; }

private:
    std::vector<RenderItem> m_renderItems;
};