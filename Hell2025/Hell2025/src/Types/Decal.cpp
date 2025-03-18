#include "Decal.h"
#include "AssetManagement/AssetManager.h"
#include "Physics/Physics.h"
#include "Renderer/RenderDataManager.h"
#include "UniqueID.h"

void Decal::Init(const DecalCreateInfo& createInfo) {
    m_parentPhysicsId = createInfo.parentPhysicsId;
    m_parentObjectId = createInfo.parentObjectId;
    m_parentPhysicsType = createInfo.parentPhysicsType;
    m_parentObjectType = createInfo.parentObjectType;

    // Compute local position: aka position relative to the parent at time of hit
    glm::mat4 parentMatrix = Physics::GetRigidStaticGlobalPose(m_parentPhysicsId);
    glm::vec3 localPosition = glm::vec3(glm::inverse(parentMatrix) * glm::vec4(createInfo.surfaceHitPosition, 1.0f));
   
    // Compute local normal: aka normal relative to the parent orientation at time of hit
    glm::mat3 parentRotation = glm::mat3(parentMatrix);
    glm::mat3 rotationTranspose = glm::transpose(parentRotation);
    m_localNormal = glm::normalize(rotationTranspose * createInfo.surfaceHitNormal);

    // Slightly offset the position to avoid z fighting
    localPosition += m_localNormal * glm::vec3(0.003f);

    // Scale and material varies per decal type
    float scale = 1.0f;
    if (createInfo.decalType == DecalType::GLASS) {
        m_material = AssetManager::GetMaterialByName("BulletHole_Glass");
        scale = 0.035f;
    }
    else if (createInfo.decalType == DecalType::PLASTER) {
        m_material = AssetManager::GetMaterialByName("BulletHole_Plaster");
        scale = 0.02f;
    }

    float maxExtent = scale / glm::sqrt(2.0f);
    m_localAABB = AABB(-glm::vec3(maxExtent), glm::vec3(maxExtent));

    // Compute the local matrix once because it never changes
    float randomRotation = Util::RandomFloat(0.0f, HELL_PI * 2.0f);
    m_localMatrix = glm::translate(glm::mat4(1.0f), localPosition);
    m_localMatrix *= Util::RotationMatrixFromForwardVector(m_localNormal, glm::vec3(0, 0, 1), glm::vec3(0, 1, 0));
    m_localMatrix *= glm::rotate(glm::mat4(1.0f), randomRotation, glm::vec3(0, 0, 1));
    m_localMatrix *= glm::scale(glm::mat4(1.0f), glm::vec3(scale));

    std::cout << "surfaceHitNormal: " << Util::Vec3ToString(createInfo.surfaceHitNormal) << "\n";
    std::cout << "m_localNormal: " << Util::Vec3ToString(m_localNormal) << "\n";

    glm::mat4 rot = Util::RotationMatrixFromForwardVector(m_localNormal, glm::vec3(0, 0, 1), glm::vec3(0, 1, 0));

    glm::vec3 result = rot * glm::vec4(createInfo.surfaceHitNormal, 0);
    result = glm::normalize(result);

    std::cout << "result: " << Util::Vec3ToString(result) << "\n";

}

void Decal::SubmitRenderItem() {
    static int meshIndex = AssetManager::GetMeshIndexByModelNameMeshName("Primitives", "Quad");

    glm::mat4 parentMatrix = Physics::GetRigidStaticGlobalPose(m_parentPhysicsId);
    m_modelMatrix = parentMatrix * m_localMatrix;

    RenderItem renderItem;
    renderItem.meshIndex = meshIndex;
    renderItem.modelMatrix = m_modelMatrix;
    renderItem.aabbMin = glm::vec4(GetPosition() - m_localAABB.GetBoundsMin(), 1.0);
    renderItem.aabbMax = glm::vec4(GetPosition() + m_localAABB.GetBoundsMax(), 1.0);
    renderItem.baseColorTextureIndex = m_material->m_basecolor;
    renderItem.normalMapTextureIndex = m_material->m_normal;
    renderItem.rmaTextureIndex = m_material->m_rma;

    RenderDataManager::SubmitDecalRenderItem(renderItem);
}