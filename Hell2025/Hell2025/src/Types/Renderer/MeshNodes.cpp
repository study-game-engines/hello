#include "MeshNodes.h"
#include "AssetManagement/AssetManager.h"
#include "Renderer/RenderDataManager.h"
#include "Input/Input.h"
#include "Util.h"

void MeshNodes::InitFromModel(Model* model) {
    CleanUp();

    if (!model) return;

    size_t meshCount = model->GetMeshCount();
    int32_t materialIndex = AssetManager::GetMaterialIndexByName(DEFAULT_MATERIAL_NAME);

    m_blendingModes.resize(meshCount, BlendingMode::NONE);
    m_materialIndices.resize(meshCount, materialIndex);
    m_transforms.resize(meshCount, Transform());
    m_localParentIndices.resize(meshCount, -1);
    m_localTransforms.resize(meshCount, glm::mat4(1.0f));
    m_inverseBindTransforms.resize(meshCount, glm::mat4(1.0f));
    m_modelMatrices.resize(meshCount, glm::mat4(1.0f));
    m_objectTypes.resize(meshCount, ObjectType::UNDEFINED);
    m_objectIds.resize(meshCount, 0);

    // Map mesh names to their local index and extra parent index and transforms
    for (int i = 0; i < meshCount; i++) {
        int meshIndex = model->GetMeshIndices()[i];

        Mesh* mesh = AssetManager::GetMeshByIndex(meshIndex);
        if (mesh) {
            m_globalMeshIndices.push_back(meshIndex);
            m_localIndexMap[mesh->GetName()] = i;
            m_localParentIndices[i] = mesh->parentIndex;
            m_localTransforms[i] = mesh->localTransform;
            m_inverseBindTransforms[i] = mesh->inverseBindTransform;

            // Update parent
            if (mesh->parentIndex != -1) {
                Mesh* parentMesh = AssetManager::GetMeshByIndex(meshIndex);
            }
        }
    }
}

bool MeshNodes::HasNodeWithObjectId(uint64_t objectId) const {
    for (uint64_t queryId : m_objectIds) {
        if (queryId == objectId) {
            return true;
        }
    }
    return false;
}

void MeshNodes::CleanUp() {
    m_blendingModes.clear();
    m_globalMeshIndices.clear();
    m_materialIndices.clear();
    m_transforms.clear();
    m_localIndexMap.clear();
    m_localParentIndices.clear();
    m_localTransforms.clear();
    m_inverseBindTransforms.clear();
    m_modelMatrices.clear();
    m_objectTypes.clear();
    m_objectIds.clear();
}

void MeshNodes::SetTransformByMeshName(const std::string& meshName, Transform transform) {
    if (m_localIndexMap.find(meshName) != m_localIndexMap.end()) {
        int nodeIndex = m_localIndexMap[meshName];

        if (nodeIndex >= 0 && nodeIndex < GetNodeCount()) {
            m_transforms[nodeIndex] = transform;
        }
    }
}

RenderItem* MeshNodes::GetRenderItemByNodeIndex(int nodeIndex) {
    if (nodeIndex >= 0 && nodeIndex < m_renderItems.size()) {
        return &m_renderItems[nodeIndex];
    }
    else {
        return nullptr;
    }
}

int32_t MeshNodes::GetGlobalMeshIndex(int nodeIndex) {
    if (nodeIndex >= 0 && nodeIndex < GetNodeCount()) {
        return m_globalMeshIndices[nodeIndex];
    }
    else { 
        return -1; 
    }
}

Material* MeshNodes::GetMaterial(int nodeIndex) {
    if (nodeIndex >= 0 && nodeIndex < GetNodeCount()) {
        return AssetManager::GetMaterialByIndex(m_materialIndices[nodeIndex]);
    }
    else {
        return AssetManager::GetDefaultMaterial();
    }
}

glm::mat4 MeshNodes::GetLocalTransform(int nodeIndex) {
    if (nodeIndex >= 0 && nodeIndex < GetNodeCount()) {
        return m_localTransforms[nodeIndex];
    }
    else {
        return glm::mat4(1.0f);
    }
}

glm::mat4 MeshNodes::GetInverseBindTransform(int nodeIndex) {
    if (nodeIndex >= 0 && nodeIndex < GetNodeCount()) {
        return m_inverseBindTransforms[nodeIndex];
    }
    else {
        return glm::mat4(1.0f);
    }
}

void MeshNodes::UpdateHierachy() {
    for (int i = 0; i < GetNodeCount(); i++) {
        int32_t parentIndex = m_localParentIndices[i];
        if (parentIndex != -1) {
            m_modelMatrices[i] = m_modelMatrices[parentIndex] * m_localTransforms[i] * m_transforms[i].to_mat4();
        }
        else {
            m_modelMatrices[i] = m_localTransforms[i] * m_transforms[i].to_mat4();
        }
    }
}

void MeshNodes::UpdateRenderItems(const glm::mat4& worldMatrix) {
    UpdateHierachy();

    m_renderItems.clear();

    for (int i = 0; i < GetNodeCount(); i++) {

        Material* material = GetMaterial(i);
        if (!material) continue;

        int32_t meshIndex = GetGlobalMeshIndex(i);
        Mesh* mesh = AssetManager::GetMeshByIndex(meshIndex);
        if (!mesh) continue;

        glm::mat4 meshModelMatrix = GetMeshModelMatrix(i);

        // Remove me
        if (Input::KeyDown(HELL_KEY_T)) {
            meshModelMatrix = glm::mat4(1.0f);
        }
        // Remove me

        RenderItem& renderItem = m_renderItems.emplace_back();
        renderItem.objectType = (int)m_objectTypes[i];
        renderItem.modelMatrix = worldMatrix * meshModelMatrix;
        renderItem.inverseModelMatrix = glm::inverse(renderItem.modelMatrix);
        renderItem.meshIndex = meshIndex;
        renderItem.baseColorTextureIndex = material->m_basecolor;
        renderItem.normalMapTextureIndex = material->m_normal;
        renderItem.rmaTextureIndex = material->m_rma;
        Util::PackUint64(m_objectIds[i], renderItem.objectIdLowerBit, renderItem.objectIdUpperBit);
        Util::UpdateRenderItemAABB(renderItem);
    }
}

void MeshNodes::SubmitRenderItems() {
    for (RenderItem& renderItem : m_renderItems) {
        RenderDataManager::SubmitRenderItem(renderItem);
    }
}

glm::mat4 MeshNodes::GetMeshModelMatrix(int nodeIndex) {
    if (nodeIndex >= 0 && nodeIndex < GetNodeCount()) {
        return m_modelMatrices[nodeIndex];
    }
    else {
        return glm::mat4(1.0f);
    }
}