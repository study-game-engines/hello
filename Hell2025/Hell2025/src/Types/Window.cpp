#include "Window.h"
#include "AssetManagement/AssetManager.h"
#include "Physics/Physics.h"
#include "Physics/Physics.h"
#include "UniqueID.h"
#include <unordered_map>

void Window::Init(WindowCreateInfo createInfo) {
    m_position = createInfo.position;
    m_rotation = createInfo.rotation;

    Transform transform;
    transform.position = m_position;
    transform.rotation = m_rotation;  
    m_modelMatrix = transform.to_mat4();   
        
    m_interiorMaterial = AssetManager::GetMaterialByName("Window");
    m_exteriorMaterial = AssetManager::GetMaterialByName("WindowExterior");

    m_model = AssetManager::GetModelByIndex(AssetManager::GetModelIndexByName("Window"));
    m_glassModel = AssetManager::GetModelByIndex(AssetManager::GetModelIndexByName("WindowGlass"));

    // Glass PhysX shapes
    PhysicsFilterData filterData;
    filterData.raycastGroup = RaycastGroup::RAYCAST_ENABLED;
    filterData.collisionGroup = CollisionGroup::NO_COLLISION;
    filterData.collidesWith = CollisionGroup::NO_COLLISION;

    filterData.collisionGroup = CollisionGroup::ENVIROMENT_OBSTACLE;
    filterData.collidesWith = (CollisionGroup)(GENERIC_BOUNCEABLE | BULLET_CASING | RAGDOLL);

    m_physicsId = Physics::CreateRigidStaticConvexMeshFromModel(transform, "WindowGlass", filterData);
    m_objectId = UniqueID::GetNext();

    // Set PhysX user data
    PhysicsUserData userData;
    userData.physicsId = m_physicsId;
    userData.objectId = m_objectId;
    userData.physicsType = PhysicsType::RIGID_STATIC;
    userData.objectType = ObjectType::WINDOW;
    Physics::SetRigidStaticUserData(m_physicsId, userData);

    // Get next unique ID
    m_objectId = UniqueID::GetNext();
}

void Window::CleanUp() {
    Physics::MarkRigidStaticForRemoval(m_physicsId);
}

void Window::Update(float deltaTime) {
    UpdateRenderItems();
}

void Window::UpdateRenderItems() {
    m_renderItems.clear();
    m_glassRenderItems.clear();

    // Handle missing materials
    if (!m_interiorMaterial) {
        m_interiorMaterial = AssetManager::GetDefaultMaterial();
    }
    if (!m_exteriorMaterial) {
        m_exteriorMaterial = AssetManager::GetDefaultMaterial();
    }

    static std::unordered_map<std::string, Material*> meshMaterialMap;
    if (meshMaterialMap.empty()) {       
        meshMaterialMap["SM_Window_01a"] = m_interiorMaterial;
        meshMaterialMap["SM_Window_FaceA"] = m_interiorMaterial;
        meshMaterialMap["SM_Window_FaceA_Bottom"] = m_interiorMaterial;
        meshMaterialMap["SM_Window_FaceA_Top"] = m_interiorMaterial;
        meshMaterialMap["SM_Window_FaceB"] = m_exteriorMaterial;
        meshMaterialMap["SM_Window_FaceB_Bottom"] = m_exteriorMaterial;
        meshMaterialMap["SM_Window_FaceB_Top"] = m_exteriorMaterial;
    }

    // Bail if model is invalid
    if (!m_model) return;

    for (uint32_t& meshIndex : m_model->GetMeshIndices()) {
        Mesh* mesh = AssetManager::GetMeshByIndex(meshIndex);
        RenderItem& renderItem = m_renderItems.emplace_back();
        renderItem.modelMatrix = m_modelMatrix;
        renderItem.inverseModelMatrix = inverse(renderItem.modelMatrix);
        renderItem.meshIndex = meshIndex;

        // Dirty. Fix me. 
        // Very error prone. Returns invalid pointer if name not found;
        Material* material = meshMaterialMap[mesh->GetName()];

        renderItem.baseColorTextureIndex = material->m_basecolor;
        renderItem.rmaTextureIndex = material->m_rma;
        renderItem.normalMapTextureIndex = material->m_normal;
    }

    // Bail if model is invalid
    if (!m_glassModel) return;
    
    for (uint32_t& meshIndex : m_glassModel->GetMeshIndices()) {
        Mesh* mesh = AssetManager::GetMeshByIndex(meshIndex);
        RenderItem& renderItem = m_glassRenderItems.emplace_back();
        renderItem.modelMatrix = m_modelMatrix;
        renderItem.inverseModelMatrix = inverse(renderItem.modelMatrix);
        renderItem.meshIndex = meshIndex;
        renderItem.baseColorTextureIndex = m_exteriorMaterial->m_basecolor;
        renderItem.rmaTextureIndex = m_exteriorMaterial->m_rma;
        renderItem.normalMapTextureIndex = m_exteriorMaterial->m_normal;
    }
}

void Window::SetMousePickIndex(int mousePickIndex) {
    m_mousePickIndex = mousePickIndex;
}
