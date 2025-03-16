#include "Player.h"
#include "Core/Audio.h"
#include "Physics/Physics.h"
#include "Util/Util.h"
#include "World/World.h"
#include <algorithm>

void Player::UpdateInteract() {

    PhysXRayResult m_cameraRayResult;
    OverlapReport m_overlapReport;

    float sphereRadius = 0.25f;
    PxCapsuleGeometry overlapSphereShape = PxCapsuleGeometry(sphereRadius, 0);

    // Camera ray
    uint32_t cameraRayFlags = RaycastGroup::RAYCAST_ENABLED;
    glm::vec3 cameraRayOrigin = GetCameraPosition();
    glm::vec3 cameraRayDirection = GetCameraForward();
    m_cameraRayResult = Physics::CastPhysXRay(cameraRayOrigin, cameraRayDirection, 100, cameraRayFlags);

    // Interact overlap test
    glm::vec3 cameraRayHitPosition = m_cameraRayResult.hitPosition;


    // Get overlap report
    const PxTransform overlapSphereTranform = PxTransform(Physics::GlmVec3toPxVec3(cameraRayHitPosition));
    m_overlapReport = Physics::OverlapTest(overlapSphereShape, overlapSphereTranform, CollisionGroup(GENERIC_BOUNCEABLE | GENERTIC_INTERACTBLE | ITEM_PICK_UP | ENVIROMENT_OBSTACLE));

    // Sort by distance to player
    sort(m_overlapReport.hits.begin(), m_overlapReport.hits.end(), [this, cameraRayHitPosition](OverlapResult& lhs, OverlapResult& rhs) {
        float distanceA = glm::distance(cameraRayHitPosition, lhs.objectPosition);
        float distanceB = glm::distance(cameraRayHitPosition, rhs.objectPosition);
        return distanceA < distanceB;
    });

    // Store closest overlap data
    if (m_overlapReport.hits.size()) {
        PhysicsUserData userData = m_overlapReport.hits[0].userData;
        m_interactObjectType = userData.objectType;
        m_interactObjectId = userData.objectId;
        m_interactFound = true;
    }

    // Pick up selected item
    if (PressedInteract()) {
        if (m_interactObjectType == ObjectType::PICK_UP) {
            PickUp* pickUp = World::GetPickUpByObjectId(m_interactObjectId);
            if (pickUp) {
                World::RemovePickUp(m_interactObjectId);
                Audio::PlayAudio("ItemPickUp.wav", 1.0f);

                m_interactObjectType = ObjectType::NONE;
                m_interactObjectId = 0;
                m_interactFound = false;
            }
        }

        if (m_interactObjectType == ObjectType::DOOR) {
            Door* door = World::GetDoorByObjectId(m_interactObjectId);
            if (door) {
                door->Interact();
            }
        }
    }
}