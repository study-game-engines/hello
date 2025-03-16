#pragma once
#pragma warning(push, 0)
#include "PxPhysicsAPI.h"
#include "geometry/PxGeometryHelpers.h"
#pragma warning(pop)
#include "HellTypes.h"
#include "Math/AABB.h"
#include "Physics/Types/RigidDynamic.h"
#include <span>
#include <vector>

using namespace physx;

struct CollisionReport {
    PxActor* rigidA = NULL;
    PxActor* rigidB = NULL;
};

struct CharacterCollisionReport {
    PxController* characterController;
    PxShape* hitShape;
    PxRigidActor* hitActor;
    glm::vec3 hitNormal;
    glm::vec3 worldPosition;
};

class CCTHitCallback : public PxUserControllerHitReport {
public:
    void onShapeHit(const PxControllerShapeHit& hit);
    void onControllerHit(const PxControllersHit& hit);
    void onObstacleHit(const PxControllerObstacleHit& hit);
};

namespace Physics {
    void Init();
    void BeginFrame();
    void StepPhysics(float deltaTime);
    void AddCollisionReport(CollisionReport& collisionReport);
    void ClearCollisionReports();
    void ClearCharacterControllerCollsionReports();
    std::vector<CollisionReport>& GetCollisionReports();
    std::vector<CharacterCollisionReport>& GetCharacterCollisionReports();
    std::vector<Vertex> GetDebugLineVertices(DebugLineRenderMode debugLineRenderMode, std::vector<PxRigidActor*> ignoreList);
    PxPhysics* GetPxPhysics();
    PxScene* GetPxScene();
    CCTHitCallback& GetCharacterControllerHitCallback();    
    PxControllerManager* GetCharacterControllerManager();

    // Materials
    PxMaterial* GetDefaultMaterial();
    PxMaterial* GetGrassMaterial();

    // Create
    PxShape* CreateBoxShape(float width, float height, float depth, Transform shapeOffset = Transform(), PxMaterial* material = NULL);
    PxRigidDynamic* CreateRigidDynamic(Transform transform, PhysicsFilterData filterData, PxShape* shape, Transform shapeOffset = Transform());
    PxShape* CreateConvexShapeFromVertexList(std::span<Vertex>& vertices);

    // Height fields
    void UpdateHeightFields();
    void CreateHeightField(vecXZ& worldSpaceOffset, const float* heightValues);
    void MarkAllHeightFieldsForRemoval();
    void RemoveAnyHeightFieldMarkedForRemoval();

    // Rigid Dynamics
    void UpdateActiveRigidDynamicAABBList();
    void MarkRigidDynamicForRemoval(uint64_t rigidDynamicId);
    void RemoveRigidDynamic(uint64_t rigidDynamicId);
    bool RigidDynamicExists(uint64_t rigidDynamicId);
    uint64_t CreateRigidDynamicFromConvexMeshVertices(Transform transform, const std::span<Vertex>& vertices, const std::span<uint32_t>& indices, float mass, PhysicsFilterData filterData, glm::vec3 initialForce = glm::vec3(0.0f), glm::vec3 initialTorque = glm::vec3(0.0f));
    uint64_t CreateRigidDynamicFromBoxExtents(Transform transform, glm::vec3 boxExtents, float mass, PhysicsFilterData filterData, glm::vec3 initialForce = glm::vec3(0.0f), glm::vec3 initialTorque = glm::vec3(0.0f));
    glm::mat4 GetRigidDynamicWorldMatrix(uint64_t rigidDynamicId);
    void RemoveAnyRigidDynamicForRemoval();
    void ActivateRigidDynamicPhysics(uint64_t rigidDynamicId);
    void DeactivateRigidDynamicPhysics(uint64_t rigidDynamicId);
    void SetRigidDynamicUserData(uint64_t rigidDynamicId, PhysicsUserData physicsUserData);
    void UpdateAllRigidDynamics(float deltaTime);
    const std::vector<AABB>& GetActiveRididDynamicAABBs();

    // Rigid statics
    void MarkRigidStaticForRemoval(uint64_t rigidStaticId);
    void RemoveRigidStatic(uint64_t rigidStaticId);
    void RemoveAnyRigidStaticForRemoval();
    void SetRigidStaticGlobalPose(uint64_t rigidStaticId, glm::mat4 globalPoseMatrix);
    glm::mat4 GetRigidStaticGlobalPose(uint64_t rigidStaticId);
    bool RigidStaticExists(uint64_t rigidStaticId);
    void SetRigidStaticUserData(uint64_t rigidStaticId, PhysicsUserData physicsUserData);
    uint64_t CreateRigidStaticFromBoxExtents(Transform transform, glm::vec3 boxExtents, PhysicsFilterData filterData);
    uint64_t CreateRigidStaticFromConvexMeshVertices(Transform transform, const std::span<Vertex>& vertices, PhysicsFilterData filterData);
    uint64_t CreateRigidStaticFromConvexMeshFromModel(Transform transform, const std::string& modelName, PhysicsFilterData filterData);


    // Destroy
    void Destroy(PxRigidDynamic*& rigidDynamic);
    void Destroy(PxRigidStatic*& rigidStatic);
    void Destroy(PxShape*& shape);
    void Destroy(PxRigidBody*& rigidBody);
    void Destroy(PxTriangleMesh*& triangleMesh);

    // Util
    glm::vec3 PxVec3toGlmVec3(PxVec3 vec);
    glm::vec3 PxVec3toGlmVec3(PxExtendedVec3 vec);
    glm::quat PxQuatToGlmQuat(PxQuat quat);
    glm::mat4 PxMat44ToGlmMat4(physx::PxMat44 pxMatrix);
    PxVec3 GlmVec3toPxVec3(glm::vec3 vec);
    PxQuat GlmQuatToPxQuat(glm::quat quat);
    PxMat44 GlmMat4ToPxMat44(glm::mat4 glmMatrix);
    PhysXRayResult CastPhysXRay(glm::vec3 rayOrigin, glm::vec3 rayDirection, float rayLength, PxU32 collisionFlags, bool cullBackFacing = false);
    OverlapReport OverlapTest(const PxGeometry& overlapShape, const PxTransform& shapePose, PxU32 collisionGroup);
}

class ContactReportCallback : public PxSimulationEventCallback {
public:
    void onConstraintBreak(PxConstraintInfo* constraints, PxU32 count) { PX_UNUSED(constraints); PX_UNUSED(count); }
    void onWake(PxActor** actors, PxU32 count) { PX_UNUSED(actors); PX_UNUSED(count); }
    void onSleep(PxActor** actors, PxU32 count) { PX_UNUSED(actors); PX_UNUSED(count); }
    void onTrigger(PxTriggerPair* pairs, PxU32 count) { PX_UNUSED(pairs); PX_UNUSED(count); }
    void onAdvance(const PxRigidBody* const*, const PxTransform*, const PxU32) {}

    void onContact(const PxContactPairHeader& pairHeader, const PxContactPair* /*pairs*/, PxU32 /*nbPairs*/) {
        if (!pairHeader.actors[0] || !pairHeader.actors[1]) {
            return;
        }
        CollisionReport report;
        report.rigidA = pairHeader.actors[0];
        report.rigidB = pairHeader.actors[1];
        Physics::AddCollisionReport(report);
    }
};
