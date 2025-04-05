#include "PickUp.h"
#include "AssetManagement/AssetManager.h"
#include "Physics/Physics.h"
#include "UniqueID.h"
#include "Util.h"

#include "Input/Input.h"
#include "World/World.h"

void PickUp::Init(PickUpCreateInfo createInfo) {
    m_transform.position = createInfo.position;
    m_transform.rotation = createInfo.rotation;
    m_pickUpType = Util::StringToPickUpType(createInfo.pickUpType);

    PhysicsFilterData filterData;
    filterData.raycastGroup = RaycastGroup::RAYCAST_ENABLED;
    filterData.collisionGroup = CollisionGroup::ITEM_PICK_UP;
    filterData.collidesWith = CollisionGroup::ENVIROMENT_OBSTACLE;

    // Shotty buckshot
    if (m_pickUpType == PickUpType::SHOTGUN_AMMO_BUCKSHOT) {
        SetModel("Shotgun_AmmoBox");
        SetAllMeshMaterials("Shotgun_AmmoBox");
        float mass = 0.45f;
        m_physicsId = Physics::CreateRigidDynamicFromBoxExtents(m_transform, m_model->GetExtents(), mass, filterData);
    }
    // Shotty slug
    else if (m_pickUpType == PickUpType::SHOTGUN_AMMO_SLUG) {
        SetModel("Shotgun_AmmoBox");
        SetAllMeshMaterials("Shotgun_AmmoBoxSlug");
        float mass = 0.45f;
        m_physicsId = Physics::CreateRigidDynamicFromBoxExtents(m_transform, m_model->GetExtents(), mass, filterData);
    }
    // AKS74U
    else if (m_pickUpType == PickUpType::AKS74U) {
        SetModel("AKS74U_PickUp");
        SetMeshMaterial("Mesh0", "AKS74U_0");
        SetMeshMaterial("Mesh1", "AKS74U_1");
        SetMeshMaterial("Mesh2", "AKS74U_2");
        SetMeshMaterial("Mesh3", "AKS74U_3");
        SetMeshMaterial("Mesh4", "AKS74U_4");
        float mass = 2.7f;

        // Convex mesh
        Model* model = AssetManager::GetModelByName("AKS74U_PickUpConvexMesh");
        if (model) {
            int32_t meshIndex = model->GetMeshIndices()[0];
            Mesh* mesh = AssetManager::GetMeshByIndex(meshIndex);
            if (mesh) {
                std::span<Vertex> vertices = AssetManager::GetVerticesSpan(mesh->baseVertex, mesh->vertexCount);
                std::span<uint32_t> indices = AssetManager::GetIndicesSpan(mesh->baseIndex, mesh->indexCount);
                m_physicsId = Physics::CreateRigidDynamicFromConvexMeshVertices(m_transform, vertices, indices, mass, filterData);
            }
        }
    }

    // Remington 870
    else if (m_pickUpType == PickUpType::REMINGTON_870) {
        SetModel("Shotgun_PickUp");
        SetAllMeshMaterials("Shotgun");
        float mass = 3.2f;

        // Convex mesh
        Model* model = AssetManager::GetModelByName("Shotgun_PickUpConvexMesh");
        if (model) {
            int32_t meshIndex = model->GetMeshIndices()[0];
            Mesh* mesh = AssetManager::GetMeshByIndex(meshIndex);
            if (mesh) {
                std::span<Vertex> vertices = AssetManager::GetVerticesSpan(mesh->baseVertex, mesh->vertexCount);
                std::span<uint32_t> indices = AssetManager::GetIndicesSpan(mesh->baseIndex, mesh->indexCount);
                m_physicsId = Physics::CreateRigidDynamicFromConvexMeshVertices(m_transform, vertices, indices, mass, filterData);
            }
        }
    }

    // Get next unique ID
    m_objectId = UniqueID::GetNext();

    // Set PhysX user data
    PhysicsUserData userData;
    userData.physicsId = m_physicsId;
    userData.objectId = m_objectId;
    userData.physicsType = PhysicsType::RIGID_DYNAMIC;
    userData.objectType = ObjectType::PICK_UP;
    Physics::SetRigidDynamicUserData(m_physicsId, userData);
}

