#pragma once
#include "HellTypes.h"
#include "Math/AABB.h"
#include <vector>
#include <glm/vec3.hpp>
#include "PhysX/include/PxShape.h"
#include "PhysX/include/PxRigidStatic.h"
#include "PhysX/include/geometry/PxHeightField.h"

using namespace physx;

struct HeightField {
    void Create(vecXZ& worldSpaceOffset, const float* heightValues);
    void ActivatePhsyics();
    void DisablePhsyics();
    void MarkForRemoval();

    const bool HasActivePhysics() const     { return m_activePhysics; }
    const bool IsMarkedForRemoval() const   { return m_markedForRemoval; }
    vecXZ GetWorldSpaceOffset()             { return m_worldSpaceOffset; }
    PxHeightField* GetPxHeightField()       { return m_pxHeightField; }
    PxRigidStatic* GetPxRigidStatic()       { return m_pxRigidStatic; }
    PxShape* GetPxShape()                   { return m_pxShape; }
    const AABB& GetAABB()                   { return m_aabb; }

private:
    vecXZ m_worldSpaceOffset;
    PxHeightField* m_pxHeightField = nullptr;
    PxRigidStatic* m_pxRigidStatic = nullptr;
    PxShape* m_pxShape = nullptr;
    bool m_activePhysics = false;
    bool m_markedForRemoval = false;
    AABB m_aabb;
};