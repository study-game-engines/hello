#include "BasicDoor.h"
#include "AssetManagement/AssetManager.h"
#include "Math/AABB.h"

BasicDoor::BasicDoor(BasicDoorCreateInfo createInfo) {
    m_parentMatrix = createInfo.parentMatrix;
    m_meshRenderingInfoSet = createInfo.m_meshRenderingInfoList;
    m_localOffsetMatrices.clear();
    m_inverselocalOffsetMatrices.clear();

    for (MeshRenderingInfo& meshRenderInfoInfo : m_meshRenderingInfoSet) {
        Mesh* mesh = AssetManager::GetMeshByIndex(meshRenderInfoInfo.meshIndex);
        if (!mesh) {
            std::cout << "BasicDoor() constructor failed: invalid mesh index\n";
            return;
        }

        AABB aabb = AABB(mesh->aabbMin, mesh->aabbMax);
        Transform orignOffsetTransform;
        orignOffsetTransform.position = aabb.GetCenter();
        orignOffsetTransform.position.y += aabb.GetExtents().y * 0.5f;
        orignOffsetTransform.position.z -= aabb.GetExtents().z * 0.5f;
        

    }
    //m_material = createInfo.parentMatrix;
}