void PickUp::SetModel(const std::string& modelName) {
    Model* model = AssetManager::GetModelByName(modelName);
    if (!model) {
        std::cout << "PickUp::SetModel() failed: '" << modelName << "' was not found\n";
    }
    m_model = model;
    m_materialIndices.resize(m_model->GetMeshCount());
}

void PickUp::SetMeshMaterial(const std::string& meshName, const std::string& materialName) {
    if (!m_model) {
        std::cout << "PickUp::SetMeshMaterial() failed: m_model was nullptr\n";
        return;
    }
    int32_t materialIndex = AssetManager::GetMaterialIndexByName(materialName);
    if (materialIndex == -1) {
        std::cout << "PickUp::SetMeshMaterial() failed: material '" << materialName << "' was not found\n";
        return;
    }

    for (int i = 0; i < m_model->GetMeshCount(); i++) {
        int32_t meshIndex = m_model->GetMeshIndices()[i];
        Mesh* mesh = AssetManager::GetMeshByIndex(meshIndex);
        if (mesh && mesh->GetName() == meshName) {
            m_materialIndices[i] = materialIndex;
            return;
        }
    }
    // If you made it this far, mesh name was nut found
    std::cout << "PickUp::SetMeshMaterial() failed: mesh name '" << meshName << "' was not found in m_model's mesh list\n";
}

void PickUp::SetAllMeshMaterials(const std::string& materialName) {
    int32_t materialIndex = AssetManager::GetMaterialIndexByName(materialName);
    if (materialIndex == -1) {
        std::cout << "PickUp::SetAllMeshMaterials() failed: material '" << materialName << "' was not found\n";
        return;
    }
    for (int i = 0; i < m_materialIndices.size(); i++) {
        m_materialIndices[i] = materialIndex;
    }
}

PickUpCreateInfo PickUp::GetCreateInfo() {
    PickUpCreateInfo createInfo;
    createInfo.position = m_transform.position;
    createInfo.rotation = m_transform.rotation;
    createInfo.pickUpType = Util::PickUpTypeToString(m_pickUpType);
    return createInfo;
}

void PickUp::Update(float deltaTime) {
    m_modelMatrix = m_transform.to_mat4();

    // If physx object exists, rip the model matrix out
    if (Physics::RigidDynamicExists(m_physicsId)) {
        m_modelMatrix = Physics::GetRigidDynamicWorldMatrix(m_physicsId);
    }

    UpdateRenderItems();
}

void PickUp::CleanUp() {
    Physics::MarkRigidDynamicForRemoval(m_physicsId);
}

void PickUp::SetMousePickIndex(int mousePickIndex) {
    m_mousePickIndex = mousePickIndex;
}

void PickUp::SetPosition(glm::vec3 position) {
    m_transform.position = position;
}

void PickUp::UpdateRenderItems() {
    m_renderItems.clear();    

    for (int i = 0; i < m_model->GetMeshCount(); i++) {
        Mesh* mesh = AssetManager::GetMeshByIndex(m_model->GetMeshIndices()[i]);
        if (mesh) {
            Material* material = AssetManager::GetMaterialByIndex(m_materialIndices[i]);
            RenderItem& renderItem = m_renderItems.emplace_back();
            renderItem.objectType = (int)ObjectType::PICK_UP;
            renderItem.mousePickIndex = m_mousePickIndex;
            renderItem.modelMatrix = GetModelMatrix();
            renderItem.inverseModelMatrix = glm::inverse(renderItem.modelMatrix);
            renderItem.meshIndex = m_model->GetMeshIndices()[i];       
            renderItem.baseColorTextureIndex = material->m_basecolor;
            renderItem.normalMapTextureIndex = material->m_normal;
            renderItem.rmaTextureIndex = material->m_rma;
            Util::PackUint64(m_objectId, renderItem.objectIdLowerBit, renderItem.objectIdUpperBit);
            Util::UpdateRenderItemAABB(renderItem);
        }
    }
}
