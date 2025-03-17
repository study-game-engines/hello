#include "Decal.h"
#include "Physics/Physics.h"

void Decal::Init(const DecalCreateInfo& createInfo) {
    m_position = createInfo.position;
    m_rotation = createInfo.rotation;
    m_scale = createInfo.scale;
    m_parentPhysicsId = createInfo.parentPhysicsId;
    m_parentObjectId = createInfo.parentObjectId;
    m_parentPhysicsType = createInfo.parentPhysicsType;
    m_parentObjectType = createInfo.parentObjectType;
    m_localPosition = createInfo.localPosition;
    m_localNormal = createInfo.localNormal;
    Update();
}

void Decal::Update() {
    glm::mat4 parentMatrix = Physics::GetRigidStaticGlobalPose(m_parentPhysicsId);

    Transform localTranslation;
    localTranslation.position = m_localPosition;
    localTranslation.rotation.y = HELL_PI * 0.5f;
    localTranslation.scale = glm::vec3(0.04f);

    Transform localRotation;
    localRotation.rotation.z = Util::RandomFloat(0.0f, HELL_PI * 2.0f);

    m_modelMatrix = parentMatrix * localTranslation.to_mat4() * localRotation.to_mat4();
}