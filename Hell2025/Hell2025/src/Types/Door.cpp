#include "Door.h"
#include "Core/Audio.h"
#include "AssetManagement/AssetManager.h"
#include "Physics/Physics.h"
#include "Physics/Physics.h"
#include "UniqueID.h"

void Door::Init(DoorCreateInfo createInfo) {
    m_position = createInfo.position;
    m_rotation = createInfo.rotation;

    m_material = AssetManager::GetMaterialByName("Door"); 
    m_doorModel = AssetManager::GetModelByIndex(AssetManager::GetModelIndexByName("Door"));
    m_frameModel = AssetManager::GetModelByIndex(AssetManager::GetModelIndexByName("DoorFrame"));

    PhysicsFilterData filterData;
    filterData.raycastGroup = RAYCAST_ENABLED;
    filterData.collisionGroup = CollisionGroup::ENVIROMENT_OBSTACLE;
    filterData.collidesWith = (CollisionGroup)(GENERIC_BOUNCEABLE | BULLET_CASING | RAGDOLL);

    Transform doorFrameTransform;
    doorFrameTransform.position = m_position;
    doorFrameTransform.rotation = m_rotation;

    glm::vec3 boxExtents = glm::vec3(DOOR_DEPTH, DOOR_HEIGHT, DOOR_WIDTH);
    m_physicsId = Physics::CreateRigidStaticFromBoxExtents(doorFrameTransform, boxExtents, filterData);

    // Get next unique ID
    m_objectId = UniqueID::GetNext();

    // Set PhysX user data
    PhysicsUserData userData;
    userData.physicsId = m_physicsId;
    userData.objectId = m_objectId;
    userData.physicsType = PhysicsType::RIGID_STATIC;
    userData.objectType = ObjectType::DOOR;
    Physics::SetRigidStaticUserData(m_physicsId, userData);
}

void Door::CleanUp() {
    Physics::MarkRigidStaticForRemoval(m_physicsId);
}

void Door::Update(float deltaTime) {

    Transform doorFrameTransform;
    doorFrameTransform.position = m_position;
    doorFrameTransform.rotation = m_rotation;
    m_frameModelMatrix = doorFrameTransform.to_mat4();

    Transform doorTransform;
    doorTransform.position = glm::vec3(0.058520, 0, 0.39550f);
    doorTransform.rotation.y = m_currentOpenRotation;
    m_doorModelMatrix = m_frameModelMatrix * doorTransform.to_mat4();

    // Update PhysX transform
    Transform physxOffsetTransform;
    physxOffsetTransform.position.z = DOOR_WIDTH * -0.5f;
    physxOffsetTransform.position.y = DOOR_HEIGHT * 0.5f;
    physxOffsetTransform.position.x = DOOR_DEPTH * -0.5f;
    glm::mat4 globalPoseMatrix = m_doorModelMatrix * physxOffsetTransform.to_mat4();
    Physics::SetRigidStaticGlobalPose(m_physicsId, globalPoseMatrix);

    float openSpeed = 5.208f;
    if (m_openingState == OpeningState::OPENING) {
        m_currentOpenRotation -= openSpeed * deltaTime;
        if (m_currentOpenRotation < -m_maxOpenRotation) {
            m_currentOpenRotation = -m_maxOpenRotation;
            m_openingState = OpeningState::OPEN;
        }
    }
    if (m_openingState == OpeningState::CLOSING) {
        m_currentOpenRotation += openSpeed * deltaTime;
        if (m_currentOpenRotation > 0) {
            m_currentOpenRotation = 0;
            m_openingState = OpeningState::CLOSED;
        }
    }

    UpdateRenderItems();
}

void Door::UpdateRenderItems() {
    m_renderItems.clear();

    // Handle missing material
    if (!m_material) {
        m_material = AssetManager::GetDefaultMaterial();
    }
    
    // Bail if models are invalid
    if (!m_doorModel || !m_frameModel) return;

    for (uint32_t& meshIndex : m_doorModel->GetMeshIndices()) {
        Mesh* mesh = AssetManager::GetMeshByIndex(meshIndex);
        RenderItem& renderItem = m_renderItems.emplace_back();
        renderItem.modelMatrix = m_doorModelMatrix;
        renderItem.inverseModelMatrix = inverse(renderItem.modelMatrix);
        renderItem.meshIndex = meshIndex;
        renderItem.baseColorTextureIndex = m_material->m_basecolor;
        renderItem.rmaTextureIndex = m_material->m_rma;
        renderItem.normalMapTextureIndex = m_material->m_normal;
    }

    for (uint32_t& meshIndex : m_frameModel->GetMeshIndices()) {
        Mesh* mesh = AssetManager::GetMeshByIndex(meshIndex);
        RenderItem& renderItem = m_renderItems.emplace_back();
        renderItem.modelMatrix = m_frameModelMatrix;
        renderItem.inverseModelMatrix = inverse(renderItem.modelMatrix);
        renderItem.meshIndex = meshIndex;
        renderItem.baseColorTextureIndex = m_material->m_basecolor;
        renderItem.rmaTextureIndex = m_material->m_rma;
        renderItem.normalMapTextureIndex = m_material->m_normal;
    }
}

void Door::SetMousePickIndex(int mousePickIndex) {
    m_mousePickIndex = mousePickIndex;
}

void Door::Interact() {
    if (m_openingState == OpeningState::CLOSED) {
        m_openingState = OpeningState::OPENING;
        Audio::PlayAudio("Door_Open.wav", 1.0f);
    }
    else if (m_openingState == OpeningState::OPEN) {
        m_openingState = OpeningState::CLOSING;
        Audio::PlayAudio("Door_Open.wav", 1.0f);
    }
}
