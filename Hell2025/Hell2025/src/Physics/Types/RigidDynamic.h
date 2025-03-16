#pragma once
#include "Math/AABB.h"
#include "PhysX/include/PxShape.h"
#include "PhysX/include/PxRigidDynamic.h"

using namespace physx;

struct RigidDynamic {
    void Update(float deltaTime);
    void ActivatePhsyics();
    void DeactivatePhysics();
    void SetPxRigidDynamic(PxRigidDynamic* rigidDynamic);
    void SetPxShape(PxShape* shape);
    void MarkForRemoval();

    bool HasActivePhysics()                 { return m_activePhysics; }
    bool IsMarkedForRemoval()               { return m_markedForRemoval; }
    PxRigidDynamic* GetPxRigidDynamic()     { return m_pxRigidDynamic; }
    PxShape* GetPxShape()                   { return m_pxShape; }
    const AABB& GetAABB()                   { return m_aabb; }
    const glm::vec3& GetCurrentPosition()   { return m_currentPosition; }
    const glm::vec3& GetPreviousPosition()  { return m_previousPosition; }

private:
    AABB m_aabb;
    PxShape* m_pxShape = nullptr;
    PxRigidDynamic* m_pxRigidDynamic = nullptr;
    glm::vec3 m_currentPosition;
    glm::vec3 m_previousPosition;
    float m_stationaryTime = 0;
    bool m_activePhysics = false;
    bool m_markedForRemoval = false;
};